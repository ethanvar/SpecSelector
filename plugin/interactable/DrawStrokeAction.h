#include "juce_data_structures/juce_data_structures.h"
#include "juce_graphics/juce_graphics.h"

struct DrawingStroke {
  juce::Path path;
  juce::Colour colour;
  float thickness;
};

class DrawStrokeAction : public juce::UndoableAction {
public:
  DrawStrokeAction();
  ~DrawStrokeAction();

private:
};
