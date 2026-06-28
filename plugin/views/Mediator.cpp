#include "Mediator.h"
#include "StftDisplay.h"
#include "juce_graphics/juce_graphics.h"

Mediator::Mediator(AudioPluginAudioProcessor &p)
    : processorRef(p), m_stftDisplay(p) {
  addMouseListener(this, true);
  addAndMakeVisible(m_stftDisplay);
  // setSize (1253, 768);
  // setSize (500, 768);

  addAndMakeVisible(m_mask);
  m_mask.setFftSize(p.fftSize);
  m_mask.setNumFrames(p.stft.getNumFrames());
}

Mediator::~Mediator() {}

void Mediator::mouseDown(const juce::MouseEvent &event) {
  // This triggers when the image is clicked
  std::cout << "CLICKED IMAGE AT (relative?): " << event.getMouseDownX() << ", "
            << event.getMouseDownY() << std::endl;

  processorRef.stft.drawOntoSpectrogram(
      event.getEventRelativeTo(this).getMouseDownX(),
      event.getEventRelativeTo(this).getMouseDownY(),
      m_stftDisplay.spectrogram);
};

void Mediator::mouseDrag(const juce::MouseEvent &event) {
  int pointOnScreenX =
      event.getEventRelativeTo(this).getMouseDownX() +
      event.getEventRelativeTo(this).getDistanceFromDragStartX();
  int pointOnScreenY =
      event.getEventRelativeTo(this).getMouseDownY() +
      event.getEventRelativeTo(this).getDistanceFromDragStartY();

  PixelUpdate spectrogramUpdate;
  spectrogramUpdate.pixelX = pointOnScreenX;
  spectrogramUpdate.pixelY = pointOnScreenY;
  spectrogramUpdate.colour = juce::Colours::red;

  m_stftDisplay.updateSpectrogram(spectrogramUpdate);
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
