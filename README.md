# ChordChainer (SpecSelector)

## Project Goal

ChordChainer is a modular, cross-platform audio plugin built with JUCE and
CMake. The user loads an audio file, views its spectrogram (computed with a
Short-Time Fourier Transform), and draws strokes directly on the spectrogram
to mark regions. Those strokes modify the audio in real time through an STFT
overlap-add processing chain, and undo/redo of strokes affects both the
displayed spectrogram and the audible output.

**Product name:** Spec Selector
**Bundle ID:** ca.ethanvarghese.SpecSelector
**Formats:** Standalone, VST3
**C++ standard:** C++17
**Build system:** CMake + JUCE (`juce_add_plugin(SpecSelector ...)`)

## Demo

<video src="specSelectorDemo.mp4" controls muted playsinline width="100%"></video>

[▶ Watch the demo (specSelectorDemo.mp4)](./specSelectorDemo.mp4) — 40 s,
1280×720. The video shows the full workflow: loading a file, letting the
spectrogram render, playing the track back, drawing strokes on the
spectrogram to select regions, adjusting the Volume/Phase sliders, and
undoing/redoing strokes.

> **Note:** `specSelectorDemo.mp4` (~33 MB) is committed to the repository.
> GitHub renders `.mp4` files referenced in markdown as an inline player;
> the plain link above works as a fallback everywhere else. If you ever
> re-encode or replace the video, keep the filename the same so the embed
> keeps working.

## Project Structure

The core logic resides within the `plugin/` directory:

```
plugin/
├── PluginProcessor.h/.cpp      # Audio engine (AudioProcessor)
├── PluginEditor.h/.cpp         # GUI container (AudioProcessorEditor)
├── events/
│   ├── EventBus.h/.cpp         # Thread-safe singleton event bus
│   └── ARCHITECTURE.md         # Original design doc (historical)
├── interactable/
│   ├── DialInputs.h            # Magnitude/phase parameter struct
│   ├── DrawStrokeAction.h/.cpp # Undo/redo memento (spectrogram snapshot)
│   ├── DrawActionTracker.h     # Memento stack + current undo/redo index
│   ├── DrawActionTracker.cpp   # Empty (class is header-only)
│   └── StrokeSnapshot.h        # StrokeCoordinate + unused StrokeSnapshot
├── stft/
│   ├── Stft.h/.cpp             # Offline STFT analysis + spectrogram rendering
│   └── StftBuffers.h/.cpp      # Real-time sample-by-sample overlap-add FFT
├── strokemap/
│   └── StrokeMap.h/.cpp        # 2D pixel-indexed stroke effect map
└── views/
    ├── Mediator.h/.cpp         # Central coordinator (mouse input, undo/redo)
    ├── StftDisplay.h/.cpp      # Spectrogram image component
    └── ProgressMask.h/.cpp     # Playback progress overlay
```

## Core Components

### PluginProcessor (`AudioPluginAudioProcessor`)

The audio engine. Owns the `Stft` instance, the `DialInputs` parameters, and
the audio transport. `processBlock()` feeds every incoming sample through
`StftBuffers` for each channel and advances the `segmentsPassed` tracker from
the playhead position. `prepareToPlay()` reports the STFT latency
(`fftSize` = 512 samples) to the host.

### PluginEditor (`AudioPluginAudioProcessorEditor`)

The GUI container. Holds the sliders ("Volume", "Phase"), buttons (file open,
undo, redo, check-time), and a `juce::Viewport` that hosts the `Mediator`.
Slider changes write directly into `processorRef.dialInputs`. A 60 Hz timer
forwards playback progress to the `Mediator` and triggers the spectrogram
render once a file has been loaded (via the processor's repaint flag).

### views/

* **`Mediator`** — a `juce::Component` and the central coordinator. Owns
  `StftDisplay` and `ProgressMask` as value members (laid out to fill its
  bounds in `resized()`), receives all mouse input
  (`addMouseListener(this, true)`), builds `DrawingStroke` objects, drives
  undo/redo through `DrawActionTracker` (`Cmd+Z` / `Cmd+Shift+Z`), and
  publishes `StrokeIndexChangedEvent`s on the `EventBus`. Declares a
  `juce::UndoManager` member that is **unused** (undo/redo uses
  `DrawActionTracker`).
* **`StftDisplay`** — owns and paints the spectrogram `juce::Image` (stretched
  to the component bounds). `initializeSpectrogram()` renders the spectrogram
  from the offline analysis (`Stft::drawSpectorgramWithStft()`) and sizes the
  `StrokeMap` (`Stft::initStrokemap()`). Mouse clicks are disabled on this
  component — all interaction is handled by the `Mediator`. `resized()`
  recreates the image at the new size.
* **`ProgressMask`** — overlay that draws a semi-transparent black rectangle
  covering already-processed time columns, driven by
  `updateProgressWithSegments()` from the editor timer. The frame-based
  progress accessors (`setNumFrames`, `getCurrentProgress`, etc.) are
  currently unused.

### stft/

* **`Stft`** — owns the offline analysis, the spectrogram rendering, the two
  real-time `StftBuffers` (left/right), and the `StrokeMap`.
  * `processEntireFile()` reads the whole file (channel 0), chunks it into
    `fftSize` blocks, windows (Hamming, normalized), FFTs each block, and
    appends a `StftSegment` to `m_stft`.
  * `drawSpectorgramWithStft()` converts complex bins to normalized dB
    magnitudes, maps them to HSV colours, paints one column per frame into the
    rightmost image column (skewed Y-axis emphasizing low frequencies), then
    shifts the image left one column with `moveImageSection()`.
  * `drawOntoSpectrogram(...)` paints stroke points onto the image (clamped to
    image bounds).
  * `addStrokeToSpectrogram(stroke, coords, spectrogram, actionIndex)` writes
    the stroke into the `StrokeMap` under its undo/redo action index.
  * `updateSegmentsPassedInTrack()` advances the public `segmentsPassed`
    counter (completed FFT frames) as playback advances.
* **`StftBuffers`** — the real-time overlap-add FFT engine; one instance per
  audio channel. Holds references to the shared window/FFT, `DialInputs`, a
  `const StrokeMap*`, and `Stft::segmentsPassed`. Subscribes to
  `StrokeIndexChangedEvent` on the `EventBus` (storing the undo/redo index in
  `std::atomic<int> currentIndex`) and **unsubscribes in its destructor** via
  the subscription handle.

  ```
  processSample(sample, bypassed)
    ├── write sample into inputFifo, read outputFifo sample
    └── every hopSize (128) samples: processFrame(bypassed)
          ├── circular copy into fftData
          ├── window (Hann) multiply
          ├── forward FFT (unless bypassed)
          │     └── processSpectrum()            ← spectral modification here
          ├── inverse FFT (unless bypassed)
          ├── window multiply + windowCorrection gain
          └── overlap-add into outputFifo
  ```

  `processSpectrum()` applies the drawn strokes: for each bin, the current
  time column (`segmentsPassed`) and the bin's spectrogram row
  (`fftIndextoY`, hard-coded 360-pixel mapping with the same skewed Y-axis as
  the rendering) are looked up in the `StrokeMap`. If any stroke covering that
  pixel has an action index `<= currentIndex` (i.e. it is in the current
  undo/redo history), the bin is **painted**: its magnitude is scaled by
  `0.01 * dialInputs.magnitude` (the Volume dial). Unpainted bins are silenced
  (`magnitude *= 0`). Phase is scaled by `dialInputs.phase` (the Phase dial).

### strokemap/

`StrokeMap` is a 2D pixel-indexed map (sized to the spectrogram image: width =
time axis, height = frequency axis) recording which strokes cover each pixel.
It gives the audio callback a fast per-bin lookup without scanning a stroke
list.

* `typeMap_` — `uint8_t` per pixel, latest-wins stroke type ID.
* `strokeIndexMap_` — `std::vector<uint16_t>` per pixel; each entry is the
  undo/redo action index of a stroke covering that pixel.
* `strokeRegistry_` — `std::vector<StrokeEffect>` indexed by the undo/redo
  action index; carries per-stroke parameters (type, magnitude, phase, radius,
  colour, thickness).

Key API: `init(width, height)`, `addStroke(effectType, points, strokeIdx)`,
`getStrokeType(x, y)`, `getStrokeIndices(x, y)`, `getStrokeEffect(x, y)`,
`getEntriesForRow(row)`, `clear()`. The effect type is currently hard-coded to
`1`. `removeStroke()` is declared but not implemented.

The map is **not** cleared or rebuilt on undo — instead the audio thread gates
on the undo/redo head (`currentIndex`), so undone strokes are simply ignored
by `processSpectrum()`. `clear()` exists for a full reset (e.g. loading a new
file).

### interactable/

* **`DialInputs`** — plain struct holding the GUI-controlled `magnitude`
  (default 50, Volume slider) and `phase` (default 1, Phase slider). Written
  by the UI thread, read by the audio thread; not synchronized.
* **`StrokeInfo`** — a single painted point: `x`, `y`, `colour`, `radius`.
* **`StrokeCoordinate`** (in `StrokeSnapshot.h`) — plain `x`/`y` collected
  while a drag is in progress.
* **`DrawingStroke`** — a completed stroke: `path`, `colour`, `thickness`,
  `coordinates`, `strokeInfo`, and `spectrogramSnapshot` (unused field).
* **`DrawStrokeAction`** — undo/redo memento. Stores a copy of the spectrogram
  image from before the stroke was drawn, plus the stroke and a re-draw
  function. `returnActionImage()` returns the pre-stroke snapshot.
* **`DrawActionTracker`** — header-only memento stack
  (`std::vector<std::unique_ptr<DrawStrokeAction>>`) with a public
  `currentIndex`. `AddMemento()` discards any redo branch after
  `currentIndex`, appends the snapshot, and moves the index to the newest
  entry. `getAction(index)` throws `std::out_of_range` on an invalid index.

### events/

`EventBus` is a thread-safe singleton decoupling the UI thread from the audio
thread:

```cpp
using SubscriptionHandle = std::uint64_t;

template <typename EventType>
SubscriptionHandle subscribe(std::function<void(const EventType &)> callback);

void unsubscribe(SubscriptionHandle handle);
void publish(const Event &event);
```

* `subscribe<T>()` registers a callback (keyed by `std::type_index`) and
  returns an opaque handle. Keep the handle and pass it to `unsubscribe()`
  (e.g. from a destructor) to avoid dangling subscriptions on the singleton.
* `publish()` copies the matching callbacks under the mutex, then invokes them
  **outside** the lock, so a callback that publishes another event cannot
  deadlock.

Event types defined in `EventBus.h`:

| Event | Fields | Status |
| --- | --- | --- |
| `StrokeIndexChangedEvent` | `int currentIndex` | **Used** — published by `Mediator` on draw/undo/redo; consumed by `StftBuffers` |
| `ParameterChangedEvent` | `juce::String parameterID`, `float newValue` | Defined, not used |
| `ClickEvent` | `double timeSeconds`, `double frequencyHz` | Defined, not used |

## Processing Constants

| Constant | Value | Meaning |
| --- | --- | --- |
| `fftOrder` | 9 | FFT order |
| `fftSize` | 512 (`1 << 9`) | FFT block size / reported plugin latency |
| `numBins` | 257 (`fftSize / 2 + 1`) | Bins analysed in `processSpectrum` |
| `overlap` | 4 | Overlap factor |
| `hopSize` | 128 (`fftSize / overlap`) | Samples between frames |
| `windowCorrection` | `22.0f / 3.0f` | Overlap-add gain correction |

## Component Interaction Flow

1. **Initialization:** The DAW loads the plugin, creating a `PluginProcessor`
   and its `PluginEditor`. The editor constructs the `Mediator` (which builds
   `StftDisplay` and `ProgressMask`) and places it in a `Viewport`.

2. **File loading:** `openButtonClicked()` -> `FileChooser` ->
   `processorRef.loadFile(file)`, which creates an `AudioFormatReader`,
   configures the `AudioTransportSource`, and runs the offline analysis
   `Stft::processEntireFile()`. It sets a repaint flag that the editor's timer
   (60 Hz) picks up, resizing the `Mediator` to `getNumFrames() + 1` wide and
   calling `mediator.initializeStftDisplay()` ->
   `Stft::drawSpectorgramWithStft()` (renders the spectrogram) and
   `Stft::initStrokemap()` (sizes the `StrokeMap` to the image).

3. **Drawing strokes (GUI -> engine):** `mouseDown` seeds
   `currentStrokePoints`; `mouseDrag` appends points and paints live onto the
   spectrogram image. On `mouseUp` a `DrawingStroke` is built (radius 20,
   colour `(100, 0, 0)`), a `DrawStrokeAction` snapshot is pushed to the
   `DrawActionTracker`, and `Stft::addStrokeToSpectrogram()` writes the stroke
   into the `StrokeMap` under the new action index. The new index is published
   as a `StrokeIndexChangedEvent`.

4. **Audio processing (engine):** During `processBlock()`, every sample is
   passed through `StftBuffers::processSample()`. Every `hopSize` samples the
   FFT runs, and `processSpectrum()` looks up the current spectrogram column
   in the `StrokeMap`. Bins covered by a stroke within the current undo/redo
   history are scaled by the magnitude dial; unpainted bins are silenced;
   phase is scaled by the phase dial. The result is inverse-transformed and
   overlap-added to the output.

5. **Undo/redo:** `Cmd+Z` / `Cmd+Shift+Z` (or the Undo/Redo buttons) call
   `Mediator::undoLastStroke()` / `redoLastStroke()`, which restore the
   previous/next spectrogram snapshot (or redraw from scratch at index `-1`)
   and publish the new index. `StftBuffers` stores it in an
   `std::atomic<int>`, and `processSpectrum()` stops applying strokes that
   have been undone.

6. **Progress visualization:** The editor timer calls
   `mediator.updateMaskProgressWithSegments(stft.segmentsPassed)` so the
   `ProgressMask` overlay tracks playback position in STFT segments.

## Thread Model

| Thread | Work |
| --- | --- |
| **Audio thread** | `processBlock()` -> `StftBuffers::processSample()` / `processFrame()` / `processSpectrum()`. Reads `StrokeMap` and `DialInputs`. |
| **Message/UI thread** | Editor timer, slider/button callbacks, `Mediator` mouse handlers, `paint()`. Writes `StrokeMap` and `DialInputs`. |

Cross-thread state:

* `StrokeMap` — every public method takes an internal `std::mutex`, so
  concurrent reads (audio thread) and writes (UI thread) are serialized. Note
  that locking a mutex on the audio thread can add latency if the UI thread is
  mid-stroke; strokes are currently small (one per mouse-up), so this is
  acceptable for the current scope.
* Undo/redo index — propagated via `StrokeIndexChangedEvent`; stored as
  `std::atomic<int>` in `StftBuffers`.
* `DialInputs` (UI writes, audio reads) — plain struct, **not** synchronized.
* `Stft::segmentsPassed` (audio writes, UI timer reads) — plain int, **not**
  synchronized.

## Known Issues & Dead Code

* `DialInputs` and `segmentsPassed` are shared across threads without
  synchronization (works in practice, but a data race on paper).
* `DrawingStroke::spectrogramSnapshot` is never populated — wasted allocation
  per stroke.
* `StrokeSnapshot.h` defines `StrokeSnapshot`, which is never used (only
  `StrokeCoordinate` from that header is used).
* `DrawActionTracker.cpp` exists but is empty — the class is entirely inline
  in the header, and only the header is listed in `CMakeLists.txt`.
* `Mediator` declares a `juce::UndoManager` member that is unused.
* `StrokeMap::removeStroke()` is declared but not implemented.
* `processSpectrum()` passes a hard-coded `imageHeight` of 360 to
  `fftIndextoY()`; this must match the spectrogram height to stay aligned.
* `ParameterChangedEvent` and `ClickEvent` are defined for future use.

## Building

### Prerequisites

* CMake **3.22** or newer
* A C++17 compiler (GCC 11+ or Clang 12+ recommended)
* **JUCE 8** checked out into the `JUCE/` directory at the project root. JUCE
  is vendored (not a submodule, and it is git-ignored), so clone it yourself
  if it is missing:

  ```sh
  git clone https://github.com/juce-framework/JUCE.git JUCE
  ```

* Linux only — the system development libraries JUCE needs for audio plugins:

  ```sh
  # Debian / Ubuntu
  sudo apt install build-essential pkg-config \
      libasound2-dev libfreetype6-dev libfontconfig1-dev \
      libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
      libxcomposite-dev libgl1-mesa-dev
  ```

  (Web browser and curl support are disabled in `CMakeLists.txt`
  (`JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`), so the WebKit/curl development
  packages are not required.)

### Configure & build

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

This builds both formats declared in `CMakeLists.txt`:

| Format | Output |
| --- | --- |
| VST3 | `build/SpecSelector_artefacts/<Config>/VST3/Spec Selector.vst3` |
| Standalone | `build/SpecSelector_artefacts/<Config>/Standalone/Spec Selector` |

Because `COPY_PLUGIN_AFTER_BUILD true` is set, the VST3 bundle is also copied
automatically after every build:

* **Linux:** `~/.vst3/Spec Selector.vst3`
* **macOS:** `~/Library/Audio/Plug-Ins/VST3/Spec Selector.vst3`
* **Windows:** `%CommonProgramFiles%\VST3\Spec Selector.vst3`

To run without a DAW, launch the Standalone binary directly and select an
audio device when prompted.

## Using Spec Selector in Reaper

1. **Install the plugin.** If you built with the steps above, the VST3 is
   already in `~/.vst3`. Otherwise copy `Spec Selector.vst3` there manually
   (Reaper scans `~/.vst3`, `/usr/lib/vst3`, and `/usr/local/lib/vst3` on
   Linux by default).

2. **Rescan.** Open Reaper → *Options → Preferences → Plug-ins → VST* and
   click **Re-scan** (use *Clear cache/re-scan* if the plugin was rebuilt).
   New plugins are also picked up automatically on Reaper startup.

3. **Insert it.** Add it as a track FX (FX button on a track → search
   "Spec Selector"). It is a stereo in/out audio effect.

4. **Load audio.** Click **Choose file** in the plugin UI and pick a
   `.wav` or `.mp3`. Wait a moment — the plugin runs an offline STFT over the
   whole file, then renders the spectrogram inside the scrollable view.

5. **Play.** Start Reaper's transport. The plugin follows the host playhead
   and reports 512 samples of latency, so the grey progress mask sweeps across
   the spectrogram in sync with playback.

6. **Paint the selection.** Drag on the spectrogram to draw strokes over the
   time/frequency regions you want to keep. Painted bins pass through (scaled
   by the **Volume** slider); everything else is silenced. The **Phase**
   slider scales the phase of all bins.

7. **Undo / redo.** Use the **Undo** / **Redo** buttons in the plugin, or the
   keyboard: **Cmd+Z** (macOS) / **Ctrl+Z** (Linux/Windows) to undo and
   **Cmd/Ctrl+Shift+Z** to redo. Undo affects both the picture and the sound.

   > **Reaper keyboard tip:** if the shortcuts don't reach the plugin, open
   > the FX window's menu and enable **"Send all keyboard input to plug-in"**
   > (the plugin is built with `EDITOR_WANTS_KEYBOARD_FOCUS`).

## General Goal

To provide a robust, modular framework for building advanced, visually
interactive audio effects plugins that can analyze and modify audio based on
spectral data. File export of the processed audio, is planned but not yet
implemented.
