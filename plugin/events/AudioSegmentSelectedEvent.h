#pragma once

#include <juce_core/juce_core.h>

// Event published when a valid audio segment selection is finalized
struct AudioSegmentSelectedEvent : public Event {
    double startTimeSeconds;
    double endTimeSeconds;

    AudioSegmentSelectedEvent(double start, double end) : startTimeSeconds(start), endTimeSeconds(end) {}
};