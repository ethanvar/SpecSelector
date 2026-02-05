#include "ProgressMask.h"
#include "juce_graphics/juce_graphics.h"

ProgressMask::ProgressMask(): progressMask(juce::Image::RGB, 1200, 600, true) {
    progressMask.clear(getLocalBounds(), juce::Colours::transparentBlack);
}

ProgressMask::~ProgressMask() {
}

void ProgressMask::updateProgress(double progress, double songLength) {
    if ((std::abs(currentProgress - progress) > 0.01) || (std::abs(currentProgress - progress) > 0.01)) {
        currentProgress = progress;
        currentSongLength = songLength;
        repaint();
    }
}

void ProgressMask::paint(juce::Graphics& g) {
    if (currentSongLength <= 0.0) return;

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::RectanglePlacement placement;

    int maskValue = (currentProgress/currentSongLength) * bounds.getWidth();

    g.drawImageWithin(progressMask, bounds.getX(), bounds.getY(), maskValue, bounds.getHeight(), placement, false);
}

void ProgressMask::resized() {

}
