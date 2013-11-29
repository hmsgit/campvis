// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef CAMPCOMMHDRECEIVER_H__
#define CAMPCOMMHDRECEIVER_H__

#include <string>

#include <CommonLib/Main/CAMPComDefinitions.h>
#include <CommonLib/DataTypes/MHDImageData.hpp>
#include <ClientLib/src/CAMPComClient.hpp>

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

        /**
         * Transforms the last received MHD image (found in _incomingMhd) into CAMPVis ImageData 
         * and stores it in \a data.
         * \param   data    DataContainer to work on
         */
        virtual void process(DataContainer& data);

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
