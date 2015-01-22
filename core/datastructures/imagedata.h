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

#ifndef IMAGEDATA_H__
#define IMAGEDATA_H__

#include <tbb/concurrent_vector.h>
#include <tbb/spin_mutex.h>

#include "cgt/logmanager.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/abstractimagerepresentation.h"
#include "core/datastructures/imagemappinginformation.h"
#include "core/datastructures/imagerepresentationconverter.h"

#include <vector>

namespace campvis {

    /**
     * Stores basic information about one (semantic) image of arbitrary dimension.
     * Different representations (e.g. local memory, OpenGL texture) are
     * to be defined by inheritance.
     */
    class CAMPVIS_CORE_API ImageData : public AbstractData, public IHasWorldBounds {
    // friend so that it can add itself as representation
    friend class AbstractImageRepresentation;

    public:
        /**
         * Creates a new ImageData instance with the given parameters.
         * \param dimensionality    Dimensionality of this image
         * \param size              Size of this image (number of elements per dimension)
         * \param numChannels       Number of channels per element
         */
        explicit ImageData(size_t dimensionality, const cgt::svec3& size, size_t numChannels);

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
         * Returns a human readable representation of the data type that can be used for the GUI.
         * \return  A string with the type name to be used for the GUI.
         */
        virtual std::string getTypeAsString() const;

        /**
         * Dimensionality of this image.
         * \return _dimensionality
         */
        size_t getDimensionality() const;

        /**
         * Size of this image (number of elements per dimension).
         * \return _size
         */
        const cgt::svec3& getSize() const;

        /**
         * Returns the number of channels per element.
         * \return  _numChannels
         */
        size_t getNumChannels() const;

        /**
         * Returns the number of elements (= cgt::hmul(getSize())).
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
        virtual cgt::Bounds getWorldBounds() const;

        /**
         * Returns the image extent in world coordinates for the given voxel coordinates.
         * \param   llf     Lower-left-front in voxel coordinates.
         * \param   urb     Upper-right-back in voxel coordinates.
         * \return  The image extent in world coordinates for the given voxel coordinates.
         */
        cgt::Bounds getWorldBounds(const cgt::svec3& llf, const cgt::svec3& urb) const;

        /**
         * Transforms a vector based position to the corresponding array index.
         * \note    Caution, this method might return wrong results for non-continuous storage.
         *          In this case you should provide an appropriate overload.
         * \param   position    Vector based image coordinates
         * \return  Array index when image is stored continuously.
         */
        size_t positionToIndex(const cgt::svec3& position) const;

        /**
         * Transforms an array index to the corresponding vector based position.
         * \note    Caution, this method might return wrong results for non-continuous storage.
         *          In this case you should provide an appropriate overload.
         * \param   index   Array index when image is stored continuously.
         * \return  Vector based image coordinates.
         */
        cgt::svec3 indexToPosition(size_t index) const;

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
        const cgt::svec3 _size;                         ///< Size of this image (number of elements per dimension)
        const size_t _numChannels;                      ///< Number of channels per element
        const size_t _numElements;                      ///< number of elements (= cgt::hmul(size))
        ImageMappingInformation _mappingInformation;    ///< Mapping information of this image

        /// Mutex protecting the representation conversions to ensure that there is only one conversion happening at a time.
        mutable tbb::spin_mutex _conversionMutex;

        static const std::string loggerCat_;
    };


// = Template definition ==========================================================================

    template<typename T>
    const T* campvis::ImageData::getRepresentation(bool performConversion) const {
        // look, whether we already have a suitable representation
        for (auto it = _representations.rbegin(); it != _representations.rend(); ++it) {
            if (const T* tester = dynamic_cast<const T*>(*it)) {
                return tester;
            }
        }

        // no representation found, create a new one
        if (performConversion) {
            tbb::spin_mutex::scoped_lock lock(_conversionMutex);

            // in the meantime, there something might have changed, so check again whether there is a new rep.
            for (auto it = _representations.rbegin(); it != _representations.rend(); ++it) {
                if (const T* tester = dynamic_cast<const T*>(*it)) {
                    return tester;
                }
            }

            return tryPerformConversion<T>();
        }

        return 0;
    }

    template<typename T>
    const T* campvis::ImageData::tryPerformConversion() const {
        for (auto it = _representations.rbegin(); it != _representations.rend(); ++it) {
            const T* tester = ImageRepresentationConverter::getRef().tryConvertFrom<T>(*it);
            if (tester != 0) {
                //LDEBUG("Performed a image representation conversion from " + std::string(typeid(**it).name()) + " to " + std::string(typeid(T).name()) + ".");
                return tester;
            }
        }

        // could not create a suitable representation
        LWARNING("Could not create a " + std::string(typeid(T).name()) + " representation.");
        return 0;
    }

}

#endif // IMAGEDATA_H__
