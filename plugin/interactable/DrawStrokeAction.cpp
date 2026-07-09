#include "DrawStrokeAction.h"

DrawStrokeAction::DrawStrokeAction(juce::Image &_spectrogram,
                                   const DrawingStroke &newStroke,
                                   DrawOntoSpectrogramFn drawFn)
    : drawFn(std::move(drawFn)), stroke(newStroke) {
  spectrogram = _spectrogram.createCopy();
}

DrawStrokeAction::~DrawStrokeAction() {};
juce::Image DrawStrokeAction::returnActionImage() { return spectrogram; }
