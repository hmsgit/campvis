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

#ifndef ITKWRAPPER_H__
#define ITKWRAPPER_H__

#include "tgt/assert.h"

#include "core/tools/typetraits.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkRGBPixel.h>

#include <cstring> // memcpy

namespace campvis {

    class ItkWrapper {
    public:
        template<typename T>
        static typename itk::Image<T, 3>::Pointer convertImageFromCampvisToItk(const ImageData* image);

        template<typename T>
        static ImageData* convertImageFromItkToCampvis(const typename itk::Image<T, 3>::Pointer image);
    };

// ================================================================================================

    template<typename T>
    typename itk::Image<T, 3>::Pointer ItkWrapper::convertImageFromCampvisToItk(const ImageData* image) {
        tgtAssert(image != 0, "Image must not be 0.");

        const GenericImageRepresentationLocal<T, 1>* representation = image->getRepresentation< GenericImageRepresentationLocal<T, 1> >();
        if (representation == 0)
            return 0;

        typename itk::ImportImageFilter<T, 3>::Pointer importer = itk::ImportImageFilter<T, 3>::New();

        typename itk::Image<T, 3>::SizeType size;
        size[0] = image->getSize().x;
        size[1] = image->getSize().y;
        size[2] = image->getSize().z;

        typename itk::Image<T, 3>::IndexType start;
        start.Fill(0);

        typename itk::Image<T, 3>::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);
        importer->SetRegion(region);

        importer->SetSpacing(image->getMappingInformation().getVoxelSize().elem);
        importer->SetOrigin(image->getMappingInformation().getOffset().elem);

        typedef typename itk::Image<T, 3>::PixelType PixelType;
        const PixelType* pixelData = representation->getImageData();
        importer->SetImportPointer(const_cast<PixelType*>(pixelData), image->getNumElements(), false);
        importer->Update();

        return importer->GetOutput();
    }

    template<typename T>
    ImageData* campvis::ItkWrapper::convertImageFromItkToCampvis(const typename itk::Image<T, 3>::Pointer image) {
        typedef typename itk::Image<T, 3>::PixelType PixelType;
        const PixelType* pixelData = image->GetBufferPointer();

        typename itk::Image<T, 3>::RegionType region;
        region = image->GetBufferedRegion();

        typename itk::Image<T, 3>::SizeType s = region.GetSize();
        tgt::svec3 size(s[0], s[1], s[2]);

        typename itk::Image<T, 3>::SpacingType sp = image->GetSpacing();
        tgt::vec3 spacing(sp[0], sp[1], sp[2]);

        typename itk::Image<T, 3>::PointType o = image->GetOrigin();
        tgt::vec3 offset(o[0], o[1], o[2]);

        ImageData* toReturn = new ImageData(3, size, 1);
        toReturn->setMappingInformation(ImageMappingInformation(size, offset, spacing));

        PixelType* pixelDataCopy = new PixelType[toReturn->getNumElements()];
        memcpy(pixelDataCopy, pixelData, toReturn->getNumElements() * TypeTraits<T, 1>::elementSize);

        new GenericImageRepresentationLocal<PixelType, 1>(toReturn, pixelDataCopy);

        return toReturn;
    }

}

#endif // ITKWRAPPER_H__

