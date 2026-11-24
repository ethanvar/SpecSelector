#include "StrokeMap.h"

void StrokeMap::init(int width, int initialHeight) {
  std::lock_guard<std::mutex> lock(mutex_);
  width_ = width;
  height_ = initialHeight;
  typeMap_.resize(width_ * height_, 0);
  strokeIndexMap_.resize(width_ * height_);
  strokeRegistry_.clear();
}

// inline void StrokeMap::expandTo(int newHeight) {
//   if (newHeight > height_) {
//     growHeight(newHeight);
//   }
//   height_ = newHeight;
// }

void StrokeMap::growHeight(int newHeight) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto oldData = std::move(typeMap_);
  auto oldIndex = std::move(strokeIndexMap_);
  int oldHeight = height_;

  typeMap_.resize(width_ * newHeight, 0);
  strokeIndexMap_.resize(width_ * newHeight);

  // Preserve existing rows.
  for (int y = 1; y < oldHeight; ++y) {
    std::memcpy(typeMap_.data() + y * width_, oldData.data() + y * width_,
                width_ * sizeof(uint8_t));
    std::memcpy(strokeIndexMap_.data() + y * width_,
                oldIndex.data() + y * width_, width_ * sizeof(uint16_t));
  }
  height_ = newHeight;
}

void StrokeMap::addStroke(uint8_t effectType,
                          const std::vector<StrokeInfo> &points,
                          uint16_t strokeIdx) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (width_ == 0 || height_ == 0) {
    return;
  }

  StrokeEffect effect;
  effect.type = effectType;

  // Average the params over all points in the stroke.
  // (Could also use max, min, or store per-point — this is a simple heuristic.)
  for (const auto &p : points) {
    // Derive magnitude/phase from the colour (or store them directly
    // on StrokeInfo if you prefer). For now, we use the colour as
    // a simple discriminator.
    auto rgb = p.colour.getARGB();
    effect.magnitude = static_cast<float>((rgb >> 16) & 0xFF) / 255.0f;
    effect.phase = static_cast<float>((rgb >> 8) & 0xFF) / 255.0f;
    effect.radius = p.radius;
    effect.thickness = 1.0f; // or derive from pointCount
  }

  if (strokeIdx >= strokeRegistry_.size()) {
    strokeRegistry_.resize(strokeIdx + 1);
  }
  strokeRegistry_[strokeIdx] = effect;

  // Paint the stroke onto the map.
  for (const auto &p : points) {
    for (int i = 0; i < p.radius; ++i) {
      int x1 = p.x + i;
      int y1 = p.y + i;
      if (x1 >= 0 && x1 < width_ && y1 >= 0 && y1 < height_) {
        int idx2d = y1 * width_ + x1;
        typeMap_[idx2d] = effectType;
        strokeIndexMap_[idx2d].push_back(strokeIdx);
      }

      int x2 = p.x - i;
      int y2 = p.y - i;
      if (x2 >= 0 && x2 < width_ && y2 >= 0 && y2 < height_) {
        int idx2d = y2 * width_ + x2;
        typeMap_[idx2d] = effectType;
        strokeIndexMap_[idx2d].push_back(strokeIdx);
      }
    }
  }
}

void removeStroke(uint8_t effectType, const std::vector<StrokeInfo> &points) {}

uint8_t StrokeMap::getStrokeType(int x, int y) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return 0;
  }
  return typeMap_[y * width_ + x];
}

std::vector<uint16_t> StrokeMap::getStrokeIndices(int x, int y) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return {};
  }
  return static_cast<std::vector<uint16_t>>(strokeIndexMap_[y * width_ + x]);
}

const std::vector<const StrokeEffect *>
StrokeMap::getStrokeEffect(int x, int y) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return {};
  }
  const auto &indices = strokeIndexMap_[y * width_ + x];

  std::vector<const StrokeEffect *> results;
  results.reserve(indices.size());

  for (uint16_t id : indices) {
    if (id < strokeRegistry_.size()) {
      results.push_back(&strokeRegistry_[id]);
    }
  }

  return results;
}

std::vector<StrokeMap::PixelEntries>
StrokeMap::getEntriesForRow(int timeRow) const {
  std::vector<PixelEntries> result;
  std::lock_guard<std::mutex> lock(mutex_);
  if (timeRow < 0 || timeRow >= height_ || width_ == 0) {
    return result;
  }
  result.reserve(static_cast<size_t>(width_));
  int rowOffset = timeRow * width_;
  for (int x = 0; x < width_; ++x) {
    uint8_t t = typeMap_[rowOffset + x];
    if (t != 0) {
      result.push_back(
          {x, t,
           static_cast<std::vector<uint16_t>>(strokeIndexMap_[rowOffset + x])});
    }
  }
  return result;
}

void StrokeMap::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  width_ = 0;
  height_ = 0;
  typeMap_.clear();
  strokeIndexMap_.clear();
  strokeRegistry_.clear();
}
