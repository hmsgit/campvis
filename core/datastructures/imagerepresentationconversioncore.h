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

#ifndef IMAGEREPRESENTATIONCONVERSIONCORE_H__
#define IMAGEREPRESENTATIONCONVERSIONCORE_H__

#include "core/coreapi.h"
#include "core/tools/opengljobprocessor.h"

#include "core/datastructures/imagerepresentationconverter.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

// = Declare converter classes ====================================================================

        /// Conversion class to convert to ImageRepresentationGL.
        struct CAMPVIS_CORE_API GlConversion {
            static ImageRepresentationGL* tryConvertFrom(const AbstractImageRepresentation* source);
        };

        /// Conversion class to convert to ImageRepresentationLocal.
        struct CAMPVIS_CORE_API LocalConversion {
            static ImageRepresentationLocal* tryConvertFrom(const AbstractImageRepresentation* source);
        };

        /// Conversion class to convert to GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>.
        template<typename BASETYPE, size_t NUMCHANNELS>
        struct GenericLocalConversion {
            static GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* tryConvertFrom(const AbstractImageRepresentation* source);
        };

// = Instantiate converter templates to register converters =======================================

        // Register converters with corresponding target representations
        template class ConversionFunctionRegistrar<ImageRepresentationGL, GlConversion>;
        template class ConversionFunctionRegistrar<ImageRepresentationLocal, LocalConversion>;

        // for GenericImageRepresentationLocal we use some macro magic to instantiate all necessary converters:
#define INSTANTIATE_TEMPLATE_BN(BASETYPE, NUMCHANNELS) template class ConversionFunctionRegistrar< GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> , GenericLocalConversion<BASETYPE, NUMCHANNELS> >
#define INSTANTIATE_TEMPLATE_N(NUMCHANNELS) \
    INSTANTIATE_TEMPLATE_BN(uint8_t, NUMCHANNELS);  INSTANTIATE_TEMPLATE_BN(int8_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_BN(uint16_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_BN(int16_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_BN(uint32_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_BN(int32_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_BN(float, NUMCHANNELS);

        INSTANTIATE_TEMPLATE_N(1);
        INSTANTIATE_TEMPLATE_N(2);
        INSTANTIATE_TEMPLATE_N(3);
        INSTANTIATE_TEMPLATE_N(4);
		INSTANTIATE_TEMPLATE_N(6);


// = Template definition ==========================================================================

        template<typename BASETYPE, size_t NUMCHANNELS>
        GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* GenericLocalConversion<BASETYPE, NUMCHANNELS>::tryConvertFrom(const AbstractImageRepresentation* source) {
            typedef typename GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType ElementType;
            typedef typename GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ThisType ThisType;
            
            if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
                // converting from disk representation
                if (tester->getBaseType() == TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType && tester->getParent()->getNumChannels() == NUMCHANNELS) {
                    WeaklyTypedPointer wtp = tester->getImageData();
                    return GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::create(tester->getParent(), static_cast<ElementType*>(wtp._pointer));
                }
                else {
                    LWARNINGC("CAMPVis.core.datastructures.GenericLocalConversion", "Could not convert since base type or number of channels mismatch.");
                }
            }

            else if (const ImageRepresentationGL* tester = dynamic_cast<const ImageRepresentationGL*>(source)) {
                // converting from GL representation
                OpenGLJobProcessor::ScopedSynchronousGlJobExecution jobGuard;

                if (tester->getTexture()->getDataType() != TypeTraits<BASETYPE, NUMCHANNELS>::glDataType)
                    LDEBUGC("CAMPVis.core.datastructures.GenericLocalConversion", "Performing conversion between data types, you may lose information or the resulting data may show other unexpected features.");

                WeaklyTypedPointer wtp = tester->getWeaklyTypedPointerConvert(TypeTraits<BASETYPE, NUMCHANNELS>::glDataType);
                if (wtp._pointer != nullptr)
                    return GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::create(tester->getParent(), static_cast<ElementType*>(wtp._pointer));

                return nullptr;
            }

            else if (const ThisType* tester = dynamic_cast<const ThisType*>(source)) {
                // just to ensure that the following else if case is really a conversion
                LDEBUGC("CAMPVis.core.datastructures.GenericLocalConversion", "Trying to convert into the same type - this should not happen, since it there is no conversion needed...");
                return tester->clone(const_cast<ImageData*>(tester->getParent()));
            }

            else if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(source)) {
                // converting from other local representation of different data type
                // (we ensured with the else if above that at least one of the template parameters does not match)
                if (tester->getParent()->getNumChannels() == NUMCHANNELS) {
                    LDEBUGC("CAMPVis.core.datastructures.GenericLocalConversion", "Performing conversion between data types, you may lose information or the resulting data may show other unexpected features.");

                    size_t numElements = tester->getNumElements();
                    ElementType* newData = new ElementType[numElements];

                    // traverse each channel of each element and convert the value
                    for (size_t i = 0; i < numElements; ++i) {
                        for (size_t channel = 0; channel < NUMCHANNELS; ++channel) {
                            // get original value normalized to float
                            float tmp = tester->getElementNormalized(i, channel);
                            // save new value denormalized from float
                            TypeTraits<BASETYPE, NUMCHANNELS>::setChannel(newData[i], channel, TypeNormalizer::denormalizeFromFloat<BASETYPE>(tmp));
                        }                    
                    }

                    return GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::create(tester->getParent(), newData);
                }
                else {
                    LWARNINGC("CAMPVis.core.datastructures.GenericLocalConversion", "Could not convert since number of channels mismatch.");
                }
            }

            return nullptr;
        }

}

#endif // IMAGEREPRESENTATIONCONVERSIONCORE_H__
