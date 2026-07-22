#include "Mediator.h"
#include "StftDisplay.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

Mediator::Mediator(AudioPluginAudioProcessor &p)
    : processorRef(p), m_stftDisplay(p) {
  addMouseListener(this, true);
  addAndMakeVisible(m_stftDisplay);
  // setSize (1253, 768);
  // setSize (500, 768);
  setWantsKeyboardFocus(true);
  addAndMakeVisible(m_mask);
  m_mask.setFftSize(p.fftSize);
  m_mask.setNumFrames(p.stft.getNumFrames());
}

Mediator::~Mediator() {}

void Mediator::mouseDown(const juce::MouseEvent &event) {
  grabKeyboardFocus();
  std::cout << "CLICKED IMAGE AT (relative?): " << event.getMouseDownX() << ", "
            << event.getMouseDownY() << std::endl;

  currentStrokePoints.clear();
  currentStrokePoints.emplace_back(
      event.getEventRelativeTo(this).getMouseDownX(),
      event.getEventRelativeTo(this).getMouseDownY());
}

void Mediator::mouseDrag(const juce::MouseEvent &event) {
  int pointOnScreenX =
      event.getEventRelativeTo(this).getMouseDownX() +
      event.getEventRelativeTo(this).getDistanceFromDragStartX();
  int pointOnScreenY =
      event.getEventRelativeTo(this).getMouseDownY() +
      event.getEventRelativeTo(this).getDistanceFromDragStartY();

  currentStrokePoints.emplace_back(pointOnScreenX, pointOnScreenY);

  processorRef.stft.drawOntoSpectrogram(pointOnScreenX, pointOnScreenY,
                                        m_stftDisplay.spectrogram);

  m_stftDisplay.repaint();
}

void Mediator::mouseUp(const juce::MouseEvent &event) {
  if (!currentStrokePoints.empty()) {
    DrawingStroke stroke;
    stroke.colour = juce::Colour(100, 0, 0);
    stroke.thickness = 1.0f;

    for (const auto &pt : currentStrokePoints) {
      StrokeInfo info;
      info.x = pt.x;
      info.y = pt.y;
      info.colour = juce::Colour(100, 0, 0);
      info.radius = 20;
      stroke.strokeInfo.push_back(info);
      stroke.coordinates.push_back(pt);
    }
    addStrokeToSpectrogram(stroke, currentStrokePoints);
    currentStrokePoints.clear();
  }
}

bool Mediator::keyPressed(const juce::KeyPress &key,
                          juce::Component *originatingComponent) {
  std::cout << "Key pressed: " << key.getTextCharacter() << std::endl;
  if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'z') {
    undoLastStroke();
    return true;
  }
  return false;
}

void Mediator::addStrokeToSpectrogram(
    const DrawingStroke &stroke, const std::vector<StrokeCoordinate> &coords) {
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "adding stroke to spectrogram" << std::endl;
  DrawingStroke newStroke = stroke;

  auto drawFn = [stft = &processorRef.stft](const DrawingStroke &s,
                                            juce::Image &img) {
    stft->drawOntoSpectrogram(s, img);
  };

  auto action = std::make_unique<DrawStrokeAction>(m_stftDisplay.spectrogram,
                                                   newStroke, drawFn);
  processorRef.stft.strokeMap.addStroke(1, newStroke.strokeInfo);
  actionManager.AddMemento(std::move(action));
}

void Mediator::undoLastStroke() {
  std::cout << std::endl;
  std::cout << std::endl;

  if (actionManager.currentIndex >= 0) {
    actionManager.currentIndex = actionManager.currentIndex - 1;

    if (actionManager.currentIndex == -1) {
      m_stftDisplay.initializeSpectrogram();
    } else {
      const auto &lastAction =
          actionManager.getAction(actionManager.currentIndex);
      m_stftDisplay.spectrogram = lastAction.returnActionImage().createCopy();
    }
    m_stftDisplay.repaint();
  }
}

void Mediator::redoLastStroke() {
  std::cout << std::endl;
  std::cout << std::endl;
  if (actionManager.getMementosSize() > 0 &&
      (actionManager.currentIndex + 1 <= actionManager.getMementosSize())) {
    DrawStrokeAction redoAction =
        actionManager.getAction(actionManager.currentIndex + 1);
    m_stftDisplay.spectrogram = redoAction.returnActionImage().createCopy();
    m_stftDisplay.repaint();
  }
}

void Mediator::initializeStftDisplay() {
  m_stftDisplay.initializeSpectrogram();
  repaint();
}

void Mediator::updateMaskProgressWithSegments(int segmentsPassed) {
  m_mask.updateProgressWithSegments(segmentsPassed);
  repaint();
}

void Mediator::paint(juce::Graphics &g) {
  m_stftDisplay.paint(g);
  m_mask.paint(g);
}

void Mediator::resized() {
  m_stftDisplay.setBounds(getLocalBounds());
  m_mask.setBounds(getLocalBounds());
  m_stftDisplay.resized();
  m_mask.resized();
}
