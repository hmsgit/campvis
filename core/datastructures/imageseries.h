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

#ifndef IMAGESERIES_H__
#define IMAGESERIES_H__

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datahandle.h"
#include <vector>

namespace campvis {
    class ImageData;

    /**
     * Class encapsulating a series of images.
     */
    class CAMPVIS_CORE_API ImageSeries : public AbstractData {
    public:
        /**
         * Constructor
         */
        ImageSeries();

        /**
         * Virtual destructor
         */
        virtual ~ImageSeries();


        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \return  A copy of this object.
         */
        virtual ImageSeries* clone() const;

        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const;

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const;
        
        /**
         * Appends the image \a image to the series.
         * \param   image   Image to be added.
         */
        void addImage(ImageData* image);
        
        /**
         * Appends the image \a image to the series.
         * \note    The DataHandle must contain walid image data.
         * \param   dh  DataHandle with image to be added.
         */
        void addImage(DataHandle dh);

        /**
         * Returns the number of images in this series
         * \return  _images.size()
         */
        size_t getNumImages() const;

        /**
         * Returns a DataHandle with the image number \a index of this series.
         * \param   index   Index of the image to return
         * \return  _images[index]
         */
        DataHandle getImage(size_t index) const;

    protected:
        std::vector<DataHandle> _images;    ///< the images of this series
    };

}

#endif // IMAGESERIES_H__