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

#include "cudaconfidencemapssolver.h"

#include "core/datastructures/transformdata.h"
#include "core/datastructures/positiondata.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string CudaConfidenceMapsSolver::loggerCat_ = "CAMPVis.modules.cudaconfidencemaps.solver";

    CudaConfidenceMapsSolver::CudaConfidenceMapsSolver() 
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputConfidenceMap("OutputConfidenceMap", "Output Confidence Map", "us.confidence", DataNameProperty::WRITE)
    {

        addProperty(p_inputImage);
        addProperty(p_outputConfidenceMap);
        /*
        _stopExecution = false;
        _receiverRunning = false;

        addProperty(p_address, VALID);
        addProperty(p_port, VALID);
        addProperty(p_deviceName, VALID);

        addProperty(p_connect, VALID);
        addProperty(p_disconnect, VALID);

        addProperty(p_receiveTransforms, INVALID_PROPERTIES);
        addProperty(p_receiveImages, INVALID_PROPERTIES);
        addProperty(p_targetTransformPrefix, VALID);
        addProperty(p_targetImagePrefix, VALID);
        addProperty(p_imageOffset, VALID);
        addProperty(p_voxelSize, VALID);
        addProperty(p_receivePositions, INVALID_PROPERTIES);
        addProperty(p_targetPositionPrefix, VALID);

		invalidate(INVALID_PROPERTIES);*/


    }

    CudaConfidenceMapsSolver::~CudaConfidenceMapsSolver() {
        /*if (_receiverRunning)
            stopReceiver();

        if (_receiverThread)
        delete _receiverThread;*/
    }

    void CudaConfidenceMapsSolver::init() {/*
        p_connect.s_clicked.connect(this, &CudaConfidenceMapsSolver::connect);
        p_disconnect.s_clicked.connect(this, &CudaConfidenceMapsSolver::disconnect);
        p_disconnect.setVisible(false);*/
    }

    void CudaConfidenceMapsSolver::deinit() {/*
        stopReceiver();
        p_connect.s_clicked.disconnect(this);
        p_disconnect.s_clicked.disconnect(this);*/
    }

    void CudaConfidenceMapsSolver::updateResult(DataContainer& data) {
/*
        if(p_receiveTransforms.getValue()) 
        {
            _transformMutex.lock();
            for(auto it = _receivedTransforms.begin(), end = _receivedTransforms.end(); it != end; ++it) { 
                TransformData * td = new TransformData(it->second);
        
                data.addData(p_targetTransformPrefix.getValue() + it->first, td);
#ifdef IGTL_CLIENT_DEBUGGING
                LDEBUG("Transform data put into container. ");
#endif
            }
            _receivedTransforms.clear();
            _transformMutex.unlock();
        }

        if(p_receiveImages.getValue()) 
        {
            _imageMutex.lock();
            for(auto it = _receivedImages.begin(), end = _receivedImages.end(); it != end; ++it)
            {
                igtl::ImageMessage::Pointer imageMessage = it->second;    
                WeaklyTypedPointer wtp;
                wtp._pointer = new uint8_t[imageMessage->GetImageSize()];
#ifdef IGTL_CLIENT_DEBUGGING
                LDEBUG("Image has " << imageMessage->GetNumComponents() << " components and is of size " << imageMessage->GetImageSize());
#endif
                memcpy(wtp._pointer, imageMessage->GetScalarPointer(), imageMessage->GetImageSize());
                wtp._numChannels = imageMessage->GetNumComponents();

                switch (imageMessage->GetScalarType()) {
                case igtl::ImageMessage::TYPE_INT8:
                    wtp._baseType = WeaklyTypedPointer::INT8; break;
                case igtl::ImageMessage::TYPE_UINT8:
                    wtp._baseType = WeaklyTypedPointer::UINT8; break;
                case igtl::ImageMessage::TYPE_INT16:
                    wtp._baseType = WeaklyTypedPointer::INT16; break;
                case igtl::ImageMessage::TYPE_UINT16:
                    wtp._baseType = WeaklyTypedPointer::UINT16; break;
                case igtl::ImageMessage::TYPE_INT32:
                    wtp._baseType = WeaklyTypedPointer::INT32; break;
                case igtl::ImageMessage::TYPE_UINT32:
                    wtp._baseType = WeaklyTypedPointer::UINT32; break;
                case igtl::ImageMessage::TYPE_FLOAT32:
                    wtp._baseType = WeaklyTypedPointer::FLOAT; break;
                default:
                    LERROR("Error while receiving IGTL IMAGE message: unsupported type: " << imageMessage->GetScalarType());
                    return;
                }

                tgt::vec3 imageOffset(0.f);
                tgt::vec3 voxelSize(1.f);
                tgt::ivec3 size_i(1);

                imageMessage->GetSpacing(voxelSize.elem);
                imageMessage->GetDimensions(size_i.elem);
                tgt::svec3 size(size_i);                
                imageMessage->GetOrigin(imageOffset.elem);

                size_t dimensionality = (size_i[2] == 1) ? ((size_i[1] == 1) ? 1 : 2) : 3;
                ImageData* image = new ImageData(dimensionality, size, wtp._numChannels);
                ImageRepresentationLocal::create(image, wtp);

                image->setMappingInformation(ImageMappingInformation(size, p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue()));
                data.addData(p_targetImagePrefix.getValue() + it->first, image);
            }
            _receivedImages.clear();
            _imageMutex.unlock();
        }

        if(p_receivePositions.getValue())
        {
            _positionMutex.lock();
            for(auto it = _receivedPositions.begin(), end = _receivedPositions.end(); it != end; ++it) 
            {
                PositionData * pd = new PositionData(it->second._position, it->second._quaternion);
                data.addData(p_targetPositionPrefix.getValue() + it->first, pd);
            }
            _receivedPositions.clear();
            _positionMutex.unlock();
        }

        validate(INVALID_RESULT);*/
    }

    void CudaConfidenceMapsSolver::updateProperties(DataContainer& dataContainer) {
        /*p_targetImagePrefix.setVisible(p_receiveImages.getValue());

        p_imageOffset.setVisible(p_receiveImages.getValue());
        p_voxelSize.setVisible(p_receiveImages.getValue());
        p_targetTransformPrefix.setVisible(p_receiveImages.getValue() || p_receiveTransforms.getValue());
		p_targetPositionPrefix.setVisible(p_receivePositions.getValue());

        validate(INVALID_PROPERTIES);*/
    }
}