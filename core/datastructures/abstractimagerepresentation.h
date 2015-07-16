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

#ifndef ABSTRACTIMAGEREPRESENTATION_H__
#define ABSTRACTIMAGEREPRESENTATION_H__

#include "cgt/bounds.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    class ImageData;

    /**
     * Abstract base class for image representations.
     * A single ImageData instance can have multiple representations: They all represent
     * the semantically same image but have their data at different locations (e.g. disk,
     * RAM, OpenGL texture, ...)
     */
    class CAMPVIS_CORE_API AbstractImageRepresentation {
    public:
        /**
         * Creates a new abstract representation for the image \a parent.
         * \param   parent  Image this representation represents, must not be 0.
         */
        AbstractImageRepresentation(ImageData* parent);

        /**
         * Virtual Destructor.
         */
        virtual ~AbstractImageRepresentation();

        /**
         * Returns the newParent ImageData this representation is representing.
         * \return _parent
         */
        const ImageData* getParent() const;

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
         * Returns the number of elements (= cgt::hmul(getSize())).
         * \return  _numElements
         */
        size_t getNumElements() const;

        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \param   newParent   Parent image of the cloned object.
         * \return  A copy of this object.
         */
        virtual AbstractImageRepresentation* clone(ImageData* newParent) const = 0;
        
        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const = 0;

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const = 0;

    protected:
        /**
         * Adds this image representations to the newParent image data.
         */
        void addToParent() const;

        const ImageData* _parent;       ///< Image this representation represents, must not be 0.

    private:
        /// Not copy-constructable
        explicit AbstractImageRepresentation(const AbstractImageRepresentation& rhs);
        /// Not assignable
        AbstractImageRepresentation& operator=(const AbstractImageRepresentation& rhs);

        static const std::string loggerCat_;
    };
}

#endif // ABSTRACTIMAGEREPRESENTATION_H__
