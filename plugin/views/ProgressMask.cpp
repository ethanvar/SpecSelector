#include "ProgressMask.h"
#include "juce_graphics/juce_graphics.h"

ProgressMask::ProgressMask() {
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
    juce::Image actualProgress(juce::Image::RGB, maskValue, bounds.getHeight(), true);
    g.drawImageAt(actualProgress, bounds.getX(), bounds.getY(), false);
}

void ProgressMask::resized() {

}
