// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "core/datastructures/imagerepresentationlocal.h"

#ifdef CAMPVIS_HAS_MODULE_ITK
#include "modules/itk/core/genericimagerepresentationitk.h"
#endif

namespace campvis {
    const std::string ImageData::loggerCat_ = "CAMPVis.core.datastructures.ImageData";

    ImageData::ImageData(size_t dimensionality, const tgt::svec3& size, size_t numChannels) 
        : AbstractData()
        , _dimensionality(dimensionality)
        , _size(size)
        , _numChannels(numChannels)
        , _numElements(tgt::hmul(size))
        , _mappingInformation(size, tgt::vec3(0.f), tgt::vec3(1.f)) // TODO: get offset/voxel size as parameter or put default values into ImageMappingInformation ctor.
    {
        tgtAssert(numChannels > 0, "Number of channels must be greater than 0!");
    }

    ImageData::~ImageData() {
        clearRepresentations();
    }

    ImageData* ImageData::clone() const {
        ImageData* toReturn = new ImageData(_dimensionality, _size, _numChannels);
        toReturn->_mappingInformation = _mappingInformation;
        tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator start = _representations.begin();
        tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator end = _representations.end();
        toReturn->_representations.assign(start, end);
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

    const tgt::svec3& ImageData::getSize() const {
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

    tgt::Bounds ImageData::getWorldBounds() const {
        return tgt::Bounds(_mappingInformation.getOffset(), _mappingInformation.getOffset() + (tgt::vec3(_size) * _mappingInformation.getVoxelSize()));
    }

    tgt::Bounds ImageData::getWorldBounds(const tgt::svec3& llf, const tgt::svec3& urb) const {
        return tgt::Bounds(
            _mappingInformation.getOffset() + (tgt::vec3(llf) * _mappingInformation.getVoxelSize()),
            _mappingInformation.getOffset() + (tgt::vec3(urb) * _mappingInformation.getVoxelSize()));
    }

    ImageData* ImageData::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");

        tgt::svec3 newSize = urb - llf;
        if (newSize == getSize()) {
            // nothing has changed, just provide a copy:
            return clone();
        }

        // compute new dimensionality
        size_t newDimensionality = 1;
        if (newSize.y > 1)
            newDimensionality = 2;
        if (newSize.z > 1)
            newDimensionality = 3;

        // create new ImageData object and assign mapping information
        ImageData* toReturn = new ImageData(newDimensionality, newSize, _numChannels);
        toReturn->_mappingInformation = ImageMappingInformation(newSize, _mappingInformation.getOffset(), _mappingInformation.getVoxelSize(), _mappingInformation.getRealWorldMapping());
        
        // create sub-image of every image representation
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it) {
            AbstractImageRepresentation* si = (*it)->getSubImage(toReturn, llf, urb);
            toReturn->addRepresentation(si);
        }

        return toReturn;
    }

    size_t ImageData::getNumElements() const {
        return _numElements;
    }

    size_t ImageData::positionToIndex(const tgt::svec3& position) const {
        return position.x + (position.y * _size.x) + (position.z * _size.x * _size.y);
    }

    tgt::svec3 ImageData::indexToPosition(size_t index) const {
        size_t z = index / (_size.x * _size.y);
        size_t y = (index % (_size.x * _size.y)) / _size.x;
        size_t x = index % _size.x;
        return tgt::svec3(x, y, z);
    }

    void ImageData::clearRepresentations() {
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it)
            delete *it;
        _representations.clear();
    }

    void ImageData::addRepresentation(const AbstractImageRepresentation* representation) {
        tgtAssert(representation != 0, "Representation must not be 0.");
        _representations.push_back(representation);
    }

    void ImageData::setInitialRepresentation(const AbstractImageRepresentation* representation) {
        clearRepresentations();
        addRepresentation(representation);
    }

    template<>
    const ImageRepresentationLocal* ImageData::getRepresentation<ImageRepresentationLocal>(bool performConversion /*= true*/) const {
        // look, whether we already have a suitable representation
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it) {
            if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(*it))
                return tester;
        }

        if (performConversion) {
            return tryPerformConversion<ImageRepresentationLocal>();
        }

        return 0;
    }

#ifdef CAMPVIS_HAS_MODULE_ITK
    template<>
    const AbstractImageRepresentationItk* ImageData::getRepresentation<AbstractImageRepresentationItk>(bool performConversion /*= true*/) const {
        // look, whether we already have a suitable representation
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it) {
            if (const AbstractImageRepresentationItk* tester = dynamic_cast<const AbstractImageRepresentationItk*>(*it))
                return tester;
        }

        if (performConversion) {
            tgtAssert(false, "Conversion to AbstractImageRepresentationItk not implemented - is it really needed?");
            LWARNING("Could not convert to AbstractImageRepresentationItk");
        }

        return 0;
    }
#endif
}