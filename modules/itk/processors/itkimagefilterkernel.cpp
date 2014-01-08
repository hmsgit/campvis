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

#include "itkimagefilterkernel.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>

#include <tbb/tbb.h>

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
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define PERFORM_ITK_FILTER_KERNEL(MA_baseType, MA_returnType, MA_numChannels, MA_dimensionality, MA_filterType, MD_filterBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRep(data, p_sourceImageID.getValue()); \
    if (itkRep != 0) { \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType InputImageType; \
        typedef GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::ItkImageType OutputImageType; \
        typedef itk::BinaryBallStructuringElement<MA_baseType, MA_dimensionality> StructuringElementType; \
        \
        StructuringElementType structuringElement; structuringElement.SetRadius(p_kernelSize.getValue()); \
        structuringElement.CreateStructuringElement(); \
        itk::MA_filterType<InputImageType, OutputImageType, StructuringElementType>::Pointer filter = itk::MA_filterType<InputImageType, OutputImageType, StructuringElementType>::New(); \
        \
        MD_filterBody \
        \
        filter->SetKernel(structuringElement); \
        filter->SetInput(itkRep->getItkImage()); \
        filter->Update(); \
        GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::create(id, filter->GetOutput()); \
    } \
    }

#define DISPATCH_ITK_FILTER_BRD(MA_WTP, MA_baseType, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._numChannels) { \
        case 1 : PERFORM_ITK_FILTER_KERNEL(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 2 : PERFORM_ITK_FILTER_KERNEL(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 3 : PERFORM_ITK_FILTER_KERNEL(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 4 : PERFORM_ITK_FILTER_KERNEL(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
    }

#define DISPATCH_ITK_FILTER_RD(MA_WTP, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._baseType) { \
        case WeaklyTypedPointer::UINT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint8_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int8_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint16_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int16_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint32_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int32_t, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::FLOAT: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, float, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        default: \
            tgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \

#define DISPATCH_ITK_FILTER_D(MA_WTP, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._baseType) { \
        case WeaklyTypedPointer::UINT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint8_t, uint8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT8: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int8_t, int8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint16_t, uint16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT16: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int16_t, int16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, uint32_t, uint32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT32: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, int32_t, int32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::FLOAT: \
            DISPATCH_ITK_FILTER_BRD(MA_WTP, float, float, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        default: \
            tgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \


/**
 * Dispatches the execution for the ITK filter \a MA_filterType with the output base type
 * of \a MA_returnType for the image \a MA_localRep.
 * \param MA_localRep       local representation of the image to apply the filter to
 * \param MA_returnType     base type of ouput image
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define DISPATCH_ITK_FILTER_WITH_EXTRA_RETURN_TYPE(MA_localRep, MA_returnType, MA_filterType, MD_filterBody) \
    do { \
        WeaklyTypedPointer wtp = MA_localRep->getWeaklyTypedPointer(); \
        switch (MA_localRep->getDimensionality()) { \
            case 1: DISPATCH_ITK_FILTER_RD(wtp, MA_returnType, 1, MA_filterType, MD_filterBody) break; \
            case 2: DISPATCH_ITK_FILTER_RD(wtp, MA_returnType, 2, MA_filterType, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_RD(wtp, MA_returnType, 3, MA_filterType, MD_filterBody) break; \
        } \
    } while (0)

/**
 * Dispatches the execution for the ITK filter \a MA_filterType for the image \a MA_localRep.
 * \param MA_localRep       local representation of the image to apply the filter to
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define DISPATCH_ITK_FILTER(MA_localRep, MA_filterType, MD_filterBody) \
    do { \
        WeaklyTypedPointer wtp = MA_localRep->getWeaklyTypedPointer(); \
        switch (MA_localRep->getDimensionality()) { \
            case 1: DISPATCH_ITK_FILTER_D(wtp, 1, MA_filterType, MD_filterBody) break; \
            case 2: DISPATCH_ITK_FILTER_D(wtp, 2, MA_filterType, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_D(wtp, 3, MA_filterType, MD_filterBody) break; \
        } \
    } while (0)

// ================================================================================================
// = Macros defined, let the party begin!                                                         =
// ================================================================================================

namespace campvis {

    static const GenericOption<std::string> filterModes[2] = {
        GenericOption<std::string>("opening", "Opening"),
        GenericOption<std::string>("closing", "Closing")
    };

    const std::string ItkImageFilterKernel::loggerCat_ = "CAMPVis.modules.classification.ItkImageFilterKernel";

    ItkImageFilterKernel::ItkImageFilterKernel()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
        , p_filterMode("FilterMode", "Filter Mode", filterModes, 2, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_kernelSize("KernelSize", "Kernel Size", 3, 3, 15)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_filterMode);
        addProperty(&p_kernelSize);
    }

    ItkImageFilterKernel::~ItkImageFilterKernel() {

    }


    void ItkImageFilterKernel::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        
        if (input != 0 && input->getParent()->getNumChannels() == 1) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 1);

            if (p_filterMode.getOptionValue() == "opening") {
                DISPATCH_ITK_FILTER(input, BinaryMorphologicalOpeningImageFilter, /* nothing here */);
            }
            else if (p_filterMode.getOptionValue() == "closing") {
                DISPATCH_ITK_FILTER(input, BinaryMorphologicalClosingImageFilter, /* nothing here */);
            }

            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        validate(INVALID_RESULT);
    }

}
