#include "observer.h"

namespace TUMVis {

    Observer::Observer() {

    }

    Observer::~Observer() {

    }

    Observable::Observable() {

    }

    Observable::~Observable() {

    }

    void Observable::addObserver(Observer* o) const {
        _observers.insert(o);
    }

    void Observable::removeObserver(Observer* o) const {
        _observers.erase(o);
    }

    void Observable::notifyObservers() const {
        for (std::set<Observer*>::iterator it = _observers.begin(); it != _observers.end(); ++it) {
            (*it)->onNotify();
        }
    }
}
