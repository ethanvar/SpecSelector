#pragma once

#include <cstdint>
#include <functional>
#include <juce_core/juce_core.h>
#include <map>
#include <mutex>
#include <typeindex>
#include <utility>
#include <vector>

// Base class for all events
struct Event {
  virtual ~Event() = default;
};

// Example Event: For when a parameter changes
struct ParameterChangedEvent : public Event {
  // Using a unique ID (like a string or enum) instead of raw int for better
  // readability
  juce::String parameterID;
  float newValue;
};

// Example Event: For when a user clicks on the spectrogram
struct ClickEvent : public Event {
  double timeSeconds;
  double frequencyHz;
};

// Event: For when the current undo/redo index changes in DrawActionTracker
struct StrokeIndexChangedEvent : public Event {
  StrokeIndexChangedEvent(int index) : currentIndex(index) {}
  int currentIndex;
};

class EventBus {
public:
  // Opaque handle returned by subscribe(); pass it to unsubscribe() to remove
  // the callback (e.g. from a destructor) and avoid dangling-subscription
  // crashes on this singleton.
  using SubscriptionHandle = std::uint64_t;

  // Singleton access
  static EventBus &getInstance();

  // Subscription mechanism. Returns a handle used to unsubscribe.
  template <typename EventType>
  SubscriptionHandle subscribe(std::function<void(const EventType &)> callback) {
    std::lock_guard<std::mutex> lock(mutex);

    SubscriptionHandle handle = nextHandle++;

    // Wrap the specific callback into a generic one that accepts const Event&
    std::function<void(const Event &)> genericCallback =
        [callback](const Event &event) {
          // Cast the base Event& back to the expected derived type
          const EventType &specificEvent = static_cast<const EventType &>(event);
          callback(specificEvent);
        };

    // Use typeid() to get the type_info for the key
    subscribers[std::type_index(typeid(EventType))].push_back(
        {handle, std::move(genericCallback)});

    return handle;
  }

  // Removes a previously registered subscription.
  void unsubscribe(SubscriptionHandle handle);

  // Publishing mechanism
  void publish(const Event &event);

private:
  EventBus() = default;
  ~EventBus() = default;
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  SubscriptionHandle nextHandle = 1;

  // Storage for callbacks, keyed by event type ID (or type_info)
  std::map<
      std::type_index,
      std::vector<std::pair<SubscriptionHandle, std::function<void(const Event &)>>>>
      subscribers;
  std::mutex mutex;
};
