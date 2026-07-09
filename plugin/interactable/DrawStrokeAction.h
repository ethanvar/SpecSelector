#pragma once

#include "StrokeSnapshot.h"
#include "juce_graphics/juce_graphics.h"
#include <functional>
#include <vector>

struct StrokeInfo {
  int x;
  int y;
  juce::Colour colour;
  int radius;
};

struct DrawingStroke {
  juce::Path path;
  juce::Colour colour;
  float thickness;
  std::vector<StrokeCoordinate> coordinates;
  std::vector<StrokeInfo> strokeInfo;
  juce::Image spectrogramSnapshot;
};

using DrawOntoSpectrogramFn =
    std::function<void(const DrawingStroke &, juce::Image &)>;

class DrawStrokeAction {
public:
  DrawStrokeAction(juce::Image &_spectrogram, const DrawingStroke &newStroke,
                   DrawOntoSpectrogramFn drawFn);

  ~DrawStrokeAction();

  juce::Image returnActionImage();

private:
  juce::Image spectrogram;
  DrawOntoSpectrogramFn drawFn;
  DrawingStroke stroke;
  size_t strokeIndex;
};
