#pragma once

#include "juce_graphics/juce_graphics.h"

struct StrokeCoordinate {
  int x;
  int y;

  StrokeCoordinate() : x(0), y(0) {}
  StrokeCoordinate(int x_, int y_) : x(x_), y(y_) {}
};

struct StrokeSnapshot {
  // juce::Image spectrogramImage;
  std::vector<StrokeCoordinate> coordinates;
  int strokeCount;
};
