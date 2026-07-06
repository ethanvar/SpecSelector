#pragma once

#include "StrokeSnapshot.h"
#include "juce_data_structures/juce_data_structures.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
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

class DrawStrokeAction : public juce::UndoableAction {
public:
  DrawStrokeAction(std::vector<DrawingStroke> &strokeList,
                   juce::Image &spectrogram,
                   juce::Component &componentToRepaint,
                   const DrawingStroke &newStroke,
                   DrawOntoSpectrogramFn drawFn);

  ~DrawStrokeAction();

  bool perform() override;

  bool undo() override;

  int getSizeInUnits() override;

  juce::Image returnActionImage();

private:
  std::vector<DrawingStroke> &strokes;
  juce::Image &spectrogram;
  DrawOntoSpectrogramFn drawFn;
  juce::Component &owner;
  DrawingStroke stroke;
  size_t strokeIndex;
  juce::Image spectrogramSnapshot;
};
