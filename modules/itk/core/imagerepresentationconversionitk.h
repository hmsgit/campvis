// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef IMAGEREPRESENTATIONCONVERSIONITK_H__
#define IMAGEREPRESENTATIONCONVERSIONITK_H__

#include "cgt/logmanager.h"

#include "core/coreapi.h"
#include "core/datastructures/imagerepresentationconversioncore.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "modules/itk/core/genericimagerepresentationitk.h"

#include <itkImportImageFilter.h>

namespace campvis {

// = Declare converter classes ====================================================================

        /// Conversion class to convert to ImageRepresentationGL.
        struct CAMPVIS_CORE_API GlFromItkConversion {
            static ImageRepresentationGL* tryConvertFrom(const AbstractImageRepresentation* source);
        };

        /// Conversion class to convert to ImageRepresentationLocal.
        struct CAMPVIS_CORE_API LocalFromItkConversion {
            static ImageRepresentationLocal* tryConvertFrom(const AbstractImageRepresentation* source);
        };

        /// Conversion class to convert to GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>.
        template<typename BASETYPE, size_t NUMCHANNELS>
        struct GenericLocalFromItkConversion {
            static GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* tryConvertFrom(const AbstractImageRepresentation* source);
        };

        /// Conversion class to convert to GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>.
        template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
        struct GenericItkConversion {
            static GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* tryConvertFrom(const AbstractImageRepresentation* source);
        };

// = Instantiate converter templates to register converters =======================================

        // Register converters with corresponding target representations
        template class ConversionFunctionRegistrar<ImageRepresentationGL, GlFromItkConversion>;
        template class ConversionFunctionRegistrar<ImageRepresentationLocal, LocalFromItkConversion>;

        // for GenericImageRepresentationLocal<> we use some macro magic to instantiate all necessary converters:
#define INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(BASETYPE, NUMCHANNELS) template class ConversionFunctionRegistrar< GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> , GenericLocalFromItkConversion<BASETYPE, NUMCHANNELS> >
#define INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_N(NUMCHANNELS) \
    INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(uint8_t, NUMCHANNELS);  INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(int8_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(uint16_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(int16_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(uint32_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(int32_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_BN(float, NUMCHANNELS);

        INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_N(1);
        INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_N(2);
        INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_N(3);
        INSTANTIATE_TEMPLATE_ITK_FROM_LOCAL_N(4);


        // same for conversion to GenericImageRepresentationItk<>
#define INSTANTIATE_TEMPLATE_GENERIC_ITK_BND(BASETYPE, NUMCHANNELS, DIMENSIONALITY) template class ConversionFunctionRegistrar< GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY> , GenericItkConversion<BASETYPE, NUMCHANNELS, DIMENSIONALITY> >

#define INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(BASETYPE, NUMCHANNELS) \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BND(BASETYPE, NUMCHANNELS, 2); \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BND(BASETYPE, NUMCHANNELS, 3); 

#define INSTANTIATE_TEMPLATE_GENERIC_ITK_N(NUMCHANNELS) \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(uint8_t, NUMCHANNELS);  INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(int8_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(uint16_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(int16_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(uint32_t, NUMCHANNELS); INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(int32_t, NUMCHANNELS); \
    INSTANTIATE_TEMPLATE_GENERIC_ITK_BN(float, NUMCHANNELS);

        INSTANTIATE_TEMPLATE_GENERIC_ITK_N(1);
        INSTANTIATE_TEMPLATE_GENERIC_ITK_N(2);
        INSTANTIATE_TEMPLATE_GENERIC_ITK_N(3);
        INSTANTIATE_TEMPLATE_GENERIC_ITK_N(4);


// = Template definition ==========================================================================

        template<typename BASETYPE, size_t NUMCHANNELS>
        GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* GenericLocalFromItkConversion<BASETYPE, NUMCHANNELS>::tryConvertFrom(const AbstractImageRepresentation* source) {
            GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* toReturn = nullptr;

            // we convert only from ITK representations
            if (source == nullptr || dynamic_cast<const AbstractImageRepresentationItk*>(source) == nullptr)
                return nullptr;

            // we perform the conversion in two steps to reuse existing code:
            // We first convert to ImageRepresentationLocal of matching type, which we then convert 
            // to the target type if necessary.
            // This is not very efficient in terms of performance (costs twice CPU and RAM) but
            // needs very little code. :)
            ImageRepresentationLocal* localRepWithMatchingType = LocalFromItkConversion::tryConvertFrom(source);

            // check whether type already matches
            toReturn = dynamic_cast< GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* >(localRepWithMatchingType);
            if (toReturn == nullptr) {
                // does not match, hence we perform the second conversion
                toReturn = GenericLocalConversion<BASETYPE, NUMCHANNELS>::tryConvertFrom(localRepWithMatchingType);
            }

            return toReturn;
        }

        template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
        GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericItkConversion<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::tryConvertFrom(const AbstractImageRepresentation* source) {
            if (source == nullptr)
                return nullptr;

            if (source->getDimensionality() != DIMENSIONALITY) {
                LWARNINGC("campvis.modules.itk.GenericItkConversion::tryConvertFrom", "Dimensionality does not match");
                return nullptr;
            }

            typedef typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkElementType ItkElementType;
            typedef typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkImageType ItkImageType;

            if (const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* tester = dynamic_cast< const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* >(source)) {
                typename itk::ImportImageFilter<ItkElementType, DIMENSIONALITY>::Pointer importer = itk::ImportImageFilter<ItkElementType, DIMENSIONALITY>::New();

                typename ItkImageType::SizeType size;
                size[0] = tester->getSize().x;
                if (DIMENSIONALITY >= 2)
                    size[1] = tester->getSize().y;
                if (DIMENSIONALITY >= 3)
                    size[2] = tester->getSize().z;

                typename ItkImageType::IndexType start;
                start.Fill(0);

                typename ItkImageType::RegionType region;
                region.SetSize(size);
                region.SetIndex(start);
                importer->SetRegion(region);

                importer->SetSpacing(tester->getParent()->getMappingInformation().getVoxelSize().elem);
                importer->SetOrigin(tester->getParent()->getMappingInformation().getOffset().elem);

                typedef typename ItkImageType::PixelType PixelType;
                const PixelType* pixelData = reinterpret_cast<const PixelType*>(tester->getImageData());
                importer->SetImportPointer(const_cast<PixelType*>(pixelData), tester->getNumElements(), false);
                importer->Update();

                typename ItkImageType::Pointer itkImage = importer->GetOutput();
                if (itkImage.IsNotNull())
                    return GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::create(const_cast<ImageData*>(tester->getParent()), itkImage); // const_cast perfectly valid here
                else
                    return nullptr;
            }

            return nullptr;
        }

}

#endif // IMAGEREPRESENTATIONCONVERSIONITK_H__
