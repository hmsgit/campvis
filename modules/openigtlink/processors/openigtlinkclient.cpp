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

#include "core/datastructures/transformdata.h"
#include "core/datastructures/positiondata.h"

#include <igtlTransformMessage.h>
#include <igtlPositionMessage.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string OpenIGTLinkClient::loggerCat_ = "CAMPVis.modules.openigtlink.client";

    OpenIGTLinkClient::OpenIGTLinkClient() 
        : AbstractProcessor()
        , p_address("ServerAddress", "Server Address", "127.0.0.1")
        , p_port("ServerPort", "Server Port", 18944, 1, 65535, 1)
        , p_deviceName("ServerDeviceName", "Device Name (empty to accept all)")
        , p_connect("Connect", "Connect to Server")
        , p_disconnect("Disconnect", "Disconnect from Server")
        , p_receiveImages("ReceiveImages", "Receive IMAGE Messages", false)
        , p_targetImagePrefix("targetImageName", "Target Image Prefix", "IGTL.image.")
        , p_receiveTransforms("ReceiveTransforms", "Receive TRANSFORM Messages", true)
        , p_targetTransformPrefix("targetTransformPrefix", "Target Transform Prefix", "IGTL.transform.")
        , p_imageOffset("ImageOffset", "Image Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
        , p_receivePositions("ReceivePositions", "Receive POSITION Messages", true)
        , p_targetPositionPrefix("targetPositionsPrefix", "Target Position Prefix", "IGTL.position.")
        , _stopExecution()
        , _receiverThread(nullptr)
        , _receiverRunning()
    {
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

        invalidate(INVALID_PROPERTIES);


    }

    OpenIGTLinkClient::~OpenIGTLinkClient() {
        if (_receiverRunning)
            stopReceiver();
    }

    void OpenIGTLinkClient::init() {
        p_connect.s_clicked.connect(this, &OpenIGTLinkClient::connect);
        p_disconnect.s_clicked.connect(this, &OpenIGTLinkClient::disconnect);
        p_disconnect.setVisible(false);
    }

    void OpenIGTLinkClient::deinit() {
        stopReceiver();
        p_connect.s_clicked.disconnect(this);
        p_disconnect.s_clicked.disconnect(this);
    }

    void OpenIGTLinkClient::updateResult(DataContainer& data) {

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

                cgt::vec3 imageOffset(0.f);
                cgt::vec3 voxelSize(1.f);
                cgt::ivec3 size_i(1);

                imageMessage->GetSpacing(voxelSize.elem);
                imageMessage->GetDimensions(size_i.elem);
                cgt::svec3 size(size_i);
                imageMessage->GetOrigin(imageOffset.elem);

                // If the voxel size boundled with the packet is practically 0.0f, make it 1.0f
                // this makes sure we don't get invalid mapping informations (non invertable matrix)
                if (minElem(voxelSize) <= 1e-10f) {
                    voxelSize = cgt::vec3(1.0f);
                }

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

        validate(INVALID_RESULT);
    }

    void OpenIGTLinkClient::updateProperties(DataContainer& dataContainer) {
        p_targetImagePrefix.setVisible(p_receiveImages.getValue());

        p_imageOffset.setVisible(p_receiveImages.getValue());
        p_voxelSize.setVisible(p_receiveImages.getValue());
        p_targetTransformPrefix.setVisible(p_receiveImages.getValue() || p_receiveTransforms.getValue());
        p_targetPositionPrefix.setVisible(p_receivePositions.getValue());

        validate(INVALID_PROPERTIES);
    }

    void OpenIGTLinkClient::connect() {

        if(_socket && _socket->GetConnected()) {
            LWARNING("Already connected!");
            return;
        }

        _socket = igtl::ClientSocket::New();
        int r = _socket->ConnectToServer(p_address.getValue().c_str(), p_port.getValue());

        if (r != 0)
        {
            LERROR("Cannot connect to the server " << p_address.getValue() << ":" << p_port.getValue());
            _socket = nullptr;
            return;
        }

        LINFO("Connected to server " << p_address.getValue() << ":" << p_port.getValue());

        startReceiver(); //start receiving data in a new thread

        validate(INVALID_RESULT);
    }

    void OpenIGTLinkClient::disconnect() {
        stopReceiver(); //sends the stop message to the receiver thread
        LINFO("Disconnected.");
    }

    int OpenIGTLinkClient::ReceiveTransform(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
#ifdef IGTL_CLIENT_DEBUGGING
        LDEBUG("Receiving TRANSFORM data type.");
#endif
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
            cgt::mat4 mtx;
            // Retrieve the transform data (this cast is a bit dubious but should be ok judging from the respective class internals)
            transMsg->GetMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx.elem));
#ifdef IGTL_CLIENT_DEBUGGING
            igtl::PrintMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx.elem));
            std::cerr << std::endl;
#endif

            _transformMutex.lock();
            _receivedTransforms[transMsg->GetDeviceName()] = mtx;
            _transformMutex.unlock();

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;
    }

    int OpenIGTLinkClient::ReceivePosition(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
#ifdef IGTL_CLIENT_DEBUGGING
        LDEBUG("Receiving POSITION data type.");
#endif

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
            PositionMessageData pmd;

            positionMsg->GetPosition(pmd._position.elem);
            positionMsg->GetQuaternion(pmd._quaternion.elem);
            

            _positionMutex.lock();
            _receivedPositions[positionMsg->GetDeviceName()] = pmd;
            _positionMutex.unlock();


#ifdef IGTL_CLIENT_DEBUGGING
            std::cerr << "position   = (" << pmd._position[0] << ", " << pmd._position[1] << ", " << pmd._position[2] << ")" << std::endl;
            std::cerr << "quaternion = (" << pmd._quaternion[0] << ", " << pmd._quaternion[1] << ", "
                << pmd._quaternion[2] << ", " << pmd._quaternion[3] << ")" << std::endl << std::endl;
#endif

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;
    }

    int OpenIGTLinkClient::ReceiveImage(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
    {
#ifdef IGTL_CLIENT_DEBUGGING
        LDEBUG("Receiving IMAGE data type.");
#endif

        // Create a message buffer to receive transform data
        igtl::ImageMessage::Pointer imgMsg;
        imgMsg = igtl::ImageMessage::New();
        imgMsg->SetMessageHeader(header);
        imgMsg->AllocatePack();

        // Receive image data from the socket
        socket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

        // Deserialize the data
        // If you want to skip CRC check, call Unpack() without argument.
        int c = imgMsg->Unpack(1);

        if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
        {
            // put the message pointer into our locked buffer
            _imageMutex.lock();
            _receivedImages[imgMsg->GetDeviceName()] = imgMsg;
            _imageMutex.unlock();

            // Retrieve the image data
            int   size[3];          // image dimension
            float spacing[3];       // spacing (mm/pixel)
            int   svsize[3];        // sub-volume size
            int   svoffset[3];      // sub-volume offset
        
            imgMsg->GetDimensions(size);
            imgMsg->GetSpacing(spacing);
            imgMsg->GetSubVolume(svsize, svoffset);

            cgt::mat4 mtx;
            // Retrieve the transform data (this cast is a bit dubious but should be ok judging from the class internals)
            imgMsg->GetMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx.elem));
            
            _transformMutex.lock();
            _receivedTransforms[imgMsg->GetDeviceName()] = mtx;
            _transformMutex.unlock();

#ifdef IGTL_CLIENT_DEBUGGING
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


            igtl::PrintMatrix(*reinterpret_cast<igtl::Matrix4x4*>(mtx.elem));
            std::cout << std::endl;
#endif

            invalidate(INVALID_RESULT);
            return 1;
        }

        return 0;

    }

    void OpenIGTLinkClient::runReceiverThread()
    {
        igtl::MessageHeader::Pointer headerMsg;
        igtl::TimeStamp::Pointer ts;

        headerMsg = igtl::MessageHeader::New();
        ts = igtl::TimeStamp::New();

        p_connect.setVisible(false);
        p_disconnect.setVisible(true);

        while (!_stopExecution && _socket && _socket->GetConnected())
        {
            // Initialize receive buffer
            headerMsg->InitPack();

            // Receive generic header from the socket
            int r = _socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
            if (r == 0)
            {
                _socket->CloseSocket();
                _socket = nullptr;
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
                LDEBUG("Received IGTL Message Type " << headerMsg->GetDeviceType() << " which is not Handled!");
                _socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
        }
        if(_socket)       
            _socket->CloseSocket();
        _socket = nullptr;
        p_disconnect.setVisible(false);
        p_connect.setVisible(true);
    }


    void OpenIGTLinkClient::stopReceiver() {
        if (!_receiverRunning || _receiverThread == 0)
            return;

        _stopExecution = true;
        try {
            if (_receiverThread->joinable())
                _receiverThread->join();

            _receiverRunning = false;
            delete _receiverThread;
            _receiverThread = nullptr;
        }
        catch (std::exception& e) {
            LERRORC("CAMPVis.modules.base.MatrixProcessor", "Caught exception during _thread.join: " << e.what());
        }
    }

    void OpenIGTLinkClient::startReceiver() {
        stopReceiver(); //make sure we are stopped and reset
        _stopExecution = false;

        _receiverThread = new std::thread([this]() {
            runReceiverThread();
        });
        _receiverRunning = true;
    }
}