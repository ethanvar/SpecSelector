#include "ProgressMask.h"
#include "juce_graphics/juce_graphics.h"

ProgressMask::ProgressMask() {
}

ProgressMask::~ProgressMask() {
}

void ProgressMask::updateProgress(double progress, double songLength) {
    if ((std::abs(m_currentProgress - progress) > 0.01) || (std::abs(m_currentProgress - progress) > 0.01)) {
        m_currentProgress = progress;
        m_currentSongLength = songLength;
        repaint();
    }
}

void ProgressMask::paint(juce::Graphics& g) {
    
    if (m_currentSongLength <= 0.0) return;

    std::cout << "Number of frames: " << *m_numFrames << std::endl;
    juce::Rectangle<int> bounds = getLocalBounds();
    juce::RectanglePlacement placement;
    int maskValue = (m_currentProgress/m_currentSongLength)*bounds.getWidth();
    std::cout << maskValue << std::endl;
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    juce::Image actualProgress(juce::Image::RGB, maskValue, bounds.getHeight(), true);
    g.drawImageAt(actualProgress, bounds.getX(), bounds.getY(), false);
}

void ProgressMask::setNumFrames(int& frame) {
    m_numFrames = &frame;
}

void ProgressMask::setFftSize(int size){
    m_fftSize = size;
}

void ProgressMask::resized() {

}
