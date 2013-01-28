// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "propertycollection.h"

namespace campvis {
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

    void HasPropertyCollection::onPropertyChanged(const AbstractProperty* prop) {
        if (prop->getInvalidationLevel().isInvalidProperties())
            updateProperties();
    }

    void HasPropertyCollection::updateProperties() {

    }

}