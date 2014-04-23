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

#ifndef IMAGEREPRESENTATIONDISK_H__
#define IMAGEREPRESENTATIONDISK_H__

#include "core/datastructures/genericabstractimagerepresentation.h"
#include "core/tools/endianhelper.h"
#include "core/tools/weaklytypedpointer.h"

namespace campvis {

    /**
     * Subclass of ImageData offering access to image data stored in binary form on the local harddisk.
     * 
     * \todo    Number of channels
     */
    class CAMPVIS_CORE_API ImageRepresentationDisk : public GenericAbstractImageRepresentation<ImageRepresentationDisk> {
    public:
        /**
         * Creates a new ImageRepresentationDisk with the given parameters and automatically
         * adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer.
         *
         * \param   parent     Image this representation represents, must not be 0, will take ownership of the returned pointer.
         * \param   url        Path to file with raw data
         * \param   type       Base type of data
         * \param   offset     Offset of first data element in file (in bytes)
         * \param   endianness Endianess of data
         * \param   stride     Number of _elements_ _between_ adjacent elements for each dimension (\see ImageRepresentationDisk::_stride).
         * \param   multichannelSideBySide  Flag whether multichannel images are stored side by side
         * \return  A pointer to the newly created ImageRepresentationDisk, you do \b not own this pointer!
         */
        static ImageRepresentationDisk* create(
            ImageData* parent,
            const std::string& url,
            WeaklyTypedPointer::BaseType type,
            size_t offset = 0,
            EndianHelper::Endianness endianness = EndianHelper::IS_LITTLE_ENDIAN,
            const tgt::svec3& stride = tgt::svec3::zero,
            bool multichannelSideBySide = false
            );


        /**
         * Destructor
         */
        virtual ~ImageRepresentationDisk();

        /**
         * Performs a conversion of \a source to an ImageRepresentationLocal if feasible.
         * Returns 0 if conversion was not successful or source representation type is not compatible.
         * \note    The callee, respectively the callee's parent, has the ownership of the returned pointer.
         * \param   source  Source image representation for conversion.
         * \return  A pointer to a local representation of \a source or 0 on failure. The caller does \b not have ownership.
         */
        static ImageRepresentationDisk* tryConvertFrom(const AbstractImageRepresentation* source);

        /// \see AbstractImageRepresentation::clone()
        virtual ImageRepresentationDisk* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;


        /**
         * Downloads the whole image data to local memory.
         * \note    The caller has to take ownership of the returned pointer.
         * \return  Pointer to the image data in the local memory, to be owned by caller.
         */
        WeaklyTypedPointer getImageData() const;


        /**
         * Returns the base type of the data
         * \return _type
         */
        WeaklyTypedPointer::BaseType getBaseType() const;

    protected:
        /**
         * Creates a new ImageData disk representation.
         * \note  The Constructor is protected since image representations are not supposed to be
         *        created via the new operator - use ImageRepresentationDisk::create() instead.
         *
         * \param parent            Image this representation represents, must not be 0.
         * \param url               Path to file with raw data
         * \param type              Base type of data
         * \param offset            Offset of first data element in file (in bytes)
         * \param endianness        Endianess of data
         * \param stride            Number of _elements_ _between_ adjacent elements for each dimension (\see ImageRepresentationDisk::_stride).
         * \param multichannelSideBySide    Flag whether multichannel images are stored side by side
         */
        ImageRepresentationDisk(
            ImageData* parent,
            const std::string& url,
            WeaklyTypedPointer::BaseType type,
            size_t offset = 0,
            EndianHelper::Endianness endianness = EndianHelper::IS_LITTLE_ENDIAN,
            const tgt::svec3& stride = tgt::svec3::zero,
            bool multichannelSideBySide = false
            );

        /**
         * Calculates the canonical stride for the given image size.
         * \param size  Image size (number of elements per dimension).
         * \return      (0, size.x, size.x * size.y)
         */
        tgt::svec3 getCanonicStride(const tgt::svec3& size) const;

        std::string _url;                       ///< path to file with raw data
        size_t _offset;                         ///< offset of first data element in file (in bytes)
        WeaklyTypedPointer::BaseType _type;     ///< base type of data
        EndianHelper::Endianness _endianess;    ///< endianess of data
        
        /**
         * Number of _elements_ _between_ adjacent elements for each dimension.
         * If the stride is 0 for a dimension, the elements are assumed to be tightly packed, hence for
         * the x-dimension 0, for the y-dimension _size.x and for the z-dimension (_size.x * _size.y).
         **/
        tgt::svec3 _stride;

        bool _multichannelSideBySide;           ///< Flag whether multichannel images are stored side by side

        static const std::string loggerCat_;
    };

}

#endif // IMAGEREPRESENTATIONDISK_H__
