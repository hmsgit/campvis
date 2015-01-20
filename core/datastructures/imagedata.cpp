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

#include "imagedata.h"

namespace campvis {
    const std::string ImageData::loggerCat_ = "CAMPVis.core.datastructures.ImageData";

    ImageData::ImageData(size_t dimensionality, const cgt::svec3& size, size_t numChannels) 
        : AbstractData()
        , _dimensionality(dimensionality)
        , _size(size)
        , _numChannels(numChannels)
        , _numElements(cgt::hmul(size))
        , _mappingInformation(size, cgt::vec3(0.f), cgt::vec3(1.f)) // TODO: get offset/voxel size as parameter or put default values into ImageMappingInformation ctor.
    {
        cgtAssert(numChannels > 0, "Number of channels must be greater than 0!");
    }

    ImageData::~ImageData() {
        clearRepresentations();
    }

    ImageData* ImageData::clone() const {
        ImageData* toReturn = new ImageData(_dimensionality, _size, _numChannels);
        toReturn->_mappingInformation = _mappingInformation;
        for (auto it = this->_representations.begin(); it != this->_representations.end(); ++it) {
            (*it)->clone(toReturn);
        }
        return toReturn;
    }

    size_t ImageData::getLocalMemoryFootprint() const {
        size_t toReturn = sizeof(*this) + _representations.size() * sizeof(AbstractImageRepresentation*);
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it)
            toReturn += (*it)->getLocalMemoryFootprint();

        return toReturn;
    }

    size_t ImageData::getVideoMemoryFootprint() const {
        size_t toReturn = 0;
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it)
            toReturn += (*it)->getVideoMemoryFootprint();

        return toReturn;
    }

    size_t ImageData::getDimensionality() const {
        return _dimensionality;
    }

    const cgt::svec3& ImageData::getSize() const {
        return _size;
    }

    size_t ImageData::getNumChannels() const {
        return _numChannels;
    }

    const ImageMappingInformation& ImageData::getMappingInformation() const {
        return _mappingInformation;
    }

    void ImageData::setMappingInformation(const ImageMappingInformation& imi) {
        _mappingInformation = imi;
    }

    cgt::Bounds ImageData::getWorldBounds() const {
        return getWorldBounds(cgt::svec3(0, 0, 0), cgt::svec3(_size));
    }

    cgt::Bounds ImageData::getWorldBounds(const cgt::svec3& llf, const cgt::svec3& urb) const {
        return cgt::Bounds(_mappingInformation.getVoxelToWorldMatrix() * cgt::vec3(llf), _mappingInformation.getVoxelToWorldMatrix() * cgt::vec3(urb));
    }

    size_t ImageData::getNumElements() const {
        return _numElements;
    }

    size_t ImageData::positionToIndex(const cgt::svec3& position) const {
        return position.x + (position.y * _size.x) + (position.z * _size.x * _size.y);
    }

    cgt::svec3 ImageData::indexToPosition(size_t index) const {
        size_t z = index / (_size.x * _size.y);
        size_t y = (index % (_size.x * _size.y)) / _size.x;
        size_t x = index % _size.x;
        return cgt::svec3(x, y, z);
    }

    void ImageData::clearRepresentations() {
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it)
            delete *it;
        _representations.clear();
    }

    void ImageData::addRepresentation(const AbstractImageRepresentation* representation) {
        cgtAssert(representation != 0, "Representation must not be 0.");
        _representations.push_back(representation);
    }

    void ImageData::setInitialRepresentation(const AbstractImageRepresentation* representation) {
        clearRepresentations();
        addRepresentation(representation);
    }

    std::string ImageData::getTypeAsString() const {
        return "Image Data";
    }

}