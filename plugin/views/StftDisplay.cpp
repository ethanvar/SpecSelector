#include "StftDisplay.h"
#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

StftDisplay::StftDisplay(AudioPluginAudioProcessor& p) : processorRef(p), spectrogram(juce::Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true) {
    displayWidth = getLocalBounds().getWidth();
    setInterceptsMouseClicks(false, false);
    setBufferedToImage(true);
}

StftDisplay::~StftDisplay() {
}

void StftDisplay::updateSpectrogram() {
    processorRef.stft.drawSpectorgramWithStft(spectrogram);
    repaint();
}

void StftDisplay::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    std::cout << bounds.getHeight() << "And y: " << bounds.getWidth() << std::endl;

    if (bounds.isEmpty()) {
        return;
    }

    g.drawImage(spectrogram, bounds.toFloat());
    if (processorRef.getRepaint()) {
        processorRef.setRepaint(false);
    }
}

void StftDisplay::checking() {
    std::cout << "CHEKCKED" << std::endl;
}

void StftDisplay::resized() {
    auto bounds = getLocalBounds();

    if (bounds.isEmpty()) return;

    spectrogram = juce::Image(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);

}
