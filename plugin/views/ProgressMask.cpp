#include "ProgressMask.h"

ProgressMask::ProgressMask(): progressMask(juce::Image::RGB, 1200, 600, true) {
}

ProgressMask::~ProgressMask() {
}

void ProgressMask::setMask(juce::Graphics& g, juce::Rectangle<int>& r, double& progress, double& songLength)
{
    juce::RectanglePlacement placement;
    int maskValue = (progress/songLength) * r.getWidth();  
    g.drawImageWithin(progressMask, r.getX(), r.getY(), maskValue, r.getHeight(), placement, false);
}
