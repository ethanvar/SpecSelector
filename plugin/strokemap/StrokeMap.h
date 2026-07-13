#pragma once

#include "../interactable/DrawStrokeAction.h"
#include <cstdint>
#include <optional>
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
  void addStroke(uint8_t effectType, const std::vector<StrokeInfo> &points,
                 int timeRow);

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

  // Clear everything. Call on undo or when loading a new file.
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

// -----------------------------------------------------------------------
// Inline implementations
// -----------------------------------------------------------------------

inline void StrokeMap::init(int width, int initialHeight) {
  width_ = width;
  height_ = initialHeight;
  typeMap_.resize(width_ * height_, 0);
  strokeIndexMap_.resize(width_ * height_, 0);
  strokeRegistry_.clear();
  nextRegistryIndex_ = 1;
}

// inline void StrokeMap::expandTo(int newHeight) {
//   if (newHeight > height_) {
//     growHeight(newHeight);
//   }
//   height_ = newHeight;
// }

inline void StrokeMap::growHeight(int newHeight) {
  auto oldData = std::move(typeMap_);
  auto oldIndex = std::move(strokeIndexMap_);
  int oldHeight = height_;

  typeMap_.resize(width_ * newHeight, 0);
  strokeIndexMap_.resize(width_ * newHeight, 0);

  // Preserve existing rows.
  for (int y = 0; y < oldHeight; ++y) {
    std::memcpy(typeMap_.data() + y * width_, oldData.data() + y * width_,
                width_ * sizeof(uint8_t));
    std::memcpy(strokeIndexMap_.data() + y * width_,
                oldIndex.data() + y * width_, width_ * sizeof(uint16_t));
  }
  height_ = newHeight;
}

inline void StrokeMap::addStroke(uint8_t effectType,
                                 const std::vector<StrokeInfo> &points,
                                 int timeRow) {
  if (timeRow < 0 || timeRow >= height_ || width_ == 0) {
    return;
  }

  // Register a new StrokeEffect entry in the registry.
  StrokeEffect effect;
  effect.type = effectType;

  // Average the params over all points in the stroke.
  // (Could also use max, min, or store per-point — this is a simple heuristic.)
  float magSum = 0, phaseSum = 0, thickSum = 0;
  int radiusSum = 0;
  int pointCount = static_cast<int>(points.size());

  for (const auto &p : points) {
    // Derive magnitude/phase from the colour (or store them directly
    // on StrokeInfo if you prefer). For now, we use the colour as
    // a simple discriminator.
    auto rgb = p.colour.getARGB();
    effect.magnitude = static_cast<float>((rgb >> 16) & 0xFF) / 255.0f;
    effect.phase = static_cast<float>((rgb >> 8) & 0xFF) / 255.0f;
    effect.radius = p.radius;
    effect.thickness = 1.0f; // or derive from pointCount
    thickSum += effect.thickness;
  }

  auto idx = nextRegistryIndex_++;

  // If the registry slot already exists from a previous call, update it.
  if (idx < strokeRegistry_.size()) {
    strokeRegistry_[idx] = effect;
  } else {
    strokeRegistry_.push_back(effect);
  }

  // Paint the stroke onto the map.
  for (const auto &p : points) {
    if (p.x < 0 || p.x >= width_ || timeRow < 0 || timeRow >= height_) {
      continue; // skip out-of-bounds points
    }
    int idx2d = timeRow * width_ + p.x;
    typeMap_[idx2d] = effectType;
    strokeIndexMap_[idx2d] = static_cast<uint16_t>(idx);
  }
}

inline uint8_t StrokeMap::getStrokeType(int x, int y) const {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return 0;
  }
  return typeMap_[y * width_ + x];
}

inline int StrokeMap::getStrokeIndex(int x, int y) const {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return 0;
  }
  return static_cast<int>(strokeIndexMap_[y * width_ + x]);
}

inline const StrokeEffect *StrokeMap::getStrokeEffect(int x, int y) const {
  int idx = getStrokeIndex(x, y);
  if (idx == 0 || idx >= static_cast<int>(strokeRegistry_.size())) {
    return nullptr;
  }
  return &strokeRegistry_[idx];
}

inline std::vector<StrokeMap::PixelEntry>
StrokeMap::getEntriesForRow(int timeRow) const {
  std::vector<PixelEntry> result;
  if (timeRow < 0 || timeRow >= height_ || width_ == 0) {
    return result;
  }
  result.reserve(static_cast<size_t>(width_));
  int rowOffset = timeRow * width_;
  for (int x = 0; x < width_; ++x) {
    uint8_t t = typeMap_[rowOffset + x];
    if (t != 0) {
      result.push_back(
          {x, t, static_cast<int>(strokeIndexMap_[rowOffset + x])});
    }
  }
  return result;
}

inline void StrokeMap::clear() {
  width_ = 0;
  height_ = 0;
  typeMap_.clear();
  strokeIndexMap_.clear();
  strokeRegistry_.clear();
  nextRegistryIndex_ = 1;
}
