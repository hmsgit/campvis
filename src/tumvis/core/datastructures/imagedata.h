// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef IMAGEDATA_H__
#define IMAGEDATA_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/imagemappinginformation.h"

namespace TUMVis {

    /**
     * Stores basic information about one (semantic) image of arbitrary dimension.
     * Different representations (e.g. local memory, OpenGL texture, OpenCL buffer) are
     * to be defined by inheritance.
     * 
     * \todo 
     */
    class ImageData : public AbstractData {
    public:
        ImageData(size_t dimensionality, const tgt::svec3& size);

        virtual ~ImageData();

        /**
         * Dimensionality of this image.
         * \return _dimensionality
         */
        size_t getDimensionality() const;

        /**
         * Size of this image (number of elements per dimension).
         * \return _size
         */
        const tgt::svec3& getSize() const;

        /**
         * Returns the number of elements (= tgt::hmul(getSize())).
         * \return  _numElements
         */
        size_t getNumElements() const;

        /**
         * Mapping information of this image
         * \return _mappingInformation
         */
        const ImageMappingInformation& getMappingInformation() const;

        /**
         * Returns the image extent in world coordinates.
         * \return  The image extent in world coordinates.
         */
        tgt::Bounds getWorldBounds() const;

        /**
         * Returns the image extent in world coordinates for the given voxel coordinates.
         * \param   llf     Lower-left-front in voxel coordinates.
         * \param   urb     Upper-right-back in voxel coordinates.
         * \return  The image extent in world coordinates for the given voxel coordinates.
         */
        tgt::Bounds getWorldBounds(const tgt::svec3& llf, const tgt::svec3& urb) const;

        /**
         * Returns the subimage of this image given by \a llf and \a urb.
         * TODO: Check whether it is necessary to adjust image mapping!
         *
         * \param   llf     Lower-Left-Front coordinates of subimage
         * \param   urb     Upper-Right-Back coordinates of subimage
         * \return  An image containing the subimage of this with the given coordinates.
         */
        virtual ImageData* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const = 0;

        /**
         * Transforms a vector based position to the corresponding array index.
         * \note    Caution, this method might return wrong results for non-continuous storage.
         *          In this case you should provide an appropriate overload.
         * \param   position    Vector based image coordinates
         * \return  Array index when image is stored continuously.
         */
        size_t positionToIndex(const tgt::svec3& position) const;

        /**
         * Transforms an array index to the corresponding vector based position.
         * \note    Caution, this method might return wrong results for non-continuous storage.
         *          In this case you should provide an appropriate overload.
         * \param   index   Array index when image is stored continuously.
         * \return  Vector based image coordinates.
         */
        tgt::svec3 indexToPosition(size_t index) const;
    protected:

        size_t _dimensionality;                         ///< Dimensionality of this image
        tgt::svec3 _size;                               ///< Size of this image (number of elements per dimension)
        size_t _numElements;                            ///< number of elements (= tgt::hmul(size))
        ImageMappingInformation _mappingInformation;    ///< Mapping information of this image

        static const std::string loggerCat_;
    };

}

#endif // IMAGEDATA_H__
