#include "EventBus.h"

#include <algorithm>

// --- Singleton Implementation ---
EventBus &EventBus::getInstance() {
  static EventBus instance;
  return instance;
}

// --- Unsubscribe Implementation ---
void EventBus::unsubscribe(SubscriptionHandle handle) {
  std::lock_guard<std::mutex> lock(mutex);

  for (auto &entry : subscribers) {
    auto &callbacks = entry.second;
    callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
                                   [handle](const auto &sub) {
                                     return sub.first == handle;
                                   }),
                    callbacks.end());
  }
}

// --- Publishing Implementation ---
void EventBus::publish(const Event &event) {
  // Copy the callbacks under the lock, then invoke them outside it so that a
  // callback which publishes another event cannot deadlock on the mutex.
  std::vector<std::pair<SubscriptionHandle, std::function<void(const Event &)>>>
      callbacks;

  {
    std::lock_guard<std::mutex> lock(mutex);
    std::type_index eventType = std::type_index(typeid(event));
    auto it = subscribers.find(eventType);
    if (it == subscribers.end()) {
      return;
    }
    callbacks = it->second;
  }

  for (const auto &callback : callbacks) {
    callback.second(event);
  }
}