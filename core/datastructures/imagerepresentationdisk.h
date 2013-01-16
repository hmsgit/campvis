// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef IMAGEREPRESENTATIONDISK_H__
#define IMAGEREPRESENTATIONDISK_H__

#include "core/datastructures/abstractimagerepresentation.h"

#include "core/tools/endianhelper.h"
#include "core/tools/weaklytypedpointer.h"

namespace campvis {

    /**
     * Subclass of ImageData offering access to image data stored in binary form on the local harddisk.
     * 
     * \todo    Number of channels
     */
    class ImageRepresentationDisk : public AbstractImageRepresentation {
    public:
        /**
         * Creates a new ImageData disk representation.
         *
         * \param parent            Image this representation represents, must not be 0.
         * \param url               Path to file with raw data
         * \param type              Base type of data
         * \param numChannels       Number of channels per element
         * \param offset            Offset of first data element in file (in bytes)
         * \param endianness        Endianess of data
         * \param stride            Number of _elemments_ _between_ adjacent elements for each dimension (\see ImageRepresentationDisk::_stride).
         */
        ImageRepresentationDisk(
            const ImageData* parent,
            const std::string& url,
            WeaklyTypedPointer::BaseType type,
            size_t numChannels,
            size_t offset = 0,
            EndianHelper::Endianness endianness = EndianHelper::LITTLE_ENDIAN,
            const tgt::svec3& stride = tgt::svec3::zero
            );

        /**
         * Destructor
         */
        virtual ~ImageRepresentationDisk();


        //\see AbstractImageRepresentation::clone()
        virtual ImageRepresentationDisk* clone() const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getSubImage
        virtual ImageRepresentationDisk* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


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

        /**
         * Returns the number of channels per element
         * \return _numChannels
         */
        size_t getNumChannels() const;

    private:
        /**
         * Calculates the canonical stride for the given image size.
         * \param size  Image size (number of elements per dimension).
         * \return      (0, size.x, size.x * size.y)
         */
        tgt::svec3 getCanonicStride(const tgt::svec3& size) const;

        std::string _url;                       ///< path to file with raw data
        size_t _offset;                         ///< offset of first data element in file (in bytes)
        WeaklyTypedPointer::BaseType _type;     ///< base type of data
        size_t _numChannels;                    ///< number of channels per element
        EndianHelper::Endianness _endianess;    ///< endianess of data
        
        /**
         * Number of _elements_ _between_ adjacent elements for each dimension.
         * If the stride is 0 for a dimension, the elements are assumed to be tightly packed, hence for
         * the x-dimension 0, for the y-dimension _size.x and for the z-dimension (_size.x * _size.y).
         **/
        tgt::svec3 _stride;

        static const std::string loggerCat_;
    };

}

#endif // IMAGEREPRESENTATIONDISK_H__
