// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "geometrydatacollection.h"
#include "cgt/logmanager.h"

namespace campvis {

    const std::string GeometryDataCollection::loggerCat_ = "CAMPVis.core.datastructures.GeometryDataCollection";;

    GeometryDataCollection::GeometryDataCollection() 
        : GeometryData()
    {
    }

    GeometryDataCollection::GeometryDataCollection(const GeometryDataCollection& rhs)
        : GeometryData(rhs)
    {
        for (size_t i = 0; i < rhs._geometries.size(); ++i)
            this->_geometries.push_back(rhs._geometries[i]->clone());
    }

    GeometryDataCollection::~GeometryDataCollection() {
        for (size_t i = 0; i < _geometries.size(); ++i)
            delete _geometries[i];
    }

    GeometryDataCollection& GeometryDataCollection::operator=(const GeometryDataCollection& rhs) {
        if (this == &rhs)
            return *this;

        GeometryData::operator=(rhs);

        // clone the geometries
        for (size_t i = 0; i < rhs._geometries.size(); ++i)
            this->_geometries.push_back(rhs._geometries[i]->clone());

        return *this;
    }

    GeometryDataCollection* GeometryDataCollection::clone() const {
        GeometryDataCollection* toReturn = new GeometryDataCollection();
        for (size_t i = 0; i < _geometries.size(); ++i)
            toReturn->_geometries.push_back(this->_geometries[i]->clone());
        return toReturn;
    }

    void GeometryDataCollection::addGeometry(GeometryData* g) {
        _geometries.push_back(g);
    }

    size_t GeometryDataCollection::getNumGeometries() const {
        return _geometries.size();
    }

    const GeometryData* GeometryDataCollection::getGeometry(size_t index) const {
        cgtAssert(index < _geometries.size(), "Index out of bounds!");
        if (index < _geometries.size()) {
            return _geometries[index];
        }

        return nullptr;
    }

    void GeometryDataCollection::removeGeometry(size_t index) {
        cgtAssert(index < _geometries.size(), "Index out of bounds!");
        if (index < _geometries.size()) {
            delete _geometries[index];
            _geometries.erase(_geometries.begin() + index);
        }
    }

    void GeometryDataCollection::render(GLenum mode /*= GL_POLYGON*/) const {
        for (size_t i = 0; i < _geometries.size(); ++i)
            _geometries[i]->render(mode);
    }

    cgt::Bounds GeometryDataCollection::getWorldBounds() const {
        cgt::Bounds toReturn;

        for (size_t i = 0; i < _geometries.size(); ++i)
            toReturn.addVolume(_geometries[i]->getWorldBounds());

        return toReturn;
    }

    bool GeometryDataCollection::hasTextureCoordinates() const {
        for (size_t i = 0; i < _geometries.size(); ++i) {
            if (! _geometries[i]->hasTextureCoordinates())
                return false;
        }

        return true;
    }

    bool GeometryDataCollection::hasPickingInformation() const {
        for (size_t i = 0; i < _geometries.size(); ++i) {
            if (! _geometries[i]->hasPickingInformation())
                return false;
        }

        return true;
    }

    void GeometryDataCollection::applyTransformationToVertices(const cgt::mat4& t) {
        for (size_t i = 0; i < _geometries.size(); ++i)
            _geometries[i]->applyTransformationToVertices(t);
    }

    size_t GeometryDataCollection::getLocalMemoryFootprint() const {
        size_t sum = sizeof(GeometryDataCollection);

        for (size_t i = 0; i < _geometries.size(); ++i)
            sum += _geometries[i]->getLocalMemoryFootprint();

        return sum;
    }

    size_t GeometryDataCollection::getVideoMemoryFootprint() const {
        size_t sum = GeometryData::getVideoMemoryFootprint();

        for (size_t i = 0; i < _geometries.size(); ++i)
            sum += _geometries[i]->getVideoMemoryFootprint();

        return sum;
    }

    std::string GeometryDataCollection::getTypeAsString() const {
        return "Collection of Geometry Data";
    }

}