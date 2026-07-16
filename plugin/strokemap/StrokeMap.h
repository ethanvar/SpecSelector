#pragma once

#include "../interactable/DrawStrokeAction.h"
#include <cstdint>
#include <vector>

// Represents the effect parameters associated with a single stroke
struct StrokeEffect {
  uint8_t type;
  float magnitude;
  float phase;
  int radius;
  juce::Colour colour;
  float thickness;
};

// StrokeMap — a 2D pixel-indexed map of spectrogram strokes.
//
// Dimensions:
//   width  = FFT bins (numBins = fftSize/2 + 1 = 513 for fftSize=1024)
//   height = time frames (grows dynamically as audio is processed)
//
// Each cell (x, y) stores:
//   - typeMap[y * width + x]   → stroke type ID (uint8_t)
//   - strokeIndexMap[y * width + x] → index into strokeRegistry
//
// A two-layer lookup lets you query "what type is at this pixel?"
// without scanning a list, while still keeping per-stroke params
// (radius, colour, thickness) available via the registry.
class StrokeMap {
public:
  StrokeMap() = default;
  ~StrokeMap() = default;

  // Must be called before any other method.
  // width = numBins from FFT (e.g., 513 for fftSize=1024).
  // height starts at 0 and expands as audio frames arrive.
  void init(int width, int initialHeight = 0);

  // ------------------------------------------------------------------
  // Adding strokes
  // ------------------------------------------------------------------

  // Add a stroke: paint its points onto the map at the given time-frame row.
  // - effectType: a user-defined category (e.g. 1 = delay, 2 = reverb, 3 =
  // filter)
  // - points: the StrokeInfo points from a DrawingStroke
  // - timeRow: the y-coordinate in the spectrogram (usually segmentsPassed)
  // If a pixel is already occupied, the new stroke overwrites it
  // (latest-wins semantics).
  void addStroke(uint8_t effectType, const std::vector<StrokeInfo> &points);

  // ------------------------------------------------------------------
  // Lookup — call from the audio thread (processSpectrum)
  // ------------------------------------------------------------------

  // Returns the stroke type at (x, y). 0 means no stroke painted.
  uint8_t getStrokeType(int x, int y) const;

  // Returns the stroke registry index at (x, y). 0 means no stroke.
  int getStrokeIndex(int x, int y) const;

  // Returns a pointer to the registry entry for the stroke at (x, y).
  // Returns nullptr if no stroke is painted at that position.
  const StrokeEffect *getStrokeEffect(int x, int y) const;

  // Convenience: returns all strokes painted at a given time-row.
  // Useful for batch processing or rebuilding the map.
  struct PixelEntry {
    int x;
    uint8_t type;
    int index;
  };
  std::vector<PixelEntry> getEntriesForRow(int timeRow) const;

  // ------------------------------------------------------------------
  // Dynamic sizing
  // ------------------------------------------------------------------

  // Call this each time a new frame is processed to grow the map.
  // height is the new desired number of frames.
  // void expandTo(int newHeight);

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  // ------------------------------------------------------------------
  // Utility
  // ------------------------------------------------------------------

  void clear();

private:
  int width_ = 0;
  int height_ = 0;

  // Flat 1D arrays, indexed as map[y * width_ + x]
  std::vector<uint8_t> typeMap_;
  std::vector<uint16_t> strokeIndexMap_;
  std::vector<StrokeEffect> strokeRegistry_;

  // Next free index in strokeRegistry_.
  uint16_t nextRegistryIndex_ = 1; // 0 = "no entry"

  void growHeight(int newHeight);
};
