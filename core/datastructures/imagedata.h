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

#ifndef IMAGEDATA_H__
#define IMAGEDATA_H__

#include "tbb/concurrent_vector.h"
#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/abstractimagerepresentation.h"
#include "core/datastructures/imagemappinginformation.h"


#include <vector>

namespace campvis {
    class ImageRepresentationLocal;
    class AbstractImageRepresentationItk;

    /**
     * Stores basic information about one (semantic) image of arbitrary dimension.
     * Different representations (e.g. local memory, OpenGL texture, OpenCL buffer) are
     * to be defined by inheritance.
     * 
     * \todo 
     */
    class ImageData : public AbstractData {
    // friend so that it can add itself as representation
    friend class AbstractImageRepresentation;

    public:
        /**
         * Creates a new ImageData instance with the given parameters.
         * \param dimensionality    Dimensionality of this image
         * \param size              Size of this image (number of elements per dimension)
         * \param numChannels       Number of channels per element
         */
        ImageData(size_t dimensionality, const tgt::svec3& size, size_t numChannels);

        /// Destructor
        virtual ~ImageData();

        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \note    Deep-copies all contained representations!
         * \return  A copy of this object.
         */
        virtual ImageData* clone() const;

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
         * Returns the number of channels per element.
         * \return  _numChannels
         */
        size_t getNumChannels() const;

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
         * Sets the mapping information for this image to \a imi.
         * \param   imi The new ImageMappingInformation for this image.
         */
        void setMappingInformation(const ImageMappingInformation& imi);

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
         * \note    Creates sub-images of all representations.
         * \param   llf     Lower-Left-Front coordinates of subimage
         * \param   urb     Upper-Right-Back coordinates of subimage
         * \return  An image containing the sub-image of this image with the given coordinates.
         */
        virtual ImageData* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;

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

        /**
         * Returns a representation of this image of type \a T.
         * Looks, whether such a representations already exists, if not and \a performConversion is 
         * set, the method tries to create it via T::tryConvertFrom(). Returns 0 on failure.
         * \note    You do \b NOT have ownership of the returned pointer!
         *          The returned pointer is valid as long as this ImageData object exists.
         * \note    If \a T is OpenGL related, make sure to call this method from a valid and locked OpenGL context.
         * \param   performConversion   Flag whether to perform representation conversion if necessary.
         * \return  A pointer to a representation of type \a T, you do NOT have ownership, valid as long 
         *          as this ImageData object exists. 0 if no such representation could be created.
         */
        template<typename T>
        const T* getRepresentation(bool performConversion = true) const;

    protected:
        template<typename T>
        const T* tryPerformConversion() const;

        /**
         * Adds the given representation to the list of representations.
         * \note    Since this is non-publich method, no sanity checks are performed!
         * \param   representation  representation to add, must not be 0.
         */
        void addRepresentation(const AbstractImageRepresentation* representation);

        /**
         * Sets the initial representation of this ImageData.
         * \note    Removes and deletes all existing representations.
         * \param   representation  Initial representation of this image.
         */
        void setInitialRepresentation(const AbstractImageRepresentation* representation);

        /**
         * Clears all representations from the vector and frees the memory.
         * \note    Make sure to call this method only when nobody else holds pointers to the
         *          representations as they will be invalidated. This method is \b not thread-safe!
         */
        void clearRepresentations();

        /// List of all representations of this image. Mutable to allow lazy instantiation of new representations.
        mutable tbb::concurrent_vector<const AbstractImageRepresentation*> _representations;

        const size_t _dimensionality;                   ///< Dimensionality of this image
        const tgt::svec3 _size;                         ///< Size of this image (number of elements per dimension)
        const size_t _numChannels;                      ///< Number of channels per element
        const size_t _numElements;                      ///< number of elements (= tgt::hmul(size))
        ImageMappingInformation _mappingInformation;    ///< Mapping information of this image

        static const std::string loggerCat_;
    };


// = Template definition ==========================================================================

    template<typename T>
    const T* campvis::ImageData::getRepresentation(bool performConversion) const {
        // look, whether we already have a suitable representation
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it) {
            if (typeid(T) == typeid(**it)) {
                return static_cast<const T*>(*it);
            }
        }

        // no representation found, create a new one
        if (performConversion) {
            return tryPerformConversion<T>();
        }

        return 0;
    }

    template<>
    const campvis::ImageRepresentationLocal* campvis::ImageData::getRepresentation<ImageRepresentationLocal>(bool performConversion) const;

#ifdef CAMPVIS_HAS_MODULE_ITK
    template<>
    const campvis::AbstractImageRepresentationItk* campvis::ImageData::getRepresentation<AbstractImageRepresentationItk>(bool performConversion) const;
#endif

    template<typename T>
    const T* campvis::ImageData::tryPerformConversion() const {
        // TODO: Currently, we do not check, for multiple parallel conversions into the same
        //       target type. This does not harm thread-safety but may lead to multiple 
        //       representations of the same type for a single image.
        for (tbb::concurrent_vector<const AbstractImageRepresentation*>::const_iterator it = _representations.begin(); it != _representations.end(); ++it) {
            const T* tester = T::tryConvertFrom(*it);
            if (tester != 0) {
                return tester;
            }
        }

        // could not create a suitable representation
        LDEBUG("Could not create a " + std::string(typeid(T*).name()) + " representation.");
        return 0;
    }

}

#endif // IMAGEDATA_H__
