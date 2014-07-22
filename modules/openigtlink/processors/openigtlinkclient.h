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

#ifndef OPENIGTLINKCLIENT_H__
#define OPENIGTLINKCLIENT_H__

//#define IGTL_CLIENT_DEBUGGING

#include <string>
#include <map>

#include <igtlOSUtil.h>
#include <igtlClientSocket.h>
#include <igtlMessageHeader.h>
#include <igtlImageMessage.h>

#include <tgt/matrix.h>

#include <tbb/compat/thread>
#include <tbb/atomic.h>
#include <tbb/mutex.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "core/datastructures/imagedata.h"


namespace campvis {
    /**
     * OpenIGTLink Client processor. Connects to a specified server and receives all OpenIGTLink messages.
     * Processes the messages according to the currently set properties p_receiveTransform, p_receivePositions
     * and p_receiveImage and puts them into the received data into the respective data containers.
     * This Class contains modified code from the OpenIGTLink ReceiveClient example.
     */
    class OpenIGTLinkClient : public AbstractProcessor {
    public:
        /**
         * Constructs a new CampcomMhdReceiver Processor
         **/
        OpenIGTLinkClient();

        /**
         * Destructor
         **/
        virtual ~OpenIGTLinkClient();

        /// \see AbstractProcessor::init()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "OpenIGTLinkClient"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "OpenIGTLink Client that can receive IMAGE and TRANSFORM messages"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Jakob Weiss <weissj@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        StringProperty p_address;           ///< server address to connect to
        NumericProperty<int> p_port;        ///< server port to connect to
        StringProperty p_deviceName;        ///< IGTL device name (leave empty to accept all)
        ButtonProperty p_connect;           ///< Connect button
        ButtonProperty p_disconnect;        ///< Disconnect button


        BoolProperty p_receiveImages;       ///< toggle receiving IMAGE messages
        StringProperty p_targetImagePrefix;   ///< image ID prefix for read image

        BoolProperty p_receiveTransforms;   ///< toggle receiving TRANSFORM messages
        StringProperty p_targetTransformPrefix; ///< data ID prefix for read transformation
        
        Vec3Property p_imageOffset;         ///< Image Offset in mm
        Vec3Property p_voxelSize;           ///< Voxel Size in mm

        BoolProperty p_receivePositions;       ///< toggle receiving IMAGE messages
        StringProperty p_targetPositionPrefix;   ///< image ID prefix for read images

        /**
         * Updates the data container with the latest received transformation/position/image data
         * \param   dataContainer    DataContainer to work on
         */
        virtual void updateResult(DataContainer& dataContainer);

        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /// Callback slot for connect button. can also be called from outside.
        void connect();
        /// Callback slot for disconnect button. can also be called from outside.
        void disconnect();
        
    protected:
        /// Stores received data from a POSITION Message
        struct PositionMessageData {
            tgt::vec3 _position;
            tgt::vec4 _quaternion;
        };

        /// Main method for the receiver thread. The new thread will
        /// go into a receive loop to receive the OpenIGTLink messages asynchronously
        virtual void runReceiverThread();

        /// Receive a TRANSFORM message from the OpenIGTLink socket and put the data into the local buffers
        int ReceiveTransform(igtl::Socket* socket, igtl::MessageHeader::Pointer& header);

        /// Receive a POSITION message from the OpenIGTLink socket and put the data into the local buffers
        int ReceivePosition(igtl::Socket* socket, igtl::MessageHeader::Pointer& header);

        /// Receive a IMAGE message from the OpenIGTLink socket and put into the local buffers
        int ReceiveImage(igtl::Socket* socket, igtl::MessageHeader::Pointer& header);

        //igtl connection
        igtl::ClientSocket::Pointer _socket;
        
        //data
        std::map<std::string, tgt::mat4> _receivedTransforms;        ///< the transforms that has been received by the igtl worker thread, mapped by device name
        std::map<std::string, igtl::ImageMessage::Pointer> _receivedImages;  ///< the image messages received by the igtl worker thread, mapped by device name
        std::map<std::string, PositionMessageData> _receivedPositions; ///< position message data received by the igtl worker thread, mapped by device name        

        tbb::mutex _transformMutex;                             ///< mutex to control access to _receivedTransforms
        tbb::mutex _imageMutex;                                 ///< mutex to control access to _receivedImages
        tbb::mutex _positionMutex;                              ///< mutex to control access to _receivedPositions

        static const std::string loggerCat_;

        // this is thread management stuff
        // very similar to the Runnable base class
    private:
        /// Start the receiver thread
        void startReceiver();
        /// stop the receiver thread
        void stopReceiver();

        tbb::atomic<bool> _stopExecution;       ///< Flag whether the thread should stop
        std::thread* _receiverThread;                    ///< Thread of the Runnable
        tbb::atomic<bool> _receiverRunning;
    };

}

#endif // OPENIGTLINKCLIENT_H__

