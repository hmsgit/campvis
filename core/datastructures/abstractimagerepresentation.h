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

#ifndef ABSTRACTIMAGEREPRESENTATION_H__
#define ABSTRACTIMAGEREPRESENTATION_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    class ImageData;

    /**
     * Abstract base class for image representations.
     * A single ImageData instance can have multiple representations: They all represent
     * the semantically same image but have their data at different locations (e.g. disk,
     * RAM, OpenGL texture, ...)
     */
    class AbstractImageRepresentation {
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
        const tgt::svec3& getSize() const;

        /**
         * Returns the number of elements (= tgt::hmul(getSize())).
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

        /**
         * Returns the subimage representation of this representation given by \a llf and \a urb.
         * TODO: Check whether it is necessary to adjust image mapping!
         *
         * \param   newParent   Parent image of the newly created subimage
         * \param   llf         Lower-Left-Front coordinates of subimage
         * \param   urb         Upper-Right-Back coordinates of subimage
         * \return  An image representation containing the subimage of this with the given coordinates.
         */
        virtual AbstractImageRepresentation* getSubImage(ImageData* newParent, const tgt::svec3& llf, const tgt::svec3& urb) const = 0;

    protected:
        /**
         * Adds this image representations to the newParent image data.
         */
        void addToParent() const;

        /// Not copy-constructable
        AbstractImageRepresentation(const AbstractImageRepresentation& rhs);
        /// Not assignable
        AbstractImageRepresentation& operator=(const AbstractImageRepresentation& rhs);

        const ImageData* _parent;       ///< Image this representation represents, must not be 0.

        static const std::string loggerCat_;
    };
}

#endif // ABSTRACTIMAGEREPRESENTATION_H__
