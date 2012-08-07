#include "abstractproperty.h"

namespace TUMVis {

    const std::string AbstractProperty::loggerCat_ = "TUMVis.core.datastructures.AbstractProperty";

    AbstractProperty::AbstractProperty(const std::string& name, const std::string& title, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/)
        : _name(name)
        , _title(title)
        , _invalidationLevel(il)
    {
        _inUse = false;
    }

    AbstractProperty::~AbstractProperty() {
    }

    const std::string& AbstractProperty::getName() {
        return _name;
    }

    const std::string& AbstractProperty::getTitle() {
        return _title;
    }

    const InvalidationLevel& AbstractProperty::getInvalidationLevel() const {
        return _invalidationLevel;
    }

    void AbstractProperty::setInvalidationLevel(const InvalidationLevel& il) {
        _invalidationLevel = il;
    }

    void AbstractProperty::addSharedProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Shared property must not be 0!");
        tgtAssert(typeid(this) == typeid(prop), "Shared property must be of the same type as this property.");

        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _sharedProperties.insert(prop);
    }

    void AbstractProperty::removeSharedProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Shared property must not be 0!");
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _sharedProperties.erase(prop);
    }

    const std::set<AbstractProperty*>& AbstractProperty::getSharedProperties() const {
        return _sharedProperties;
    }

    void AbstractProperty::lock() {
        _inUse = true;
    }

    void AbstractProperty::unlock() {
        _inUse = false;
    }

    void AbstractProperty::init() {

    }

    void AbstractProperty::deinit() {

    }

}
