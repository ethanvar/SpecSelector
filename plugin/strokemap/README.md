# StrokeMap

A 2D pixel-indexed map for recording which stroke (and what kind) was drawn
on each pixel of a spectrogram.  Enables fast O(1) lookup from the audio
callback so that `processSpectrum()` can modify FFT bins based on what the
user drew.

## File layout

```
plugin/strokemap/
  StrokeMap.h   — single-header class (declarations + inline implementations)
```

## Why two layers?

```
pixel (x,y) ──typeMap──► stroke type ID (1=delay, 2=reverb, …)
          ──strokeIndexMap──► registry index ──► StrokeEffect { radius, colour, … }
```

The type field lets the audio callback dispatch to effect-specific logic
without iterating a list.  The index field gives access to the full
`StrokeEffect` params that vary between individual strokes of the same type.

## Usage

### 1. Add to `Stft` (PluginProcessor → Stft ownership)

In `plugin/stft/Stft.h`, add a member:

```cpp
#include "../strokemap/StrokeMap.h"

class Stft {
  StrokeMap spectrogramMap;
  // ...
};
```

### 2. Initialise when the audio format is known

In `Stft::processEntireFile()` or `prepareToPlay()`, after you know the
FFT size:

```cpp
spectrogramMap.init(513);          // 513 = fftSize/2 + 1 for fftSize=1024
spectrogramMap.expandTo(1);        // start with one time-frame
```

### 3. Grow the map as audio plays

At the end of each `processFrame()` call (or wherever a new spectrogram
row is produced):

```cpp
spectrogramMap.expandTo(segmentsPassed + 1);
```

### 4. Paint a stroke from the Editor

In `PluginEditor` or wherever a `DrawingStroke` is finalised, after
painting the visual on the `juce::Image`:

```cpp
// Assign your own effect-type integer (e.g. 1 = delay, 2 = chorus).
uint8_t effectType = 1;

processorRef.stft.spectrogramMap.addStroke(
    effectType,
    stroke.strokeInfo,   // from DrawingStroke
    processorRef.stft.segmentsPassed  // current time-row in spectrogram
);
```

### 5. Read from the audio thread in `processSpectrum()`

In `plugin/stft/StftBuffers.cpp`, inside `processSpectrum()`:

```cpp
void StftBuffers::processSpectrum(float *data, int numBins) {
    std::complex<float> *cdata = reinterpret_cast<std::complex<float> *>(data);

    // Get the current time-row (frame index).
    int timeRow = /* derive from your class, e.g. count / hopSize */;

    for (int i = 0; i < numBins; ++i) {
        auto *effect = inputsRef.strokeMap->getStrokeEffect(i, timeRow);
        if (effect != nullptr) {
            // Apply effect-specific modification to cdata[i].
            // Example:
            //   magnitude *= effect->magnitude;
            //   phase   *= effect->phase;
            //   cdata[i] = std::polar(magnitude, phase);
            //
            // You can also branch on effect->type for different effects.
        }

        // … existing processing …
    }
}
```

You'll need to pass the `StrokeMap` pointer to `StftBuffers`.  The
simplest approach is via the existing `inputsRef` or a new constructor
parameter:

```cpp
// In StftBuffers.h
class StftBuffers {
    const StrokeMap* strokeMap_ = nullptr;
public:
    // existing constructor
    StftBuffers(..., const StrokeMap* map) : strokeMap_(map) {}
    // or a setter:
    void setStrokeMap(const StrokeMap* map) { strokeMap_ = map; }
};
```

Then in `Stft` constructor / init:

```cpp
fftBuffers[0] = StftBuffers(w, f, inputs, &spectrogramMap);
fftBuffers[1] = StftBuffers(w, f, inputs, &spectrogramMap);
```

### 6. Undo

When undoing a stroke:

```cpp
spectrogramMap.clear();
// OR — if you want finer-grained undo — rebuild the map from the
// DrawingStroke objects stored in DrawActionTracker.
```

For a full undo system you would snapshot `typeMap_`, `strokeIndexMap_`,
and `strokeRegistry_` before each `addStroke()` call and restore on undo.
This is left as a future extension because storing full snapshots can be
expensive for large spectrograms.

## Thread-safety notes

- `addStroke()` is called from the **UI thread** (Editor).
- `getStrokeEffect()` is called from the **audio thread** (processSpectrum).

With the current design, writes from the UI thread can race with reads
from the audio thread.  For a production plugin you have three options:

1. **Atomic flag** — have the Editor set an `std::atomic<bool> dirty{true}`
   flag after `addStroke()`.  In `processSpectrum()`, check and clear the
   flag at the start; if dirty, skip the current frame's lookup (the
   spectrogram frame hasn't arrived yet anyway, so this is safe).

2. **Double-buffered map** — keep two `StrokeMap` instances and swap
   pointers atomically.  The audio thread always reads the old buffer
   while the Editor writes to the new one.

3. **Queue strokes** — collect stroke additions in a `juce::MessageQueue`
   and replay them on the audio thread before `processSpectrum()`.

For development, option 1 is the simplest and matches how the existing
code already avoids race conditions with `DialInputs`.

## Memory footprint

For a typical 1024-point FFT:
- `width = 513` bins
- `height = N` frames (N grows with audio length)

```
typeMap:        N × 513 × 1 byte     ≈ N × 0.5 kB
strokeIndexMap: N × 513 × 2 bytes    ≈ N × 1.0 kB
strokeRegistry: M × 32 bytes         (M = number of strokes, typically small)
Total:          ≈ N × 1.5 kB + M × 32 B
```

For a 5-minute song at hopSize=256: N ≈ 70 000 → ~105 MB.

If memory becomes an issue, consider:
- Using `uint8_t` for the index map as well (supports 65 535 max strokes)
- Compressing the map (run-length encoding by row)
- Storing only stroke boundaries instead of full pixel coverage
