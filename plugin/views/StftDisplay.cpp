#include "StftDisplay.h"
#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

StftDisplay::StftDisplay(AudioPluginAudioProcessor &p)
    : processorRef(p), spectrogram(juce::Image::RGB, 1, 1, true) {
  displayWidth = 1;
  setInterceptsMouseClicks(false, false);
  setBufferedToImage(true);
}

StftDisplay::~StftDisplay() {}

void StftDisplay::createSpectrogram(int w, int h) {
  if (w <= 0 || h <= 0)
    return;
  spectrogram = juce::Image(juce::Image::RGB, w, h, true);
  displayWidth = w;
}

void StftDisplay::initializeSpectrogram() {
  processorRef.stft.drawSpectorgramWithStft(spectrogram);
  repaint();
  processorRef.stft.strokeMap.init(spectrogram.getBounds().getWidth(),
                                   spectrogram.getBounds().getHeight());
  std::cout << "spectrogram bounds: " << spectrogram.getBounds().getHeight()
            << " & " << spectrogram.getBounds().getWidth() << std::endl;
}

void StftDisplay::updateSpectrogram(PixelUpdate &update) {
  processorRef.stft.drawOntoSpectrogram(update.pixelX, update.pixelY,
                                        spectrogram);
  repaint();
}

void StftDisplay::undoSpectrogramAction() {}

void StftDisplay::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds();

  if (bounds.isEmpty()) {
    return;
  }

  if (spectrogram.getWidth() <= 0 || spectrogram.getHeight() <= 0) {
    return;
  }

  g.drawImage(spectrogram, bounds.toFloat());
  if (processorRef.getRepaint()) {
    processorRef.setRepaint(false);
  }
}

void StftDisplay::checking() { std::cout << "CHEKCKED" << std::endl; }

void StftDisplay::resized() {
  auto bounds = getLocalBounds();

  if (bounds.isEmpty())
    return;

  createSpectrogram(bounds.getWidth(), bounds.getHeight());
}
