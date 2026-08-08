#pragma once

#include <functional>
#include <juce_core/juce_core.h>
#include <map>
#include <mutex>
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
  // Singleton access
  static EventBus &getInstance();

  // Subscription mechanism
  template <typename EventType>
  void subscribe(std::function<void(const EventType &)> callback);

  // Publishing mechanism
  void publish(const Event &event);

private:
  EventBus() = default;
  ~EventBus() = default;
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  // Storage for callbacks, keyed by event type ID (or type_info)
  std::map<std::type_index, std::vector<std::function<void(const Event &)>>>
      subscribers;
  std::mutex mutex;
};
