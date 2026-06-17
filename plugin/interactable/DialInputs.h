struct DialInputs {
  int magnitude;
  int phase;
  DialInputs() : phase(50), magnitude(50) {};

  void setMagnitude(int magnitude_) { magnitude = magnitude_; }
  void setPhase(int phase_) { magnitude = phase_; }
};
