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

    ImageData::ImageData(size_t dimensionality, const tgt::svec3& size) 
        : AbstractData()
        , _dimensionality(dimensionality)
        , _size(size)
        , _numElements(tgt::hmul(size))
        , _mappingInformation(size, tgt::vec3(0.f), tgt::vec3(1.f)) // TODO: get offset/voxel size as parameter or put default values into ImageMappingInformation ctor.
    {
    }

    ImageData::~ImageData() {
    }

    size_t ImageData::getDimensionality() const {
        return _dimensionality;
    }

    const tgt::svec3& ImageData::getSize() const {
        return _size;
    }

    const ImageMappingInformation& ImageData::getMappingInformation() const {
        return _mappingInformation;
    }

    tgt::Bounds ImageData::getWorldBounds() const {
        return tgt::Bounds(_mappingInformation.getOffset(), _mappingInformation.getOffset() + (tgt::vec3(_size) * _mappingInformation.getVoxelSize()));
    }

    tgt::Bounds ImageData::getWorldBounds(const tgt::svec3& llf, const tgt::svec3& urb) const {
        return tgt::Bounds(
            _mappingInformation.getOffset() + (tgt::vec3(llf) * _mappingInformation.getVoxelSize()),
            _mappingInformation.getOffset() + (tgt::vec3(urb) * _mappingInformation.getVoxelSize()));
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

}