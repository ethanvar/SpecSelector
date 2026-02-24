#include "Mediator.h"
#include "PluginProcessor.h"

Mediator::Mediator(AudioPluginAudioProcessor &p) : processorRef(p), m_stftDisplay(p){
   m_stftDisplay.setBounds(getLocalBounds());
   addAndMakeVisible(m_stftDisplay);
   
   m_mask.setBounds(getLocalBounds());
   m_mask.setFftSize(p.fftSize);
   m_mask.setNumFrames(p.stft.getNumFrames());
   addAndMakeVisible(m_mask);
}

Mediator::~Mediator() {
}
