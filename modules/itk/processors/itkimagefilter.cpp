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

#include "itkimagefilter.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"
#include <itkMedianImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkSobelEdgeDetectionImageFilter.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkLaplacianSharpeningImageFilter.h>

#include "tbb/tbb.h"

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

#define DISPATCH_ITK_FILTER_BRD(MA_WTP, MA_baseType, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._numChannels) { \
        case 1 : PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 2 : PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 3 : PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
        case 4 : PERFORM_ITK_FILTER_SPECIFIC(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody) break; \
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

    static const GenericOption<std::string> filterModes[6] = {
        GenericOption<std::string>("median", "Median"),
        GenericOption<std::string>("gauss", "Gauss"),
        GenericOption<std::string>("sobel", "Sobel"),
        GenericOption<std::string>("gradientDiffusion", "Gradient Anisotropic Diffusion"),
        GenericOption<std::string>("curvatureDiffusion", "Curvature Anisotropic Diffusion"),
        GenericOption<std::string>("laplacianSharpening", "Laplacian Sharpening")
    };

    const std::string ItkImageFilter::loggerCat_ = "CAMPVis.modules.classification.ItkImageFilter";

    ItkImageFilter::ItkImageFilter()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
        , p_filterMode("FilterMode", "Filter Mode", filterModes, 6, InvalidationLevel::INVALID_RESULT | InvalidationLevel::INVALID_PROPERTIES)
        , p_kernelSize("KernelSize", "Kernel Size", 3, 3, 15)
        , p_sigma("Sigma", "Sigma", 1.f, .1f, 10.f)
        , p_numberOfSteps("NumberOfSteps", "Number of Steps", 5, 1, 15)
        , p_timeStep("TimeStep", "Time Step", .0625, .001f, .12499f)
        , p_conductance("Conductance", "Conductance", 1.f, .1f, 5.f)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_filterMode);
        addProperty(&p_kernelSize);
        addProperty(&p_sigma);
        addProperty(&p_numberOfSteps);
        addProperty(&p_timeStep);
        addProperty(&p_conductance);
    }

    ItkImageFilter::~ItkImageFilter() {

    }


    void ItkImageFilter::process(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        
        if (input != 0 && input->getParent()->getNumChannels() == 1) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 1);

            if (p_filterMode.getOptionValue() == "median") {
                DISPATCH_ITK_FILTER(input, MedianImageFilter, \
                    InputImageType::SizeType indexRadius; \
                    indexRadius.Fill(1); \
                    filter->SetRadius(indexRadius); \
                    );
            }
            else if (p_filterMode.getOptionValue() == "gauss") {
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
            else if (p_filterMode.getOptionValue() == "laplacianSharpening") {
                DISPATCH_ITK_FILTER(input, LaplacianSharpeningImageFilter, /* nothing here */);
            }
            data.addData(p_targetImageID.getValue(), id);
            p_targetImageID.issueWrite();
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        _invalidationLevel.setValid();
    }

    void ItkImageFilter::updateProperties() {
        if (p_filterMode.getOptionValue() == "median") {
            p_kernelSize.setVisible(true);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "gauss") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(true);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "sobel" || p_filterMode.getOptionValue() == "laplacianSharpening") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(false);
            p_timeStep.setVisible(false);
            p_conductance.setVisible(false);
        }
        else if (p_filterMode.getOptionValue() == "gradientDiffusion" || p_filterMode.getOptionValue() == "curvatureDiffusion") {
            p_kernelSize.setVisible(false);
            p_sigma.setVisible(false);
            p_numberOfSteps.setVisible(true);
            p_timeStep.setVisible(true);
            p_conductance.setVisible(true);
        }
    }

}
