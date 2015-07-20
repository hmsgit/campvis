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

#include "itkimagefilter.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"
#include <itkMedianImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkSobelEdgeDetectionImageFilter.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkLaplacianSharpeningImageFilter.h>
#include <itkThresholdImageFilter.h>

#include <tbb/tbb.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"


// In this class we want to use various ITK filters. Each filter needs the same ITK boilerplate
// code to be written before and after calling the filter. Futhermore, we need to distinguish 
// between the different input base types, since ITK doesn't  know runtime type inference.
// Hence, we define various handy macros that will assemble the necessary C++ code for using the
// corresponding ITK filters within this processor. Good luck!

/**
 * Executes the specified filter on the data specified filter (in-out-filter).
 * \param MA_baseType       base type of input image
 * \param MA_returnType     base type of ouput image
 * \param MA_numChannels    number of channels of input image
 * \param MA_dimensionality dimensionality of images
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, MA_numChannels, MA_dimensionality, MA_filterType, MD_filterBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRep(data, p_sourceImageID.getValue()); \
    if (itkRep != 0) { \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType InputImageType; \
        typedef GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::ItkImageType OutputImageType; \
        itk::MA_filterType<InputImageType, OutputImageType>::Pointer filter = itk::MA_filterType<InputImageType, OutputImageType>::New(); \
        \
        MD_filterBody \
        \
        filter->SetInput(itkRep->getItkImage()); \
        filter->Update(); \
        GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::create(id, filter->GetOutput()); \
    } \
    }

/**
 * Executes the specified filter on the data specified filter (in-place filter)
 * \param MA_baseType       base type of input and output image
 * \param MA_numChannels    number of channels of input image
 * \param MA_dimensionality dimensionality of images
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define PERFORM_ITK_FILTER_SPECIFIC_INPLACE(MA_baseType, MA_numChannels, MA_dimensionality, MA_filterType, MD_filterBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRep(data, p_sourceImageID.getValue()); \
    if (itkRep != 0) { \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType ImageType; \
        itk::MA_filterType<ImageType>::Pointer filter = itk::MA_filterType<ImageType>::New(); \
        \
        MD_filterBody \
        \
        filter->SetInput(itkRep->getItkImage()); \
        filter->Update(); \
        GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::create(id, filter->GetOutput()); \
    } \
    }

// Multi-channel images not supported by most ITK processors...
#define DISPATCH_ITK_FILTER_BRD(MA_WTP, MA_baseType, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
    cgtAssert(MA_WTP._numChannels == 1, "ItkImageFilter only supports single-channel images.") \
    PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody)

#define DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, MA_baseType, MA_dimensionality, MA_filterType, MD_filterBody) \
    cgtAssert(MA_WTP._numChannels == 1, "ItkImageFilter only supports single-channel images.") \
    PERFORM_ITK_FILTER_SPECIFIC_INPLACE(MA_baseType, 1, MA_dimensionality, MA_filterType, MD_filterBody)


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
            cgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
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
            cgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \

#define DISPATCH_ITK_FILTER_INPLACE_D(MA_WTP, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._baseType) { \
        case WeaklyTypedPointer::UINT8: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, uint8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT8: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, int8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT16: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, uint16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT16: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, int16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::UINT32: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, uint32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::INT32: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, int32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        case WeaklyTypedPointer::FLOAT: \
            DISPATCH_ITK_FILTER_INPLACE_BD(MA_WTP, float, MA_dimensionality, MA_filterType, MD_filterBody) \
            break; \
        default: \
            cgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
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
            case 2: DISPATCH_ITK_FILTER_RD(wtp, MA_returnType, 2, MA_filterType, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_RD(wtp, MA_returnType, 3, MA_filterType, MD_filterBody) break; \
            default: cgtAssert(false, "Unsupported dimensionality!"); break; \
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
            case 2: DISPATCH_ITK_FILTER_D(wtp, 2, MA_filterType, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_D(wtp, 3, MA_filterType, MD_filterBody) break; \
            default: cgtAssert(false, "Unsupported dimensionality!"); break; \
        } \
    } while (0)

/**
 * Dispatches the execution for the in-place ITK filter \a MA_filterType for the image \a MA_localRep.
 * \param MA_localRep       local representation of the image to apply the filter to
 * \param MA_filterType     type name if the ITK filter to use (within itk:: namespace)
 * \param MD_filterBody     additional stuff to execute between filter definition and execution
 */
#define DISPATCH_ITK_FILTER_INPLACE(MA_localRep, MA_filterType, MD_filterBody) \
    do { \
        WeaklyTypedPointer wtp = MA_localRep->getWeaklyTypedPointer(); \
        switch (MA_localRep->getDimensionality()) { \
            case 2: DISPATCH_ITK_FILTER_INPLACE_D(wtp, 2, MA_filterType, MD_filterBody) break; \
            case 3: DISPATCH_ITK_FILTER_INPLACE_D(wtp, 3, MA_filterType, MD_filterBody) break; \
            default: cgtAssert(false, "Unsupported dimensionality!"); break; \
        } \
    } while (0)


// ================================================================================================
// = Macros defined, let the party begin!                                                         =
// ================================================================================================

namespace campvis {

    static const GenericOption<std::string> filterModes[7] = {
        GenericOption<std::string>("median", "Median"),
        GenericOption<std::string>("gauss", "Gauss"),
        GenericOption<std::string>("sobel", "Sobel"),
        GenericOption<std::string>("gradientDiffusion", "Gradient Anisotropic Diffusion"),
        GenericOption<std::string>("curvatureDiffusion", "Curvature Anisotropic Diffusion"),
        GenericOption<std::string>("laplacianSharpening", "Laplacian Sharpening"),
        GenericOption<std::string>("thresholding", "Thresholding")
    };

    const std::string ItkImageFilter::loggerCat_ = "CAMPVis.modules.classification.ItkImageFilter";

    ItkImageFilter::ItkImageFilter()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
        , p_filterMode("FilterMode", "Filter Mode", filterModes, 7)
        , p_kernelSize("KernelSize", "Kernel Size", 3, 3, 15)
        , p_sigma("Sigma", "Sigma", 1.f, .1f, 10.f, 0.1f)
        , p_numberOfSteps("NumberOfSteps", "Number of Steps", 5, 1, 15)
        , p_timeStep("TimeStep", "Time Step", .0625, .001f, .12499f, 0.001f)
        , p_conductance("Conductance", "Conductance", 1.f, .1f, 5.f, 0.1f)
        , p_thresMin("ThresholdMin", "Threshold Minimum", 0.1f, 0.0f, 1.0f, 0.05f)
        , p_thresMax("ThresholdMax", "Threshold Maximum", 0.9f, 0.0f, 1.0f, 0.05f)
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
        addProperty(p_filterMode, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_kernelSize);
        addProperty(p_sigma);
        addProperty(p_numberOfSteps);
        addProperty(p_timeStep);
        addProperty(p_conductance);
        addProperty(p_thresMin);
        addProperty(p_thresMax);
    }

    ItkImageFilter::~ItkImageFilter() {

    }


    void ItkImageFilter::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        
        if (input != 0 && input->getParent()->getNumChannels() == 1 && (input->getDimensionality() == 2 || input->getDimensionality() == 3)) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 1);

            if (p_filterMode.getOptionValue() == "median") {
                DISPATCH_ITK_FILTER(input, MedianImageFilter, \
                    InputImageType::SizeType indexRadius; \
                    indexRadius.Fill(1); \
                    filter->SetRadius(indexRadius); \
                    );
            }
            else if (p_filterMode.getOptionValue() == "gauss") {
// disable known false-positive warning in ITK code when using GCC:
#pragma GCC diagnostic ignored "-Warray-bounds"
                DISPATCH_ITK_FILTER(input, DiscreteGaussianImageFilter, \
                    filter->SetUseImageSpacing(false); \
                    filter->SetVariance(p_sigma.getValue()); \
                    );
            }
            else if (p_filterMode.getOptionValue() == "sobel") {
                DISPATCH_ITK_FILTER_WITH_EXTRA_RETURN_TYPE(input, float, SobelEdgeDetectionImageFilter, /* no body here */);
            }
            else if (p_filterMode.getOptionValue() == "gradientDiffusion") {
                DISPATCH_ITK_FILTER_WITH_EXTRA_RETURN_TYPE(input, float, GradientAnisotropicDiffusionImageFilter, \
                    filter->SetNumberOfIterations(p_numberOfSteps.getValue()); \
                    filter->SetTimeStep(p_timeStep.getValue()); \
                    filter->SetConductanceParameter(p_conductance.getValue()); \
                    );
            }
            else if (p_filterMode.getOptionValue() == "curvatureDiffusion") {
                DISPATCH_ITK_FILTER_WITH_EXTRA_RETURN_TYPE(input, float, CurvatureAnisotropicDiffusionImageFilter, \
                    filter->SetNumberOfIterations(p_numberOfSteps.getValue()); \
                    filter->SetTimeStep(p_timeStep.getValue()); \
                    filter->SetConductanceParameter(p_conductance.getValue()); \
                    );
            }
            if (p_filterMode.getOptionValue() == "laplacianSharpening") {
                DISPATCH_ITK_FILTER(input, LaplacianSharpeningImageFilter, /* nothing here */);
            }
            else if (p_filterMode.getOptionValue() == "thresholding") {
                DISPATCH_ITK_FILTER_INPLACE(input, ThresholdImageFilter, \
                    filter->ThresholdOutside(p_thresMin.getValue(), p_thresMax.getValue()); \
                    );
            }
            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void ItkImageFilter::updateProperties(DataContainer& /*dataContainer*/) {
        if (p_filterMode.getOptionValue() == "median") {
            p_kernelSize.setVisible(true);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
            p_thresMin.setVisible(false);
            p_thresMax.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "gauss") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(true);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
            p_thresMin.setVisible(false);
            p_thresMax.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "sobel" || p_filterMode.getOptionValue() == "laplacianSharpening") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
            p_thresMin.setVisible(false);
            p_thresMax.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "gradientDiffusion" || p_filterMode.getOptionValue() == "curvatureDiffusion") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(true);
            p_timeStep.setVisible(true);
            p_conductance.setVisible(true);
            p_thresMin.setVisible(false);
            p_thresMax.setVisible(false);
        }
        else if(p_filterMode.getOptionValue() == "thresholding") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
            p_thresMin.setVisible(true);
            p_thresMax.setVisible(true);
        }
    }

}
