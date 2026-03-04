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

    if (bounds.isEmpty()) {
        std::cout << "NO bounds!" << std::endl;
        return;
    }

    g.fillAll (juce::Colours::black);
    
    processorRef.stft.drawSpectorgramWithStft(spectrogram);

    g.drawImage(spectrogram, bounds.toFloat());
    processorRef.setRepaint(false);
}

void StftDisplay::checking() {
    std::cout << "CHEKCKED" << std::endl;
}

void StftDisplay::resized() {
    auto bounds = getLocalBounds();

    if (bounds.isEmpty()) return;

    spectrogram = juce::Image(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);

}
