#include "Mediator.h"
#include "PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

Mediator::Mediator(AudioPluginAudioProcessor &p) : processorRef(p), m_stftDisplay(p){
    addAndMakeVisible(m_stftDisplay);
    // setSize (1253, 768);
    // setSize (500, 768);
    
    addAndMakeVisible(m_mask);
    m_mask.setFftSize(p.fftSize);
    m_mask.setNumFrames(p.stft.getNumFrames());
}

Mediator::~Mediator() {
}

void Mediator::updatetStftDisplay() {
    m_stftDisplay.updateSpectrogram();
    repaint();
}

void Mediator::updateMaskProgress(double progress, double songLength) {
    if ((std::abs(m_mask.getCurrentProgress() - progress) > 0.01) || (std::abs(m_mask.getCurrentProgress() - progress) > 0.01)) {
        m_mask.updateProgress(progress, songLength);
        repaint();
    }
}

void Mediator::paint(juce::Graphics &g) {
    m_stftDisplay.paint(g);
    m_mask.paint(g);
}

void Mediator::resized() {
    m_stftDisplay.setBounds(getLocalBounds());
    m_mask.setBounds(getLocalBounds());
    m_stftDisplay.resized();
    m_mask.resized();
}
