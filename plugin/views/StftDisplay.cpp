#include "StftDisplay.h"
#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

StftDisplay::StftDisplay(AudioPluginAudioProcessor& p) : processorRef(p), spectrogram(juce::Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true) {
  std::cout << "StftDisplay Created!" << std::endl;  
  setInterceptsMouseClicks(false, false);
  setBufferedToImage(true);
}

StftDisplay::~StftDisplay() {
}

void StftDisplay::paint(juce::Graphics& g) {
    if (!processorRef.getRepaint()) return;
    auto bounds = getLocalBounds();

    std::cout << "painting Stft display" << std::endl;

    if (bounds.isEmpty())
        return;

    g.fillAll (juce::Colours::black);

    //juce::Image newImage(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);
    //g.drawImageWithin (spectrogram, 0, 0, bounds.getWidth(), bounds.getHeight(), juce::RectanglePlacement::stretchToFit);
    
    processorRef.stft.drawSpectorgramWithStft(spectrogram);

    g.drawImage(spectrogram, bounds.toFloat());
    processorRef.setRepaint(false);
}

void StftDisplay::resized() {
    auto bounds = getLocalBounds();

    if (bounds.isEmpty()) return;

    spectrogram = juce::Image(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);
    //if (processorRef.stft.hasData()) {
    //    repaint();
    //}
}

//void StftDisplay::updateProgress(double progress, double songLength) {
//    if (std::abs(currentProgress - progress) > 0.01) {
//        currentProgress = progress;
//        currentSongLength = songLength;
//        repaint();
//    }
//}
