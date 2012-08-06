#include "propertycollection.h"

namespace TUMVis {
    HasPropertyCollection::HasPropertyCollection() {
    }

    HasPropertyCollection::~HasPropertyCollection() {
    }

    void HasPropertyCollection::addProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Property must not be 0!");
        PropertyCollection::iterator it = findProperty(prop->getName());
        if (it != _properties.end()) {
            (*it)->s_changed.disconnect(this);
            *it = prop;
        }
        else {
            _properties.push_back(prop);
        }
        prop->s_changed.connect(this, &HasPropertyCollection::onPropertyChanged);
    }

    void HasPropertyCollection::removeProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Property must not be 0!");
        PropertyCollection::iterator it = findProperty(prop->getName());
        if (it != _properties.end()) {
            (*it)->s_changed.disconnect(this);
            _properties.erase(it);
        }
    }

    AbstractProperty* HasPropertyCollection::getProperty(const std::string& name) const {
        PropertyCollection::const_iterator it = findProperty(name);
        if (it != _properties.end())
            return *it;
        return 0;
    }

    const PropertyCollection& HasPropertyCollection::getProperties() const {
        return _properties;
    }

    PropertyCollection& HasPropertyCollection::getProperties() {
        return _properties;
    }

    PropertyCollection::iterator HasPropertyCollection::findProperty(const std::string& name) {
        // using std::find would be more elegant, but also need more LOC...
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    PropertyCollection::const_iterator HasPropertyCollection::findProperty(const std::string& name) const {
        // using std::find would be more elegant, but also need more LOC...
        for (PropertyCollection::const_iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    void HasPropertyCollection::lockAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->lock();
        }
    }

    void HasPropertyCollection::unlockAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->unlock();
        }
    }

    void HasPropertyCollection::initAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->init();
        }
    }

    void HasPropertyCollection::deinitAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->deinit();
        }
    }

}