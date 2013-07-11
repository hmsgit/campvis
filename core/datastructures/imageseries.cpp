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

#include "imageseries.h"
#include "imagedata.h"
#include "tgt/assert.h"

namespace campvis {


    ImageSeries::ImageSeries()
        : AbstractData()
    {

    }

    ImageSeries::~ImageSeries() {
    }

    ImageSeries* ImageSeries::clone() const {
        ImageSeries* toReturn = new ImageSeries();
        for (size_t i = 0; i < _images.size(); ++i)
            toReturn->_images.push_back(_images[i]);

        return toReturn;
    }

    size_t ImageSeries::getLocalMemoryFootprint() const {
        size_t toReturn = sizeof(DataHandle) * _images.capacity();
        for (size_t i = 0; i < _images.size(); ++i)
            toReturn += static_cast<const ImageData*>(_images[i].getData())->getLocalMemoryFootprint();
        return toReturn;
    }

    size_t ImageSeries::getVideoMemoryFootprint() const {
        size_t toReturn = 0;
        for (size_t i = 0; i < _images.size(); ++i)
            toReturn += static_cast<const ImageData*>(_images[i].getData())->getVideoMemoryFootprint();
        return toReturn;
    }

    void ImageSeries::addImage(ImageData* image) {
        _images.push_back(DataHandle(image));
    }

    void ImageSeries::addImage(DataHandle dh) {
        tgtAssert(dynamic_cast<const ImageData*>(dh.getData()) != 0, "DataHandle must contain ImageData!");
        _images.push_back(dh);
    }

    size_t ImageSeries::getNumImages() const {
        return _images.size();
    }

    DataHandle ImageSeries::getImage(size_t index) const {
        tgtAssert(index < _images.size(), "Index out of bounds.");
        return _images[index];
    }


}