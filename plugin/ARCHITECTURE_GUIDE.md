# Architectural Improvement Guide: Moving Beyond the Mediator

This guide outlines how to refactor the current plugin architecture to improve scalability, robustness, and adherence to best practices, particularly by replacing the Mediator with an Event/Observer pattern and isolating heavy tasks.

## 1. Implementing an Event/Observer System (Replacing Mediator)

The goal is to decouple components so that changes in one area don't require modifying the central mediator.

**Implementation Steps:**
1.  **Define Event:** Create a base `Event` class or struct (e.g., `ParameterChangedEvent`, `ClickEvent`).
2.  **Implement an Event Bus:** Create a singleton or static class, `EventBus`, responsible for managing subscriptions.
    *   It needs methods like `subscribe<T>(std::function<void(const T&)> callback)` and `publish(const Event& event)`.
3.  **Update Components:**
    *   **`StftDisplay`:** This component will now be responsible for detecting mouse clicks. Instead of relying on external listeners, it must publish a `ClickEvent` to the `EventBus` upon a mouse down event.
    *   **`Mediator`:** Remove the direct `mouseDown` implementation. Instead, `Mediator` must subscribe to the `ClickEvent` on the `EventBus` in its initialization. It will implement a handler method that processes the event data.
    *   **`PluginProcessor`:** If it needs click data, it must also subscribe to the `ClickEvent` on the `EventBus` and update its internal state accordingly.

## 2. State Management and Data Flow
The source of truth for any piece of data (like the clicked coordinates) must be centralized and immutable once set.

**Implementation Steps:**
1.  **Central State Holder:** Keep the state variables (e.g., `clickedTime`, `clickedFrequency`) within the `PluginProcessor`.
2.  **Event Trigger:** When the GUI detects a click, it publishes a `ClickEvent` containing the raw coordinates.
3.  **State Update:** The `PluginProcessor`'s event handler receives this event and *updates its internal state* (`this->clickedTime = event.time;`).
4.  **Consumption:** The `processBlock` method reads the state directly from the processor (`this->clickedTime`) rather than relying on a passed-in parameter.

## 3. Background Threading for Non-Real-Time Tasks (Export)
File I/O and complex calculations must be offloaded from the audio thread.

**Implementation Steps:**
1.  **Introduce a Job Queue:** Use `juce::ThreadPool` or a similar mechanism.
2.  **Export Trigger:** When the user clicks "Export," the `PluginEditor` should *not* start the export itself. Instead, it should create a `ExportJob` object and submit it to the `ThreadPool`.
3.  **Job Execution:** The `ExportJob` runs on a background thread. It will:
    *   Acquire the necessary audio data (this might require the `PluginProcessor` to buffer the audio data or provide a snapshot).
    *   Perform the complex processing/encoding.
    *   Write the final file using JUCE's file utilities.
4.  **Completion Callback:** The `ExportJob` must include a callback mechanism that notifies the `PluginEditor` (via the `EventBus`) when the file is successfully written, allowing the GUI to show a "Export Complete" message.

## 4. Abstracting Effects (The Plugin Effect Interface)
To make the system truly modular, define an interface for any processing unit.

**Implementation Steps:**
1.  **Define Interface:** Create an abstract base class, `AudioEffect`, in a common header file.
    ```cpp
    class AudioEffect {
    public:
        virtual ~AudioEffect() = default;
        virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
        virtual void process(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) = 0;
        virtual juce::String getName() const = 0;
    };
    ```
2.  **Refactor `PluginProcessor`:**
    *   Change the internal structure to hold a `std::vector<std::unique_ptr<AudioEffect>>` instead of hardcoding the processing logic.
    *   In `processBlock`, iterate through this vector:
        ```cpp
        for (auto& effect : effects) {
            effect->process(buffer, startSample, numSamples);
        }
        ```
3.  **Concrete Effects:** Create concrete implementations (e.g., `STFTEffect`, `FilterEffect`) that inherit from `AudioEffect`. The `PluginEditor` would then be responsible for creating and adding these effect instances to the processor's list.
