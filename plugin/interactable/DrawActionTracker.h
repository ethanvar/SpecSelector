#include "DrawStrokeAction.h"
#include <memory>
#include <vector>
class DrawActionTracker {
private:
  std::vector<std::unique_ptr<DrawStrokeAction>> mementos;

public:
  void AddMemento(std::unique_ptr<DrawStrokeAction> memento) {
    mementos.push_back(std::move(memento));
  }

  const DrawStrokeAction &getAction(int index) const {
    if (index >= 0 && index < mementos.size()) {
      return *mementos[index];
    }
    throw std::out_of_range("Invalid Memento index");
  }

  const int getMementosSize() const { return mementos.size(); }
};
