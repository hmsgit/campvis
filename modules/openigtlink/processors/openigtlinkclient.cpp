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

#include "openigtlinkclient.h"

#include "transformdata.h"
#include "positiondata.h"

#include <igtlTransformMessage.h>
#include <igtlPositionMessage.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string OpenIGTLinkClient::loggerCat_ = "CAMPVis.modules.openigtlink.client";

    OpenIGTLinkClient::OpenIGTLinkClient() 
        : AbstractProcessor()
        , p_address("ServerAddress", "Server Address", "127.0.0.1", StringProperty::BASIC_STRING)
        , p_port("ServerPort", "Server Port", 18944, 1, 65535, 1)
        , p_deviceName("ServerDeviceName", "Device Name (empty to accept all)")
        , p_connect("Connect", "Connect to Server")
        , p_receiveImages("ReceiveImages", "Receive IMAGE Messages", false)
        , p_targetImageID("targetImageName", "Target Image ID", "OpenIGTLinkClient.output", DataNameProperty::WRITE)
        , p_receiveTransforms("ReceiveTransforms", "Receive TRANSFORM Messages", true)
        , p_targetTransformID("targetTransformName", "Target Transform ID", "OpenIGTLinkClient.transform", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f), tgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(0.1f))
        , p_receivePositions("ReceivePositions", "Receive POSITION Messages", true)
        , p_targetPositionID("targetPositionsID", "Target Position ID", "OpenIGTLinkClient.position", DataNameProperty::WRITE)
    {
        _lastReceivedTransform = 0;
        _lastReceivedImageMessage = 0;

        addProperty(p_address, VALID);
        addProperty(p_port, VALID);
        addProperty(p_deviceName, VALID);

        addProperty(p_connect, VALID);

        addProperty(p_receiveTransforms, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_receiveImages, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetTransformID, VALID);
        addProperty(p_targetImageID, VALID);
        addProperty(p_imageOffset, VALID);
        addProperty(p_voxelSize, VALID);
        addProperty(p_receivePositions, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetPositionID, VALID);
    }

    OpenIGTLinkClient::~OpenIGTLinkClient() {

    }

    void OpenIGTLinkClient::init() {
        p_connect.s_clicked.connect(this, &OpenIGTLinkClient::onBtnConnectClicked);
    }

    void OpenIGTLinkClient::deinit() {
        p_connect.s_clicked.disconnect(this);
        stop(); //stop the receiving thread
    }

    void OpenIGTLinkClient::updateResult(DataContainer& data) {

        if(p_receiveTransforms.getValue()) 
        {
            tgt::mat4 * newTransform = _lastReceivedTransform.fetch_and_store(nullptr);
            if(newTransform) {
                TransformData * td = new TransformData(*newTransform);
        
                data.addData(p_targetTransformID.getValue(), td);
                delete newTransform; //was allocated in the receiver thread - not needed anymore

                LDEBUG("Transform data put into container. ");
            }
        }

        if(p_receiveImages.getValue()) 
        {
            _lastReceivedImageMessageMutex.lock();
            igtl::ImageMessage::Pointer imageMessage = _lastReceivedImageMessage;    
            _lastReceivedImageMessageMutex.unlock();

            if(imageMessage) 
            {
                WeaklyTypedPointer wtp;
                wtp._pointer = new uint8_t[imageMessage->GetImageSize()];
                LDEBUG("Image has " << imageMessage->GetNumComponents() << " components and is of size " << imageMessage->GetImageSize());
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
                imageMessage->GetDimensions(size_i.elem);
                tgt::svec3 size(size_i);

                size_t dimensionality = (size_i[2] == 1) ? ((size_i[1] == 1) ? 1 : 2) : 3;
                ImageData* image = new ImageData(dimensionality, size, wtp._numChannels);
                ImageRepresentationLocal::create(image, wtp);
                image->setMappingInformation(ImageMappingInformation(size, p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue()));
                data.addData(p_targetImageID.getValue(), image);
            }
        }

        if(p_receivePositions.getValue())
        {
            _lastReceivedPositionMutex.lock();
            PositionData * pd = new PositionData(_lastReceivedPosition, _lastReceivedQuaternion);
            _lastReceivedPositionMutex.unlock();

            data.addData(p_targetPositionID.getValue(), pd);
        }

        validate(INVALID_RESULT);
    }

    void OpenIGTLinkClient::updateProperties(DataContainer& dataContainer) {
        p_targetImageID.setVisible(p_receiveImages.getValue());

        p_imageOffset.setVisible(p_receiveImages.getValue());
        p_voxelSize.setVisible(p_receiveImages.getValue());
        p_targetTransformID.setVisible(p_receiveImages.getValue() || p_receiveTransforms.getValue());
    }

    void OpenIGTLinkClient::onBtnConnectClicked() {

        if(_socket && _socket->GetConnected()) {
            LWARNING("Already connected!");
            return;
        }


        _socket = igtl::ClientSocket::New();
        int r = _socket->ConnectToServer(p_address.getValue().c_str(), p_port.getValue());

        if (r != 0)
        {
            LERROR("Cannot connect to the server " << p_address.getValue());
            return;
        }

        LINFO("Connected to server!");

        start(); //start receiving data in a new thread

        validate(INVALID_RESULT);
    }

    int OpenIGTLinkClient::ReceiveTransform(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
        LDEBUG("Receiving TRANSFORM data type.");

        // Create a message buffer to receive transform data
        igtl::TransformMessage::Pointer transMsg;
        transMsg = igtl::TransformMessage::New();
        transMsg->SetMessageHeader(header);
        transMsg->AllocatePack();

        // Receive transform data from the socket
        socket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize());

        // Deserialize the transform data
        // If you want to skip CRC check, call Unpack() without argument.
        int c = transMsg->Unpack(1);

        if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
        {
            tgt::mat4 * mtx = new tgt::mat4;
            // Retrieve the transform data (this cast is a bit dubious but should be ok judging from the class internals)
            transMsg->GetMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx->elem));
            igtl::PrintMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx->elem));
            std::cerr << std::endl;

            tgt::mat4 * toDelete = _lastReceivedTransform.fetch_and_store(mtx);
            if(toDelete) delete toDelete;

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;
    }

    int OpenIGTLinkClient::ReceivePosition(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
        LDEBUG("Receiving POSITION data type.");

        // Create a message buffer to receive position data
        igtl::PositionMessage::Pointer positionMsg;
        positionMsg = igtl::PositionMessage::New();
        positionMsg->SetMessageHeader(header);
        positionMsg->AllocatePack();

        // Receive position position data from the socket
        socket->Receive(positionMsg->GetPackBodyPointer(), positionMsg->GetPackBodySize());

        // Deserialize the transform data
        // If you want to skip CRC check, call Unpack() without argument.
        int c = positionMsg->Unpack(1);

        if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
        {
            _lastReceivedPositionMutex.lock();
            positionMsg->GetPosition(_lastReceivedPosition.elem);
            positionMsg->GetQuaternion(_lastReceivedQuaternion.elem);
            _lastReceivedPositionMutex.unlock();

            std::cerr << "position   = (" << _lastReceivedPosition[0] << ", " << _lastReceivedPosition[1] << ", " << _lastReceivedPosition[2] << ")" << std::endl;
            std::cerr << "quaternion = (" << _lastReceivedQuaternion[0] << ", " << _lastReceivedQuaternion[1] << ", "
                << _lastReceivedQuaternion[2] << ", " << _lastReceivedQuaternion[3] << ")" << std::endl << std::endl;

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;
    }

    int OpenIGTLinkClient::ReceiveImage(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
        LDEBUG("Receiving IMAGE data type.");

        // Create a message buffer to receive transform data
        igtl::ImageMessage::Pointer imgMsg;
        imgMsg = igtl::ImageMessage::New();
        imgMsg->SetMessageHeader(header);
        imgMsg->AllocatePack();

        // Receive image data from the socket
        socket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

        // Deserialize the transform data
        // If you want to skip CRC check, call Unpack() without argument.
        int c = imgMsg->Unpack(1);

        if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
        {
            // put the message pointer into our locked buffer
            _lastReceivedImageMessageMutex.lock();
            _lastReceivedImageMessage = imgMsg;
            _lastReceivedImageMessageMutex.unlock();

            // Retrieve the image data
            int   size[3];          // image dimension
            float spacing[3];       // spacing (mm/pixel)
            int   svsize[3];        // sub-volume size
            int   svoffset[3];      // sub-volume offset
            int   scalarType;       // scalar type

            scalarType = imgMsg->GetScalarType();
            imgMsg->GetDimensions(size);
            imgMsg->GetSpacing(spacing);
            imgMsg->GetSubVolume(svsize, svoffset);

            tgt::mat4 * mtx = new tgt::mat4;
            // Retrieve the transform data (this cast is a bit dubious but should be ok judging from the class internals)
            imgMsg->GetMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx->elem));
            std::cerr << std::endl;

            tgt::mat4 * toDelete = _lastReceivedTransform.fetch_and_store(mtx);
            if(toDelete) delete toDelete;

            std::cerr << "Device Name           : " << imgMsg->GetDeviceName() << std::endl;
            std::cerr << "Scalar Type           : " << scalarType << std::endl;
            std::cerr << "Dimensions            : ("
                << size[0] << ", " << size[1] << ", " << size[2] << ")" << std::endl;
            std::cerr << "Spacing               : ("
                << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << ")" << std::endl;
            std::cerr << "Sub-Volume dimensions : ("
                << svsize[0] << ", " << svsize[1] << ", " << svsize[2] << ")" << std::endl;
            std::cerr << "Sub-Volume offset     : ("
                << svoffset[0] << ", " << svoffset[1] << ", " << svoffset[2] << ")" << std::endl << std::endl;

            igtl::PrintMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx->elem));

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;

    }

    void OpenIGTLinkClient::run()
    {
        igtl::MessageHeader::Pointer headerMsg;
        igtl::TimeStamp::Pointer ts;

        headerMsg = igtl::MessageHeader::New();
        ts = igtl::TimeStamp::New();

        while (!_stopExecution)
        {
            // Initialize receive buffer
            headerMsg->InitPack();

            // Receive generic header from the socket
            int r = _socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
            if (r == 0)
            {
                _socket->CloseSocket();
                LINFO("Socket Connection closed.");
                break;
            }
            if (r != headerMsg->GetPackSize())
            {
                continue;
            }

            // Deserialize the header
            headerMsg->Unpack();

            // Get time stamp
            igtlUint32 sec;
            igtlUint32 nanosec;

            headerMsg->GetTimeStamp(ts);
            ts->GetTimeStamp(&sec, &nanosec);

            /*std::cerr << "Time stamp: "
                << sec << "." << std::setw(9) << std::setfill('0') 
                << nanosec << std::endl;*/

            std::string acceptedName = p_deviceName.getValue();
            
            if (!acceptedName.empty() && strcmp(acceptedName.c_str(), headerMsg->GetDeviceName()) ) 
            {
                _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
                LDEBUG("Ignoring Message from device " << headerMsg->GetDeviceName() << ". Does not match " << acceptedName);
            }
            // Check data type and receive data body
            else if (strcmp(headerMsg->GetDeviceType(), "TRANSFORM") == 0)
            {
                if(p_receiveTransforms.getValue())
                    ReceiveTransform(_socket, headerMsg);
                else
                    _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
            else if (strcmp(headerMsg->GetDeviceType(), "POSITION") == 0)
            {
                if(p_receivePositions.getValue())
                    ReceivePosition(_socket, headerMsg);
                else
                    _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
            else if (strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0)
            {
                if(p_receiveImages.getValue())
                    ReceiveImage(_socket, headerMsg);
                else
                    _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
//            else if (strcmp(headerMsg->GetDeviceType(), "STATUS") == 0)
//            {
//                ReceiveStatus(socket, headerMsg);
//            }
//#if OpenIGTLink_PROTOCOL_VERSION >= 2
//            else if (strcmp(headerMsg->GetDeviceType(), "POINT") == 0)
//            {
//                ReceivePoint(socket, headerMsg);
//            }
//            else if (strcmp(headerMsg->GetDeviceType(), "TRAJ") == 0)
//            {
//                ReceiveTrajectory(socket, headerMsg);
//            }
//            else if (strcmp(headerMsg->GetDeviceType(), "STRING") == 0)
//            {
//                ReceiveString(socket, headerMsg);
//            }
//            else if (strcmp(headerMsg->GetDeviceType(), "TDATA") == 0)
//            {
//                ReceiveTrackingData(socket, headerMsg);
//            }
//            else if (strcmp(headerMsg->GetDeviceType(), "QTDATA") == 0)
//            {
//                ReceiveQuaternionTrackingData(socket, headerMsg);
//            }
//            else if (strcmp(headerMsg->GetDeviceType(), "CAPABILITY") == 0)
//            {
//                ReceiveCapability(socket, headerMsg);;
//            }
//#endif //OpenIGTLink_PROTOCOL_VERSION >= 2
            else
            {
                LDEBUG("Receiving : " << headerMsg->GetDeviceType() << " which is not Handled!");
                _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
        }
        _socket->CloseSocket();
    }
}