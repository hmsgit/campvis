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

#include "imageseries.h"
#include "imagedata.h"
#include "cgt/assert.h"

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
        cgtAssert(dynamic_cast<const ImageData*>(dh.getData()) != 0, "DataHandle must contain ImageData!");
        _images.push_back(dh);
    }

    size_t ImageSeries::getNumImages() const {
        return _images.size();
    }

    DataHandle ImageSeries::getImage(size_t index) const {
        cgtAssert(index < _images.size(), "Index out of bounds.");
        return _images[index];
    }

    std::string ImageSeries::getTypeAsString() const {
        return "Image Series";
    }

    cgt::Bounds ImageSeries::getWorldBounds() const {
        cgt::Bounds b;
        for (size_t i = 0; i < _images.size(); ++i) {
            b.addVolume(static_cast<const ImageData*>(_images[i].getData())->getWorldBounds());
        }
        return b;
    }


}