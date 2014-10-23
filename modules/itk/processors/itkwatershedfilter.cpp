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

#pragma warning(push)
#pragma warning(disable : 4996 4244) // disable unchecked iterators warnings in ITK code

#include "itkwatershedfilter.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"

#include <itkIntTypes.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkWatershedImageFilter.h>
#include <itkCastImageFilter.h>

#pragma warning(pop)

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"


// In this class we want to use various ITK filters. Each filter needs the same ITK boilerplate
// code to be written before and after calling the filter. Futhermore, we need to distinguish 
// between the different input base types, since ITK doesn't  know runtime type inference.
// Hence, we define various handy macros that will assemble the necessary C++ code for using the
// corresponding ITK filters within this processor. Good luck!

/**
 * Executes the specified filter on the data specified filter.
 * \param MA_baseType       base type of input image
 * \param MA_returnType     base type of ouput image
 * \param MA_numChannels    number of channels of input image
 * \param MA_dimensionality dimensionality of images
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define PERFORM_ITK_FILTER_WATERSHED(MA_baseType, MA_returnType, MA_numChannels, MA_dimensionality, MD_filterBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRep(data, p_sourceImageID.getValue()); \
    if (itkRep != 0) { \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType InputImageType; \
        typedef GenericImageRepresentationItk<float, MA_numChannels, MA_dimensionality>::ItkImageType FloatImageType; \
        typedef itk::Image<itk::IdentifierType, MA_dimensionality> LabelImageType; \
        typedef GenericImageRepresentationItk<uint16_t, MA_numChannels, MA_dimensionality>::ItkImageType OutputImageType; \
        \
        itk::GradientMagnitudeImageFilter<InputImageType, FloatImageType>::Pointer gradientMagnitudeImageFilter = itk::GradientMagnitudeImageFilter<InputImageType, FloatImageType>::New(); \
        gradientMagnitudeImageFilter->SetInput(itkRep->getItkImage()); \
        gradientMagnitudeImageFilter->Update(); \
        \
        itk::WatershedImageFilter<FloatImageType>::Pointer filter = itk::WatershedImageFilter<FloatImageType>::New(); \
        MD_filterBody \
        filter->SetInput(gradientMagnitudeImageFilter->GetOutput()); \
        filter->Update(); \
        \
        itk::CastImageFilter<LabelImageType, OutputImageType>::Pointer caster = itk::CastImageFilter<LabelImageType, OutputImageType>::New(); \
        caster->SetInput(filter->GetOutput()); \
        caster->Update(); \
        \
        GenericImageRepresentationItk<uint16_t, MA_numChannels, MA_dimensionality>::create(id, caster->GetOutput()); \
    } \
    }

#define DISPATCH_ITK_FILTER_BRD(MA_WTP, MA_baseType, MA_returnType, MA_dimensionality, MD_filterBody) \
    tgtAssert(MA_WTP._numChannels == 1, "ItkWatershedFilter only supports single-channel images.") \
    PERFORM_ITK_FILTER_WATERSHED(MA_baseType, MA_returnType, 1, MA_dimensionality, MD_filterBody)

#define DISPATCH_ITK_FILTER_D(MA_WTP, MA_dimensionality, MD_filterBody) \
    switch (MA_WTP._baseType) { \
        case WeaklyTypedPointer::UINT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint8_t, uint8_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int8_t, int8_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint16_t, uint16_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int16_t, int16_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint32_t, uint32_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int32_t, int32_t, MA_dimensionality, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::FLOAT: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, float, float, MA_dimensionality, MD_filterBody) \
            break; \
        default: \
            tgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \


/**
 * Dispatches the execution for the ITK filter \a MA_filterType for the image \a MA_localRep.
 * \param MA_localRep       local representation of the image to apply the filter to
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define DISPATCH_ITK_FILTER(MA_localRep, MD_filterBody) \
    do { \
        WeaklyTypedPointer wtp = MA_localRep->getWeaklyTypedPointer(); \
        switch (MA_localRep->getDimensionality()) { \
            case 2: DISPATCH_ITK_FILTER_D(wtp, 2, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_D(wtp, 3, MD_filterBody) break; \
            default: tgtAssert(false, "Unsupported dimensionality!"); break; \
        } \
    } while (0)

// ================================================================================================
// = Macros defined, let the party begin!                                                         =
// ================================================================================================

namespace campvis {

    const std::string ItkWatershedFilter::loggerCat_ = "CAMPVis.modules.classification.ItkWatershedFilter";

    ItkWatershedFilter::ItkWatershedFilter()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
        , p_threshold("Threshold", "Threshold", .005f, .001f, 1.f, .01f)
        , p_level("Level", "Level", .5f, .1f, 10.f, 1.f)
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
        addProperty(p_threshold);
        addProperty(p_level);
    }

    ItkWatershedFilter::~ItkWatershedFilter() {

    }


    void ItkWatershedFilter::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        
        if (input != 0 && input->getParent()->getNumChannels() == 1) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 1);

            // disable known false-positive warning in ITK code when using GCC:
#pragma GCC diagnostic ignored "-Warray-bounds"
            DISPATCH_ITK_FILTER(input, \
                filter->SetLevel(p_level.getValue()); \
                filter->SetThreshold(p_threshold.getValue()); \
                );

            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}
