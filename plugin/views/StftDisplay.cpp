#include "StftDisplay.h"
#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

StftDisplay::StftDisplay(AudioPluginAudioProcessor& p) : processorRef(p) {
  std::cout << "StftDisplay Created!" << std::endl;  
  setInterceptsMouseClicks(false, false);
}

StftDisplay::~StftDisplay() {
}

void StftDisplay::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    std::cout << "painting Stft display" << std::endl;

    if (bounds.isEmpty())
        return;

    g.fillAll (juce::Colours::black);
    
    g.drawImageWithin (spectrogram, 0, 0, bounds.getWidth(), bounds.getHeight(), juce::RectanglePlacement::stretchToFit);

    processorRef.stft.drawSpectorgramWithStft(spectrogram);

    g.drawImage(spectrogram, bounds.toFloat());
}

void StftDisplay::resized() {
    if (processorRef.stft.hasData()) {
        repaint();
    }
}

//void StftDisplay::updateProgress(double progress, double songLength) {
//    if (std::abs(currentProgress - progress) > 0.01) {
//        currentProgress = progress;
//        currentSongLength = songLength;
//        repaint();
//    }
//}
