#include "DrawStrokeAction.h"
#include <memory>
#include <vector>
class DrawActionTracker {
private:
  std::vector<std::unique_ptr<DrawStrokeAction>> mementos;

public:
  int currentIndex = 0;

  void AddMemento(std::unique_ptr<DrawStrokeAction> memento) {
    if (mementos.size() > 0) {
      mementos.resize(currentIndex + 1);
    }

    mementos.push_back(std::move(memento));

    currentIndex = mementos.size() - 1;
    if (currentIndex < 0) {
      currentIndex = 0;
    }

    for (int i = 0; i < mementos.size() - 1; ++i) {
      if (currentIndex == i) {
        std::cout << "------------------------------" << std::endl;
      }
      std::cout << "Memento " << i << ":" << &mementos[i] << std::endl;
      if (currentIndex == i) {
        std::cout << "------------------------------" << std::endl;
      }
    }
  }

  const DrawStrokeAction &getAction(int index) {
    if (index >= 0 && index < mementos.size()) {
      currentIndex = index;
      std::cout << "Getting memento: " << index << std::endl;
      for (int i = 0; i < mementos.size() - 1; ++i) {
        if (currentIndex == i) {
          std::cout << "------------------------------" << std::endl;
        }
        std::cout << "Memento " << i << ":" << &mementos[i] << std::endl;
        if (currentIndex == i) {
          std::cout << "------------------------------" << std::endl;
        }
      }
      return *mementos[index];
    } else {
      std::cout << "Cannot find index" << std::endl;
    }
    throw std::out_of_range("Invalid Memento index");
  }

  const int getMementosSize() const { return mementos.size(); }
};
