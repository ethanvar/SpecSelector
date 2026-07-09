#pragma once

#include "../PluginProcessor.h"
#include "../interactable/DrawActionTracker.h"
#include "../interactable/DrawStrokeAction.h"
#include "ProgressMask.h"
#include "StftDisplay.h"
#include "juce_gui_basics/juce_gui_basics.h"

class Mediator : public juce::Component {
public:
  Mediator(AudioPluginAudioProcessor &p);
  ~Mediator();

  void resized() override;
  void paint(juce::Graphics &g) override;
  void initializeStftDisplay();
  void updateMaskProgressWithSegments(int segmentsPassed);
  void mouseDown(const juce::MouseEvent &event) override;
  void mouseDrag(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;
  bool keyPressed(const juce::KeyPress &key,
                  juce::Component *originatingComponent);

  void addStrokeToSpectrogram(const DrawingStroke &stroke,
                              const std::vector<StrokeCoordinate> &coords);
  void undoLastStroke();
  void redoLastStroke();

private:
  AudioPluginAudioProcessor &processorRef;
  ProgressMask m_mask;
  StftDisplay m_stftDisplay;

  std::vector<DrawingStroke> drawnStrokes;
  std::vector<StrokeCoordinate> currentStrokePoints;
  juce::UndoManager undoManager;
  DrawActionTracker actionManager;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mediator)
};
