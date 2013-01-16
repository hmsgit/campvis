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

#ifndef IMAGEREPRESENTATIONCONVERTER_H__
#define IMAGEREPRESENTATIONCONVERTER_H__

#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationlocal.h"

#include <string>

namespace campvis {

    /**
     * Static class offering conversion between differen ImageRepresentation types
     */
    struct ImageRepresentationConverter {
    public:
        /**
         * Tries to convert \a source to \a TO ImageRepresentation type.
         * \note    Caller takes ownership of the created ImageRepresentation pointer.
         * \tparam  TO      Target conversion type.
         * \param   source  ImageRepresentation source for conversion.
         * \return  A pointer to the converted ImageRepresentation object, 0 if conversion failed.
         */
        template<class TO>
        static TO* tryConvert(const AbstractImageRepresentation* source);

        /**
         * Converts \a source to \a TO ImageRepresentation type.
         * \note    Caller takes ownership of the created ImageRepresentation pointer.
         * \note    Valid conversions have to be implemented using template specialization.
         * \tparam  FROM    Source conversion type.
         * \tparam  TO      Target conversion type.
         * \param   source  ImageRepresentation source for conversion.
         * \return  A pointer to the converted ImageRepresentation object, 0 if conversion failed.
         */
        template<class FROM, class TO>
        static TO* convert(const FROM* source);

    protected:
        /**
         * Helper function for creating a GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> from an ImageRepresentationDisk
         * \note    Caller takes ownership of the created ImageRepresentation pointer.
         * \tparam  BASETYPE    Image element base type
         * \tparam  NUMCHANNELS Number of channels per image element.
         * \param   source      ImageRepresentation source for conversion.
         * \return  A pointer to the converted GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> object, 0 if conversion failed.
         */
        template<typename BASETYPE, size_t NUMCHANNELS>
        static GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* convertToGenericLocal(const ImageRepresentationDisk* source);
        
        static const std::string loggerCat_;
    };

// ================================================================================================

    template<class FROM, class TO>
    TO* campvis::ImageRepresentationConverter::convert(const FROM* source) {
        tgtAssert(false, "Conversion not implemented.");
        return 0;
    }

    template<class TO>
    TO* campvis::ImageRepresentationConverter::tryConvert(const AbstractImageRepresentation* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
            return convert<ImageRepresentationDisk, TO>(tester);
        }
        if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(source)) {
            return convert<ImageRepresentationLocal, TO>(tester);
        }
        return 0;
    }

    template<>
    campvis::ImageRepresentationLocal* ImageRepresentationConverter::convert(const ImageRepresentationDisk* source);

    template<>
    campvis::ImageRepresentationGL* ImageRepresentationConverter::convert(const ImageRepresentationDisk* source);

    template<>
    campvis::ImageRepresentationGL* ImageRepresentationConverter::convert(const ImageRepresentationLocal* source);


    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::ImageRepresentationConverter::convertToGenericLocal(const ImageRepresentationDisk* source) {
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
        GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* toReturn = new GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>(
            source->getParent(), 
            reinterpret_cast<typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType*>(wtp._pointer));        

        return toReturn;
    }

}

#endif // IMAGEREPRESENTATIONCONVERTER_H__
