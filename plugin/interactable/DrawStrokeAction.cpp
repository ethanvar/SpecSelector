#include "DrawStrokeAction.h"

DrawStrokeAction::DrawStrokeAction(std::vector<DrawingStroke> &strokeList,
                                   juce::Image &spectrogram,
                                   juce::Component &componentToRepaint,
                                   const DrawingStroke &newStroke,
                                   DrawOntoSpectrogramFn drawFn)
    : strokes(strokeList), spectrogram(spectrogram), drawFn(std::move(drawFn)),
      owner(componentToRepaint), stroke(newStroke),
      strokeIndex(strokeList.size()) {}

DrawStrokeAction::~DrawStrokeAction() {}

bool DrawStrokeAction::perform() {
  spectrogramSnapshot = spectrogram;

  drawFn(stroke, spectrogram);

  strokes.push_back(stroke);

  owner.repaint();
  std::cout << "DrawStrokeAction::This action has been performed" << std::endl;
  return true;
}

bool DrawStrokeAction::undo() {
  std::cout << "DrawStrokeAction: Undoing action" << std::endl;
  if (strokeIndex < strokes.size()) {
    strokes.erase(strokes.begin() + static_cast<int>(strokeIndex));
  }

  if (spectrogramSnapshot.isValid()) {
    spectrogram = spectrogramSnapshot;
  }

  owner.repaint();

  return true;
}

juce::Image DrawStrokeAction::returnActionImage() { return spectrogram; }
