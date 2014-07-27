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

#include "imageseriessplitter.h"

#include "core/datastructures/imageseries.h"

namespace campvis {
    const std::string ImageSeriesSplitter::loggerCat_ = "CAMPVis.modules.io.ImageSeriesSplitter";

    ImageSeriesSplitter::ImageSeriesSplitter() 
        : AbstractProcessor()
        , p_inputID("InputID", "Input Image Series ID", "input", DataNameProperty::READ)
        , p_outputID("OutputID", "Output Image ID", "output", DataNameProperty::WRITE)
        , p_imageIndex("ImageIndex", "Image to Select", 0, 0, 0)
    {
        addProperty(p_inputID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_outputID);
        addProperty(p_imageIndex);
    }

    ImageSeriesSplitter::~ImageSeriesSplitter() {

    }

    void ImageSeriesSplitter::updateResult(DataContainer& data) {
        ScopedTypedData<ImageSeries> series(data, p_inputID.getValue());
        if (series != 0) {
            if (p_imageIndex.getValue() < static_cast<int>(series->getNumImages())) {
                data.addDataHandle(p_outputID.getValue(), series->getImage(p_imageIndex.getValue()));
            }
        }
    }

    void ImageSeriesSplitter::updateProperties(DataContainer& dataContainer) {
        ScopedTypedData<ImageSeries> series(dataContainer, p_inputID.getValue());
        if (series != 0)
            p_imageIndex.setMaxValue(static_cast<int>(series->getNumImages()));
    }

}
