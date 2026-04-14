# Plugin Component Interaction Diagram (Current State)

This document outlines the current dependencies and data flow between components within the `plugin/` directory, detailing how the system operates *before* implementing the full Event/Observer pattern refactor.

## 1. Core Components & Roles

*   **`PluginEditor`**: The main GUI container. It initializes and manages the layout of other components (`Mediator`, `StftDisplay`, etc.).
*   **`Mediator`**: Acts as the central coordinator. It receives raw input events (like mouse clicks) and translates them into actions for other components.
*   **`PluginProcessor`**: Contains the core audio processing logic. It receives updates and state changes from the `Mediator`.
*   **`StftDisplay`**: A JUCE `Component` responsible for visualizing the spectrogram. It is directly attached to the `Mediator` and handles its own drawing lifecycle.
*   **`ProgressMask`**: A JUCE `Component` responsible for visualizing progress (e.g., song playback). It is also managed and updated by the `Mediator`.
*   **`EventBus`**: (Currently being implemented/designed) Intended to replace the direct coupling shown below.

## 2. Data Flow & Dependencies

### A. Mouse Click Handling (GUI Input)
1.  **Source:** A user clicks on the `StftDisplay` component.
2.  **Event Capture:** The `Mediator` is set up to listen for mouse events on the `StftDisplay` (via `addMouseListener` in `Mediator::Mediator()`).
3.  **Processing:** The `Mediator::mouseDown` method captures the raw screen coordinates (`event.getMouseDownX()`, etc.).
4.  **Action:** The `Mediator` processes these coordinates and uses them to drive the application logic (this is the point of coupling that needs replacement).

### B. State Updates & Visualization
1.  **Progress Update:** The `Mediator` is responsible for calling `m_mask.updateProgress(progress, songLength)` when the song playback progresses.
2.  **Spectrogram Update:** The `Mediator` calls `m_stftDisplay.updateSpectrogram()` to refresh the visual data.
3.  **Layout:** The `Mediator` manages the layout by calling `resized()` on both `StftDisplay` and `ProgressMask`.

### C. Audio Processing Flow
1.  **Input:** The `PluginProcessor` receives audio data via `processBlock(juce::AudioBuffer<float>& buffer, ...)` (standard JUCE audio callback).
2.  **Control:** The `Mediator` controls the *state* of the processor by calling methods on it (e.g., setting parameters, which is implied by the architecture guide).

## 4. Component Interaction Diagram (New State - Event/Observer Pattern)

This diagram illustrates the intended, decoupled flow after refactoring to use the `EventBus`.

### A. Mouse Click Handling (GUI Input)
1.  **Source:** A user clicks on the `StftDisplay` component.
2.  **Event Trigger:** `StftDisplay` detects the mouse down event and immediately publishes a `ClickEvent` containing the raw coordinates to the `EventBus`.
3.  **Event Consumption:** The `Mediator` (and potentially `PluginProcessor`) is subscribed to `ClickEvent` on the `EventBus`.
4.  **Action:** The subscribed handler method in `Mediator` receives the event and executes the necessary logic, completely decoupling the click source from the logic handler.

### B. State Updates & Visualization
*   **Progress Update:** The `Mediator` remains responsible for calling `m_mask.updateProgress(...)` when the song playback progresses, as this is a direct state synchronization, not an event reaction.
*   **Spectrogram Update:** The `Mediator` remains responsible for calling `m_stftDisplay.updateSpectrogram()` to refresh the visual data.

### C. Audio Processing Flow
*   **Input:** The `PluginProcessor` receives audio data via `processBlock(...)`.
*   **State Update:** If the click data is needed, the `PluginProcessor` subscribes to `ClickEvent` and updates its internal state (`this->clickedTime = event.time;`) within its event handler.
*   **Consumption:** The `processBlock` method reads the state directly from the processor (`this->clickedTime`) rather than relying on a passed-in parameter.

## 5. Summary of Coupling Points (Goal)
*   **`StftDisplay` $\rightarrow$ `EventBus`**: Publishes `ClickEvent`.
*   **`Mediator` $\leftarrow$ `EventBus`**: Subscribes to `ClickEvent`.
*   **`PluginProcessor` $\leftarrow$ `EventBus`**: Subscribes to `ClickEvent`.
*   **`Mediator` $\rightarrow$ `ProgressMask`**: Direct state update call (remains).
*   **`Mediator` $\rightarrow$ `StftDisplay`**: Direct state update call (remains).

This new model minimizes direct dependencies, making the system more modular.