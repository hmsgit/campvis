// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "itksegmentation.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "modules/itk/core/genericimagerepresentationitk.h"

#include <itkIntTypes.h>
#include <itkCastImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

// In this class we want to use various ITK segmentation methods.

/**
* Executes the specified segmentation on the data.
* \param MA_baseType       base type of input image
* \param MA_returnType     base type of output image
* \param MA_numChannels    number of channels of input image
* \param MA_dimensionality dimensionality of images
* \param MD_filterBody     additional stuff to execute between filter definition and execution
*/
#define PERFORM_ITK_SEGMENTATION(MA_baseType, MA_returnType, MA_numChannels, MA_dimensionality, MA_filterType, MD_filterBody) \
    { \
    GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ScopedRepresentation itkRep(data, p_sourceImageID.getValue()); \
    if (itkRep != 0) { \
        typedef GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::ItkImageType InputImageType; \
        typedef GenericImageRepresentationItk<MA_returnType, MA_numChannels, MA_dimensionality>::ItkImageType OutputImageType; \
        itk::MA_filterType<InputImageType, OutputImageType>::Pointer filter = itk::MA_filterType<InputImageType, OutputImageType>::New(); \
        typedef itk::Image<itk::IdentifierType, MA_dimensionality> LabelImageType; \
        typedef itk::MaskImageFilter< OutputImageType, OutputImageType > MaskFilterType;\
        /*rescale the intensity values for the interval [0,255] (some images yield intensities outside this range)*/ \
        typedef itk::RescaleIntensityImageFilter< InputImageType, InputImageType > RescaleFilterType;\
        RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New(); \
        rescaleFilter->SetInput(itkRep->getItkImage()); \
        rescaleFilter->SetOutputMinimum(0); \
        rescaleFilter->SetOutputMaximum(255); \
        MaskFilterType::Pointer maskFilter = MaskFilterType::New(); \
        MD_filterBody \
        filter->SetInput(rescaleFilter->GetOutput()); \
        filter->Update(); \
        maskFilter->SetInput(rescaleFilter->GetOutput()); \
        maskFilter->SetMaskImage(filter->GetOutput());\
        itk::CastImageFilter<OutputImageType, OutputImageType>::Pointer caster = itk::CastImageFilter<OutputImageType, OutputImageType>::New(); \
        caster->SetInput(maskFilter->GetOutput()); \
        caster->Update(); \
        \
        GenericImageRepresentationItk<MA_baseType, MA_numChannels, MA_dimensionality>::create(id, caster->GetOutput()); \
    } \
    }

#define DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, MA_baseType, MA_returnType, MA_dimensionality, MA_filterType, MD_filterBody) \
    tgtAssert(MA_WTP._numChannels == 1, "ItkSegmentation only supports single-channel images.") \
    PERFORM_ITK_SEGMENTATION(MA_baseType, MA_returnType, 1, MA_dimensionality, MA_filterType, MD_filterBody)

#define DISPATCH_ITK_SEGMENTATION_D(MA_WTP, MA_dimensionality, MA_filterType, MD_filterBody) \
    switch (MA_WTP._baseType) { \
    case WeaklyTypedPointer::UINT8: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, uint8_t, uint8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::INT8: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, int8_t, int8_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::UINT16: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, uint16_t, uint16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::INT16: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, int16_t, int16_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::UINT32: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, uint32_t, uint32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::INT32: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, int32_t, int32_t, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    case WeaklyTypedPointer::FLOAT: \
    DISPATCH_ITK_SEGMENTATION_BRD(MA_WTP, float, float, MA_dimensionality, MA_filterType, MD_filterBody) \
    break; \
    default: \
    tgtAssert(false, "Should not reach this - wrong base type in WeaklyTypedPointer!"); \
    } \

/**
* Dispatches the execution for the ITK filter \a MA_filterType for the image \a MA_localRep.
* \param MA_localRep       local representation of the image to apply the filter to
* \param MA_filterType     type name of the ITK filter to use (within itk:: namespace)
* \param MD_filterBody     additional stuff to execute between filter definition and execution
*/
#define DISPATCH_ITK_SEGMENTATION(MA_localRep, MA_filterType, MD_filterBody) \
    do { \
    WeaklyTypedPointer wtp = MA_localRep->getWeaklyTypedPointer(); \
    switch (MA_localRep->getDimensionality()) { \
    case 2: DISPATCH_ITK_SEGMENTATION_D(wtp, 2, MA_filterType, MD_filterBody) break; \
    case 3: DISPATCH_ITK_SEGMENTATION_D(wtp, 3, MA_filterType, MD_filterBody) break; \
    default: tgtAssert(false, "Unsupported dimensionality!"); break; \
        } \
    } while (0)

// ================================================================================================
// = Macros defined, let the party begin!                                                         =
// ================================================================================================

namespace campvis {

    static const GenericOption<std::string> segmentationTypes[1] = {
        GenericOption<std::string>("regionGrowing", "Region Growing")
    };

    const std::string ItkSegmentation::loggerCat_ = "CAMPVis.modules.classification.ItkSegmentation";

    ItkSegmentation::ItkSegmentation(IVec2Property* viewportSizeProp)
    : VolumeExplorer(viewportSizeProp)
    , p_sourceImageID("InputSegmentationVolume", "Input Segmentation Volume ID", "volume", DataNameProperty::READ)
    , p_targetImageID("OutputSegmentationVolume", "Output Segmented Volume ID", "segmented_volume", DataNameProperty::WRITE)
    , p_segmentationType("SegmentationType", "Segmentation Type", segmentationTypes, 1)
    , p_seedX("SeedX", "Seed X", 0, 0, 0, 1)
    , p_seedY("SeedY", "Seed Y", 0, 0, 0, 1)
    , p_seedZ("SeedZ", "Seed Z", 0, 0, 0, 1)
    , p_thresMin("ThresMin", "Min Threshold", 70, 0, 255, 1)
    , p_thresMax("ThresMax", "Max Threshold", 130, 0, 255, 1)
    {
        addProperty(p_sourceImageID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetImageID);
        addProperty(p_segmentationType, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_seedX);
        addProperty(p_seedY);
        addProperty(p_seedZ);
        addProperty(p_thresMin);
        addProperty(p_thresMax);
        p_enableScribbling.setValue(true);
    }

    ItkSegmentation::~ItkSegmentation() {
    }

    void ItkSegmentation::updateResult(DataContainer& data) {
    	VolumeExplorer::updateResult(data);
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        
        if (input != 0 && input->getParent()->getNumChannels() == 1 && (input->getDimensionality() == 2 || input->getDimensionality() == 3)) {
            const size_t dim = input->getDimensionality();
            p_seedX.setMaxValue(input->getSize().elem[0]);
            p_seedY.setMaxValue(input->getSize().elem[1]);
            p_seedZ.setMaxValue(input->getSize().elem[2]);
            ImageData* id = new ImageData(dim, input->getSize(), 1);

            if (p_segmentationType.getOptionValue() == "regionGrowing") {

                if (dim == 2) {
#pragma GCC diagnostic ignored "-Warray-bounds"
                    DISPATCH_ITK_SEGMENTATION(input, ConnectedThresholdImageFilter, \
                        InputImageType::IndexType index; \
                        index[0] = p_seedX.getValue(); \
                        index[1] = p_seedY.getValue(); \
                        filter->SetLower(p_thresMin.getValue()); \
                        filter->SetUpper(p_thresMax.getValue()); \
                        filter->SetReplaceValue(255); \
                        filter->SetSeed(index); \
                        );
                } else if (dim == 3) {
#pragma GCC diagnostic ignored "-Warray-bounds"
                    DISPATCH_ITK_SEGMENTATION(input, ConnectedThresholdImageFilter, \
                        InputImageType::IndexType index; \
                        index[0] = p_seedX.getValue(); \
                        index[1] = p_seedY.getValue(); \
                        index[2] = p_seedZ.getValue(); \
                        filter->SetLower(p_thresMin.getValue()); \
                        filter->SetUpper(p_thresMax.getValue()); \
                        filter->SetReplaceValue(255); \
                        filter->SetSeed(index); \
                        );
                } else {
                    tgtAssert(false, "Unsupported dimensionality!");
                }
            }

            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void ItkSegmentation::updateProperties(DataContainer& data) {
        VolumeExplorer::updateProperties(data);

        if (p_segmentationType.getOptionValue() == "regionGrowing") {
            p_seedX.setVisible(true);
            p_seedY.setVisible(true);
            p_seedZ.setVisible(true);
            p_thresMin.setVisible(true);
            p_thresMax.setVisible(true);
        }
    }

    void ItkSegmentation::onEvent(tgt::Event* e) {
        VolumeExplorer::onEvent(e);
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (p_enableScribbling.getValue() && (me->modifiers() & tgt::Event::CTRL || me->modifiers() & tgt::Event::ALT)) {

                //update the input image for the segmentation (take the one that is explored by the VolumeExplorer)
                p_sourceImageID.setValue(p_inputVolume.getValue());

                // update the maximum size
                p_seedX.setMaxValue(_sliceExtractor.p_xSliceNumber.getMaxValue());
                p_seedY.setMaxValue(_sliceExtractor.p_ySliceNumber.getMaxValue());
                p_seedZ.setMaxValue(_sliceExtractor.p_zSliceNumber.getMaxValue());

                tgt::svec3 voxel;
                voxel = tgt::vec3(_yesScribbles[0]);
                p_seedX.setValue(voxel.x);
                p_seedY.setValue(voxel.y);
                p_seedZ.setValue(voxel.z);
            }
        }
    }

}
