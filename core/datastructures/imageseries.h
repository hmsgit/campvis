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
    class ImageSeries : public AbstractData {
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