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
    int maskValue = (currentProgress/fftSize);
    std::cout << currentProgress << std::endl;
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    juce::Image actualProgress(juce::Image::RGB, maskValue, bounds.getHeight(), true);
    g.drawImageAt(actualProgress, bounds.getX(), bounds.getY(), false);
}

void ProgressMask::setFftSize(int size){
    fftSize = size;
}

void ProgressMask::resized() {

}
