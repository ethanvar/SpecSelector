#pragma once

#include <juce_core/juce_core.h>

// Event for tracking mouse movement during a drag operation
struct DragEvent : public Event {
    double x;
    double y;
    double timeSeconds;

    DragEvent(double x, double y, double time) : x(x), y(y), timeSeconds(time) {}
};

// Event published when a valid audio segment selection is finalized
struct AudioSegmentSelectedEvent : public Event {
    double startTimeSeconds;
    double endTimeSeconds;

    AudioSegmentSelectedEvent(double start, double end) : startTimeSeconds(start), endTimeSeconds(end) {}
};