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

#ifndef CAMPCOMMHDRECEIVER_H__
#define CAMPCOMMHDRECEIVER_H__

#include <CommonLib/Main/CAMPComDefinitions.h>
#include <CommonLib/DataTypes/MHDImageData.hpp>
#include <ClientLib/src/CAMPComClient.hpp>

#include <string>

#include <tbb/atomic.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

namespace campvis {
    /**
     * Experimental demo implementation how to receive MHD files via CAMPCom, convert it to
     * CAMPVis ImageData and store it into the DataContainer.
     */
    class CampcomMhdReceiver : public AbstractProcessor {
    public:
        /**
         * Constructs a new CampcomMhdReceiver Processor
         **/
        CampcomMhdReceiver();

        /**
         * Destructor
         **/
        virtual ~CampcomMhdReceiver();

        /// \see AbstractProcessor::init()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CampcomMhdReceiver"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Experimental demo implementation how to receive MHD files via CAMPCom."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        StringProperty p_address;           ///< URL for file to read
        ButtonProperty p_connect;           ///< 
        DataNameProperty p_targetImageID;   ///< image ID for read image

        Vec3Property p_imageOffset;         ///< Image Offset in mm
        Vec3Property p_voxelSize;           ///< Voxel Size in mm

    protected:
        /**
         * Transforms the last received MHD image (found in _incomingMhd) into CAMPVis ImageData 
         * and stores it in \a data.
         * \param   dataContainer    DataContainer to work on
         */
        virtual void updateResult(DataContainer& dataContainer);

        /// Callback slot for connect button
        void onBtnConnectClicked();

        /**
         * Callback for CAMPCom when receiving an image.
         * \param   msg     Received CAMPCom message
         */
        void ccReceiveImage(std::vector<campcom::Byte>& msg);

        /**
         * Callback for CAMPCom when connection/subscribtion was successful.
         * \param   b   Flag whether subscription was successful.
         */
        void ccSuccessCallback(bool b);

        campcom::CAMPComClient* _ccclient;                  ///< Pointer to CAMPComClient (!=0 when connected)
        tbb::atomic<campcom::MHDImageData*> _incomingMhd;   ///< Pointer to last received MHD file

        static const std::string loggerCat_;
    };

}

#endif // CAMPCOMMHDRECEIVER_H__
