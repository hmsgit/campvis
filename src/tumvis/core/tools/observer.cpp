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
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _observers.insert(o);
    }

    void Observable::removeObserver(Observer* o) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _observers.erase(o);
    }

    void Observable::notifyObservers() const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        for (std::set<Observer*>::iterator it = _observers.begin(); it != _observers.end(); ++it) {
            (*it)->onNotify();
        }
    }
}
