#include "propertycollection.h"

namespace TUMVis {
    PropertyCollection::PropertyCollection() {

    }

    void PropertyCollection::addProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Property must not be 0!");
        std::vector<AbstractProperty*>::iterator it = findProperty(prop->getName());
        if (it != _properties.end()) {
            *it = prop;
        }
        else {
            _properties.push_back(prop);
        }
    }

    void PropertyCollection::removeProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Property must not be 0!");
        std::vector<AbstractProperty*>::iterator it = findProperty(prop->getName());
        if (it != _properties.end())
            _properties.erase(it);
    }

    AbstractProperty* PropertyCollection::getProperty(const std::string& name) const {
        std::vector<AbstractProperty*>::const_iterator it = findProperty(name);
        if (it != _properties.end())
            return *it;
        return 0;
    }

    const std::vector<AbstractProperty*>& PropertyCollection::getProperties() const {
        return _properties;
    }

    std::vector<AbstractProperty*>::iterator PropertyCollection::findProperty(const std::string& name) {
        // using std::find would be more elegant, but also need more LOC...
        for (std::vector<AbstractProperty*>::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    std::vector<AbstractProperty*>::const_iterator PropertyCollection::findProperty(const std::string& name) const {
        // using std::find would be more elegant, but also need more LOC...
        for (std::vector<AbstractProperty*>::const_iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    void PropertyCollection::lockAllProperties() {
        for (std::vector<AbstractProperty*>::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->lock();
        }
    }

    void PropertyCollection::unlockAllProperties() {
        for (std::vector<AbstractProperty*>::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->unlock();
        }
    }

}