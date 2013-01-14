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

#ifndef IMAGEDATACONVERTER_H__
#define IMAGEDATACONVERTER_H__

#include "core/datastructures/imagedatadisk.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/datastructures/genericimagedatalocal.h"
#include "core/datastructures/imagedatagl.h"

#include <string>

namespace campvis {

    /**
     * Static class offering conversion between differen ImageData types
     */
    struct ImageDataConverter {
    public:
        /**
         * Tries to convert \a source to \a TO ImageData type.
         * \note    Caller takes ownership of the created ImageData pointer.
         * \tparam  TO      Target conversion type.
         * \param   source  ImageData source for conversion.
         * \return  A pointer to the converted ImageData object, 0 if conversion failed.
         */
        template<class TO>
        static TO* tryConvert(const ImageData* source);

        /**
         * Converts \a source to \a TO ImageData type.
         * \note    Caller takes ownership of the created ImageData pointer.
         * \note    Valid conversions have to be implemented using template specialization.
         * \tparam  FROM    Source conversion type.
         * \tparam  TO      Target conversion type.
         * \param   source  ImageData source for conversion.
         * \return  A pointer to the converted ImageData object, 0 if conversion failed.
         */
        template<class FROM, class TO>
        static TO* convert(const FROM* source);

    protected:
        /**
         * Helper function for creating a GenericImageDataLocal<BASETYPE, NUMCHANNELS> from an ImageDataDisk
         * \note    Caller takes ownership of the created ImageData pointer.
         * \tparam  BASETYPE    Image element base type
         * \tparam  NUMCHANNELS Number of channels per image element.
         * \param   source      ImageData source for conversion.
         * \return  A pointer to the converted GenericImageDataLocal<BASETYPE, NUMCHANNELS> object, 0 if conversion failed.
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        static GenericImageDataLocal<BASETYPE, NUMCHANNELS>* convertToGenericLocal(const ImageDataDisk* source);
        
        static const std::string loggerCat_;
    };

// ================================================================================================

    template<class FROM, class TO>
    TO* campvis::ImageDataConverter::convert(const FROM* source) {
        tgtAssert(false, "Conversion not implemented.");
        return 0;
    }

    template<class TO>
    TO* campvis::ImageDataConverter::tryConvert(const ImageData* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageDataDisk* tester = dynamic_cast<const ImageDataDisk*>(source)) {
            return convert<ImageDataDisk, TO>(tester);
        }
        if (const ImageDataLocal* tester = dynamic_cast<const ImageDataLocal*>(source)) {
            return convert<ImageDataLocal, TO>(tester);
        }
        return 0;
    }

    template<>
    campvis::ImageDataLocal* ImageDataConverter::convert(const ImageDataDisk* source);

    template<>
    campvis::ImageDataGL* ImageDataConverter::convert(const ImageDataDisk* source);

    template<>
    campvis::ImageDataGL* ImageDataConverter::convert(const ImageDataLocal* source);


    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>* campvis::ImageDataConverter::convertToGenericLocal(const ImageDataDisk* source) {
        if (TypeTraits<BASETYPE, NUMCHANNELS>::basetypeSize != WeaklyTypedPointer::numBytes(source->getBaseType())) {
            LERROR("Basetype size does not match.");
            return 0;
        }
        if (NUMCHANNELS != source->getNumChannels()) {
            LERROR("Number of channels does not match.");
            return 0;
        }
        // TODO: check (un)signed int/float?

        WeaklyTypedPointer wtp = source->getImageData();
        GenericImageDataLocal<BASETYPE, NUMCHANNELS>* toReturn = new GenericImageDataLocal<BASETYPE, NUMCHANNELS>(
            source->getDimensionality(), 
            source->getSize(), 
            reinterpret_cast<typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType*>(wtp._pointer));        
        toReturn->setMappingInformation(source->getMappingInformation());

        return toReturn;
    }

}

#endif // IMAGEDATACONVERTER_H__
