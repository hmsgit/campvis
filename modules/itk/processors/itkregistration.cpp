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

#include "itkregistration.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"

#include <itkIntTypes.h>
#include <itkCastImageFilter.h>
#include <itkImageRegistrationMethod.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkResampleImageFilter.h>
#include <itkVersorRigid3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkRescaleIntensityImageFilter.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

// In this class we want to use various ITK registration methods.

/**
* Executes the specified registration on the data.
* \param MA_baseType       base type of input images
* \param MA_returnType     base type of output image
* \param MA_numChannels    number of channels of input image
* \param MA_dimensionality dimensionality of images
* \param MA_registrationType   type name of the ITK registration to use 
* \param MA_transformationType type name of the ITK transformation to use (within itk:: namespace)
* \param MD_registrationBody   additional stuff to execute between registration definition and execution
*/
#define PERFORM_ITK_REGISTRATION(MA_baseType, MA_returnType, MA_numChannels, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRepFixed(data, p_sourceImageIDFixed.getValue()); \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRepMoving(data, p_sourceImageIDMoving.getValue()); \
    if ((MA_dimensionality == 3) && ( itkRepFixed != 0) && (itkRepMoving != 0)) { \
        typedef itk::VersorRigid3DTransformOptimizer OptimizerType; \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType InputImageType; \
        typedef GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::ItkImageType OutputImageType; \
        typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType; \
        typedef itk::ImageRegistrationMethod<InputImageType, InputImageType> RegistrationType; \
        typedef itk::MA_registrationType<InputImageType, InputImageType> MetricType; \
        typedef itk::ResampleImageFilter<InputImageType, InputImageType>    ResampleFilterType; \
        typedef itk::MA_transformationType<double> TransformType; \
        TransformType::Pointer transform = TransformType::New(); \
        OptimizerType::Pointer optimizer = OptimizerType::New(); \
        InterpolatorType::Pointer interpolator = InterpolatorType::New(); \
        RegistrationType::Pointer registration = RegistrationType::New(); \
        MetricType::Pointer metric = MetricType::New(); \
        \
        registration->SetOptimizer(optimizer); \
        registration->SetTransform(transform); \
        registration->SetInterpolator(interpolator); \
        registration->SetMetric(metric); \
        MD_registrationBody \
        registration->SetFixedImage(itkRepFixed->getItkImage()); \
        registration->SetMovingImage(itkRepMoving->getItkImage()); \
        registration->SetFixedImageRegion(itkRepFixed->getItkImage()->GetBufferedRegion()); \
        typedef itk::CenteredTransformInitializer<TransformType, InputImageType, InputImageType> TransformInitializerType; \
        TransformInitializerType::Pointer initializer = TransformInitializerType::New(); \
        initializer->SetTransform(transform); \
        initializer->SetFixedImage(itkRepFixed->getItkImage()); \
        initializer->SetMovingImage(itkRepMoving->getItkImage()); \
        initializer->MomentsOn(); \
        initializer->InitializeTransform(); \
        typedef TransformType::VersorType  VersorType; \
        typedef VersorType::VectorType     VectorType; \
        VersorType     rotation; \
        VectorType     axis; \
        axis[0] = 0.0; \
        axis[1] = 0.0; \
        axis[2] = 1.0; \
        const double angle = 0; \
        rotation.Set(axis, angle); \
        transform->SetRotation(rotation); \
        registration->SetInitialTransformParameters(transform->GetParameters()); \
        \
        typedef OptimizerType::ScalesType       OptimizerScalesType; \
        OptimizerScalesType optimizerScales(transform->GetNumberOfParameters()); \
        optimizer->MinimizeOn(); \
        const double translationScale = 1.0 / 1000.0; \
        optimizerScales[0] = 1.0; \
        optimizerScales[1] = 1.0; \
        optimizerScales[2] = 1.0; \
        optimizerScales[3] = translationScale; \
        optimizerScales[4] = translationScale; \
        optimizerScales[5] = translationScale; \
        optimizer->SetScales(optimizerScales); \
        optimizer->SetMaximumStepLength(0.2000); \
        optimizer->SetMinimumStepLength(0.0001); \
        optimizer->SetNumberOfIterations(200); \
        registration->Update(); \
        \
        OptimizerType::ParametersType finalParameters = registration->GetLastTransformParameters(); \
        transform->SetParameters(finalParameters); \
        TransformType::Pointer finalTransform = TransformType::New(); \
        finalTransform->SetCenter(transform->GetCenter()); \
        finalTransform->SetParameters(finalParameters); \
        finalTransform->SetFixedParameters(transform->GetFixedParameters()); \
        \
        ResampleFilterType::Pointer resample = ResampleFilterType::New(); \
        resample->SetTransform(finalTransform); \
        resample->SetInput(itkRepMoving->getItkImage()); \
        MA_baseType defaultPixelValue = 0; \
        resample->SetSize(itkRepFixed->getItkImage()->GetLargestPossibleRegion().GetSize()); \
        resample->SetOutputOrigin(itkRepFixed->getItkImage()->GetOrigin()); \
        resample->SetOutputSpacing(itkRepFixed->getItkImage()->GetSpacing()); \
        resample->SetOutputDirection(itkRepFixed->getItkImage()->GetDirection()); \
        resample->SetDefaultPixelValue(defaultPixelValue); \
        \
        itk::CastImageFilter<InputImageType, OutputImageType>::Pointer caster = itk::CastImageFilter<InputImageType, OutputImageType>::New(); \
        caster->SetInput(resample->GetOutput()); \
        caster->Update(); \
        \
        GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::create(id, caster->GetOutput()); \
    } \
    }

#define DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, MA_baseType, MA_returnType, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    tgtAssert(MA_WTPF._numChannels == 1, "ItkRegistration only supports single-channel images.") \
    PERFORM_ITK_REGISTRATION(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody)

#define DISPATCH_ITK_REGISTRATION_D(MA_WTPF, MA_WTPM, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    switch (MA_WTPF._baseType) { \
    case WeaklyTypedPointer::UINT8: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, uint8_t, uint8_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::INT8: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, int8_t, int8_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::UINT16: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, uint16_t, uint16_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::INT16: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, int16_t, int16_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::UINT32: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, uint32_t, uint32_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::INT32: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, int32_t, int32_t, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    case WeaklyTypedPointer::FLOAT: \
    DISPATCH_ITK_REGISTRATION_BRD(MA_WTPF, MA_WTPM, float, float, MA_dimensionality, MA_registrationType, MA_transformationType, MD_registrationBody) \
    break; \
    default: \
    tgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \

/**
* Dispatches the execution for the ITK registration based on transformation \a MA_transformationType
* and registration \a MA_RegistrationType for the images \a MA_localRepFixed and \a MA_localRepMoving.
* \param MA_localRepFixed      local representation of the fixed image to be registered with the moving one
* \param MA_localRepMoving     local representation of the moving image to be registered with the fixed one
* \param MA_registrationType   type name of the ITK registration to use 
* \param MA_transformationType type name of the ITK transformation to use (within itk:: namespace)
* \param MD_registrationBody   additional stuff to execute between registration definition and execution
*/
#define DISPATCH_ITK_REGISTRATION(MA_localRepFixed, MA_localRepMoving, MA_registrationType, MA_transformationType, MD_registrationBody) \
    do { \
    WeaklyTypedPointer wtpf = MA_localRepFixed->getWeaklyTypedPointer(); \
    WeaklyTypedPointer wtpm = MA_localRepMoving->getWeaklyTypedPointer(); \
    switch (MA_localRepFixed->getDimensionality()) { \
    case 3: DISPATCH_ITK_REGISTRATION_D(wtpf, wtpm, 3, MA_registrationType, MA_transformationType, MD_registrationBody) break; \
    default: tgtAssert(false, "Unsupported dimensionality!"); break; \
        } \
    } while (0)

// ================================================================================================
// = Macros defined, let the party begin!                                                         =
// ================================================================================================

namespace campvis {

    static const GenericOption<std::string> registrationTypes[1] = {
        GenericOption<std::string>("MattesMIRigid3D", "Mattes Mutual Information Rigid 3D")
    };

    const std::string ItkRegistration::loggerCat_ = "CAMPVis.modules.classification.ItkRegistration";

    ItkRegistration::ItkRegistration()
        : AbstractProcessor()
        , p_sourceImageIDFixed("InputVolumeFixed", "Fixed Input Volume ID", "volume_fixed", DataNameProperty::READ)
        , p_sourceImageIDMoving("InputVolumeMoving", "Moving Input Volume ID", "volume_moving", DataNameProperty::READ)
        , p_targetImageID("OutputRegistered", "Output Registered Volume ID", "registered_volume", DataNameProperty::WRITE)
        , p_registrationType("RegistrationType", "Registration Type", registrationTypes, 1)
        , p_noOfBins("NoOfBins", "No. of Bins", 20, 1, 256, 1)
        , p_noOfSamples("NoOfSampels", "No. of Samples", 10000, 1, 20000, 1)
    {
        addProperty(p_sourceImageIDFixed);
        addProperty(p_sourceImageIDMoving);
        addProperty(p_targetImageID);
        addProperty(p_registrationType, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_noOfBins);
    }

    ItkRegistration::~ItkRegistration() {

    }

    void ItkRegistration::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation inputFixed(data, p_sourceImageIDFixed.getValue());
        ImageRepresentationLocal::ScopedRepresentation inputMoving(data, p_sourceImageIDMoving.getValue());

        if (inputFixed != 0 && inputMoving != 0) {
            size_t dimInputFixed = inputFixed->getDimensionality();
            size_t dimInputMoving = inputMoving->getDimensionality();

            if(inputFixed->getParent()->getNumChannels() == 1 && 
                inputMoving->getParent()->getNumChannels() == 1 && 
                (dimInputFixed == dimInputMoving) && dimInputFixed == 3) {
                const size_t dim = dimInputFixed;
                ImageData* id = new ImageData(dim, inputFixed->getSize(), 1);

                if (p_registrationType.getOptionValue() == "MattesMIRigid3D") {

                    if (dim == 3) {
#pragma GCC diagnostic ignored "-Warray-bounds"
                        DISPATCH_ITK_REGISTRATION(inputFixed, inputMoving, MattesMutualInformationImageToImageMetric, VersorRigid3DTransform, \
                        unsigned long noOfBins = p_noOfBins.getValue(); \
                        unsigned long noOfSamples = p_noOfSamples.getValue(); \
                        metric->SetNumberOfHistogramBins(noOfBins); \
                        metric->SetNumberOfSpatialSamples(noOfSamples); \
                        );
                    }
                    else {
                        tgtAssert(false, "Unsupported dimensionality!");
                    }
                }

                data.addData(p_targetImageID.getValue(), id);
            }
            else {
                LDEBUG("No suitable input image found.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void ItkRegistration::updateProperties(DataContainer& /*dataContainer*/) {
        if (p_registrationType.getOptionValue() == "MattesMIRigid3D") {
            p_noOfBins.setVisible(true);
            p_noOfSamples.setVisible(true);
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
