# Event System Architecture

## Purpose
This directory houses the core communication infrastructure for the plugin. It implements the Observer/Event pattern to decouple components (GUI, Processor, etc.), ensuring that changes in one module do not require modifying another.

## Core Components
1.  **`EventBus` (Singleton):**
    *   **Role:** The central dispatcher. It manages subscriptions and publishes events.
    *   **Implementation Detail:** Must be thread-safe, as events can be published from the GUI thread and consumed on the audio thread.
    *   **Methods:** `subscribe<T>(callback)`, `publish(const T& event)`.

2.  **Event Classes (e.g., `ParameterChangedEvent`, `ClickEvent`):**
    *   **Role:** Simple data structures that encapsulate the necessary information about an event. They should contain only data, not logic.
    *   **Example:** `ParameterChangedEvent` must contain `(UniqueID id, float newValue)`.

## Interaction Flow
1.  **Publishing:** A component (e.g., `PluginEditor`) detects a change and calls `EventBus::getInstance().publish(ParameterChangedEvent{...})`.
2.  **Dispatching:** The `EventBus` iterates through all registered callbacks for that event type and executes them.
3.  **Subscribing:** The `PluginProcessor` subscribes to relevant events in its constructor, allowing it to react to external changes without knowing *who* caused the change.

## Next Steps
The next steps will involve implementing the `EventBus` singleton and defining the initial event structures.
