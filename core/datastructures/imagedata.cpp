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

#include "imagedata.h"

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
        toReturn->_representations.assign(_representations.begin(), _representations.end());
        return toReturn;
    }

    size_t ImageData::getLocalMemoryFootprint() const {
        size_t toReturn = sizeof(*this) + _representations.size() * sizeof(AbstractImageRepresentation*);
        for (std::vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it)
            toReturn += (*it)->getLocalMemoryFootprint();

        return toReturn;
    }

    size_t ImageData::getVideoMemoryFootprint() const {
        size_t toReturn = 0;
        for (std::vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it)
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
        for (std::vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it) {
            AbstractImageRepresentation* si = (*it)->getSubImage(toReturn, llf, urb);
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
        for (std::vector<const AbstractImageRepresentation*>::iterator it = _representations.begin(); it != _representations.end(); ++it)
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


}