#include "StftBuffers.h"

StftBuffers::StftBuffers(const juce::dsp::WindowingFunction<float> &w,
                         const juce::dsp::FFT &f, DialInputs &inputs,
                         StrokeMap *map, int &segsPassed)
    : window(w), forwardFFT(f), inputsRef(inputs), strokeMap(map),
      segmentsPassed(segsPassed) {}

StftBuffers::~StftBuffers() {}

void StftBuffers::reset() {
  count = 0;
  pos = 0;
  std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
  std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
}

float StftBuffers::processSample(float sample, bool bypassed) {
  inputFifo[pos] = sample;
  float outputSample = outputFifo[pos];
  // float outputSample = 0;
  outputFifo[pos] = 0.0f;

  pos += 1;
  if (pos == fftSize) {
    pos = 0;
  }

  count += 1;
  if (count == hopSize) {
    count = 0;
    processFrame(bypassed);
  }

  return outputSample;
}

void StftBuffers::processFrame(bool bypassed) {
  const float *inputPtr = inputFifo.data();
  float *fftPtr = fftData.data();

  std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
  if (pos > 0) {
    std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
  }

  window.multiplyWithWindowingTable(fftPtr, fftSize);

  if (!bypassed) {
    forwardFFT.performRealOnlyForwardTransform(fftPtr, true);
    processSpectrum(fftPtr, numBins);
    forwardFFT.performRealOnlyInverseTransform(fftPtr);
  }

  window.multiplyWithWindowingTable(fftPtr, fftSize);

  for (int i = 0; i < fftSize; ++i) {
    fftPtr[i] *= windowCorrection;
  }

  for (int i = 0; i < pos; ++i) {
    outputFifo[i] += fftData[i + fftSize - pos];
  }
  for (int i = 0; i < fftSize - pos; ++i) {
    outputFifo[i + pos] += fftData[i];
  }
}

int fftIndextoY(int fftIndex, int imageHeight, int m_fftSize) {
  // Guard against edge cases
  if (fftIndex <= 0)
    return imageHeight - 1; // Highest frequency → bottom
  if (fftIndex >= m_fftSize / 2)
    return 0; // Lowest frequency → top

  float skewedProportionY = (float)fftIndex / (float(m_fftSize) / 2.0f);
  float proportionY = std::pow(1.0f - skewedProportionY, 5.0f);

  return juce::jlimit(0, imageHeight - 1,
                      static_cast<int>(proportionY * (float)imageHeight));
}

void StftBuffers::processSpectrum(float *data, int numBins) {
  std::complex<float> *cdata = reinterpret_cast<std::complex<float> *>(data);
  int xCoordinate = segmentsPassed;

  for (int i = 0; i < numBins; ++i) {
    int yCoordinate = fftIndextoY(i, 360, numBins);
    float magnitude = std::abs(cdata[i]);
    float phase = std::arg(cdata[i]);
    // std::cout << inputsRef.magnitude << std::endl;
    //  This is where you'd do your spectral processing..
    //
    // if (i > 50) {
    //  magnitude *= (float(0.01) * inputsRef.magnitude);
    //}

    if (strokeMap->getStrokeIndex(xCoordinate, yCoordinate) != 0) {
      magnitude *= (float(0.01) * inputsRef.magnitude);
    }

    // magnitude *= (float(0.01) * inputsRef.magnitude);
    phase *= inputsRef.phase;
    cdata[i] = std::polar(magnitude, phase);
  }
}
