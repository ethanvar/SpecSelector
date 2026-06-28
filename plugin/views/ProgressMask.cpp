#include "ProgressMask.h"
#include "juce_graphics/juce_graphics.h"

ProgressMask::ProgressMask() {}

ProgressMask::~ProgressMask() {}

void ProgressMask::updateProgress(double progress, double songLength) {
  if (std::abs(m_currentProgress - progress) > 0.01) {
    m_currentProgress = progress;
    m_currentSongLength = songLength;
    repaint();
  }
}

void ProgressMask::updateProgressWithSegments(int segmentsPassed) {
  std::cout << "segmentsPassed: " << segmentsPassed << std::endl;
  m_stftSegmentsPassed = segmentsPassed;
  repaint();
}

void ProgressMask::paint(juce::Graphics &g) {

  if (m_currentSongLength <= 0.0)
    return;

  // std::cout << "Number of frames: " << *m_numFrames << std::endl;
  juce::Rectangle<int> bounds = getLocalBounds();
  juce::RectanglePlacement placement;
  // int currFramesPassed = ((double) m_currentProgress / double (m_fftSize*2));
  // std::cout << "currFramesPassed: " << currFramesPassed << std::endl;
  // double percentOfTotalFramesPassed = (double) currFramesPassed / (double)
  // *m_numFrames; std::cout << "Progress over frames: " <<
  // percentOfTotalFramesPassed << std::endl; std::cout << "Current song length:
  // " << m_currentSongLength << std::endl;
  // double maskValue = ((m_currentProgress) / m_currentSongLength) *
  //                   static_cast<double>(bounds.getWidth());
  double maskValue = m_stftSegmentsPassed;
  // double maskValue = (percentOfTotalFramesPassed) * bounds.getWidth();
  // std::cout << maskValue << std::endl;
  g.setColour(juce::Colours::black.withAlpha(0.5f));
  juce::Image actualProgress(juce::Image::RGB, maskValue, bounds.getHeight(),
                             true);
  g.drawImageAt(actualProgress, bounds.getX(), bounds.getY(), false);
}

void ProgressMask::setNumFrames(int &frame) { m_numFrames = &frame; }

void ProgressMask::setFftSize(int size) { m_fftSize = size; }

double ProgressMask::getCurrentProgress() { return m_currentProgress; }

double ProgressMask::getCurrentSongLength() { return m_currentSongLength; }

void ProgressMask::resized() {}
