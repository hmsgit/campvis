#include "referencecounted.h"

namespace TUMVis {
    ReferenceCounted::ReferenceCounted()
        : _shareable(true)
    {
        _refCount = 0;
    }

    ReferenceCounted::ReferenceCounted(const ReferenceCounted& rhs)
        : _shareable(true)
    {
        _refCount = 0;
    }

    ReferenceCounted& ReferenceCounted::operator=(const ReferenceCounted& rhs) {
        return *this;
    }

    ReferenceCounted::~ReferenceCounted() {
    }

    void ReferenceCounted::addReference() {
        ++_refCount;
    }

    void ReferenceCounted::removeReference() {
        // TODO: I'm afraid this is not 100% thread-safe - refCount might change between atomic decrement, check and deletion...
        if (--_refCount == 0)
            delete this;
    }

    void ReferenceCounted::markUnsharable() {
        _shareable = false;
    }

    bool ReferenceCounted::isShareable() const {
        return _shareable;
    }

    bool ReferenceCounted::isShared() const {
        return _refCount > 1;
    }

}

