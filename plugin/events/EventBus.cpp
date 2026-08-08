#include "EventBus.h"

// --- Singleton Implementation ---
EventBus &EventBus::getInstance() {
  static EventBus instance;
  return instance;
}

// --- Subscription Implementation ---
template <typename EventType>
void EventBus::subscribe(std::function<void(const EventType &)> callback) {
  std::lock_guard<std::mutex> lock(mutex);

  // Wrap the specific callback into a generic one that accepts const Event&
  std::function<void(const Event &)> genericCallback =
      [callback](const Event &event) {
        // Safely cast the base Event& back to the expected derived type
        const EventType &specificEvent = static_cast<const EventType &>(event);
        callback(specificEvent);
      };

  // Use typeid() to get the type_info for the key
  subscribers[std::type_index(typeid(EventType))].push_back(genericCallback);
}

// --- Publishing Implementation ---
void EventBus::publish(const Event &event) {
  std::lock_guard<std::mutex> lock(mutex);

  std::type_index eventType = std::type_index(typeid(event));

  if (subscribers.count(eventType)) {
    // Iterate over a copy of the vector to prevent issues if a callback
    // modifies the subscription list
    auto callbacks = subscribers.at(eventType);
    for (const auto &callback : callbacks) {
      // Pass the event reference directly to the stored generic callback
      callback(event);
    }
  }
}

// Explicit instantiation declarations for common event types
template void EventBus::subscribe<ParameterChangedEvent>(
    std::function<void(const ParameterChangedEvent &)>);
template void
    EventBus::subscribe<ClickEvent>(std::function<void(const ClickEvent &)>);
template void EventBus::subscribe<StrokeIndexChangedEvent>(
    std::function<void(const StrokeIndexChangedEvent &)>);
