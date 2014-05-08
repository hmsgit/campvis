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

#include <string>

#include <igtlOSUtil.h>
#include <igtlClientSocket.h>
#include <igtlMessageHeader.h>
#include <igtlImageMessage.h>

#include <tgt/matrix.h>

#include <tbb/atomic.h>
#include <tbb/mutex.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "core/tools/runnable.h"
#include "core/datastructures/imagedata.h"


namespace campvis {
    /**
     * Experimental demo implementation how to receive MHD files via CAMPCom, convert it to
     * CAMPVis ImageData and store it into the DataContainer.
     */
    class OpenIGTLinkClient : public AbstractProcessor, public Runnable {
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


        BoolProperty p_receiveImages;       ///< toggle receiving IMAGE messages
        DataNameProperty p_targetImageID;   ///< image ID for read image

        BoolProperty p_receiveTransforms;   ///< toggle receiving TRANSFORM messages
        DataNameProperty p_targetTransformID; ///< data ID for read transformation
        
        Vec3Property p_imageOffset;         ///< Image Offset in mm
        Vec3Property p_voxelSize;           ///< Voxel Size in mm

        

        /**
         * Updates the data container with the latest received frame/transformation
         * \param   dataContainer    DataContainer to work on
         */
        virtual void updateResult(DataContainer& dataContainer);

        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /// Callback slot for connect button
        void onBtnConnectClicked();
        
        static const std::string loggerCat_;

    protected:
        int OpenIGTLinkClient::ReceiveTransform(igtl::Socket * socket, igtl::MessageHeader::Pointer& header);
        int OpenIGTLinkClient::ReceivePosition(igtl::Socket * socket, igtl::MessageHeader::Pointer& header);
        int ReceiveImage(igtl::Socket * socket, igtl::MessageHeader::Pointer& header);

        virtual void run();

        //connection
        igtl::ClientSocket::Pointer _socket;
        
        //data
        tbb::atomic<tgt::mat4 *> _lastReceivedTransform;        ///< the last transform that has been received by the igtl worker thread
        igtl::ImageMessage::Pointer _lastReceivedImageMessage;  ///< last received igtl image message
        tbb::mutex _lastReceivedImageMessageMutex;              ///< mutex to control access to the _lastReceivedImageMessage pointer

    };

}

#endif // OPENIGTLINKCLIENT_H__

