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

#define CAMPCOM_FAST_SERIALIZATION

#include "campcommhdreceiver.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string CampcomMhdReceiver::loggerCat_ = "CAMPVis.modules.io.CampcomMhdReceiver";

    CampcomMhdReceiver::CampcomMhdReceiver() 
        : AbstractProcessor()
        , p_address("ServerAddress", "Server Address", "127.0.0.1")
        , p_connect("Connect", "Connect to Server", AbstractProcessor::VALID)
        , p_targetImageID("targetImageName", "Target Image ID", "CampcomMhdReceiver.output", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f), tgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(0.1f))
        , _ccclient(0)
    {
        _incomingMhd = 0;

        addProperty(&p_address);
        addProperty(&p_targetImageID);
        addProperty(&p_connect);
        addProperty(&p_imageOffset);
        addProperty(&p_voxelSize);

    }

    CampcomMhdReceiver::~CampcomMhdReceiver() {

    }

    void CampcomMhdReceiver::init() {
        p_connect.s_clicked.connect(this, &CampcomMhdReceiver::onBtnConnectClicked);
    }

    void CampcomMhdReceiver::deinit() {
        p_connect.s_clicked.disconnect(this);
        if (_ccclient) {
            _ccclient->disconnect();
            delete _ccclient;
        }
        _ccclient = 0;
    }

    void CampcomMhdReceiver::updateResult(DataContainer& data) {
        validate(INVALID_RESULT);

        // Get the last received MHD file:
        // Use atomic fetch and store because at the same time CAMPCom may receive another file!
        campcom::MHDImageData* mid = _incomingMhd.fetch_and_store(0);
        if (mid == 0)
            return;

        // Transform campcom::MHDImageData to campvis::ImageData
        int numChannels = 1;
        size_t dimensionality = mid->NDims;
        tgt::svec3 size(1);
        for (int i = 0; i < mid->DimSize.size(); ++i)
            size.elem[i] = mid->DimSize[i];

        WeaklyTypedPointer wtp;
        wtp._pointer = &(mid->imageData.front().front());
        wtp._numChannels = numChannels;

        std::string et = StringUtils::uppercase(mid->elementType);
        if (et == "MET_UCHAR")
            wtp._baseType = WeaklyTypedPointer::UINT8;
        else if (et == "MET_CHAR")
            wtp._baseType = WeaklyTypedPointer::INT8;
        else if (et == "MET_USHORT")
            wtp._baseType = WeaklyTypedPointer::UINT16;
        else if (et == "MET_SHORT")
            wtp._baseType = WeaklyTypedPointer::INT16;
        else if (et == "MET_UINT")
            wtp._baseType = WeaklyTypedPointer::UINT32;
        else if (et == "MET_INT")
            wtp._baseType = WeaklyTypedPointer::INT32;
        else if (et == "MET_FLOAT")
            wtp._baseType = WeaklyTypedPointer::FLOAT;
        else {
            LERROR("Error while parsing MHD header: Unsupported element type: " << et);
            return;
        }

        tgt::vec3 imageOffset(0.f);
        tgt::vec3 voxelSize(1.f);

        // all parsing done - lets create the image:
        ImageData* image = new ImageData(dimensionality, size, numChannels);
        ImageRepresentationLocal::create(image, wtp);
        image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue()));
        data.addData(p_targetImageID.getValue(), image);
    }

    void CampcomMhdReceiver::onBtnConnectClicked() {
        // CAMPComClient does not support dis-/reconnect. So we have to delete it and recreate it.
        if (_ccclient) {
            _ccclient->disconnect();
            delete _ccclient;
            _ccclient = 0;
        }

        // create CAMPComClient and subscribe.
        _ccclient = new campcom::CAMPComClient("Campvis", campcom::Device_TestDevice, p_address.getValue());
        _ccclient->connect();

        if (_ccclient->isConnected()) {
            // use ugly boost magic to connect to member function (unfortunately CAMPCom only supports free functions...)
            campcom::DataCallback dc = boost::bind(&CampcomMhdReceiver::ccReceiveImage, this, _1);
            campcom::SuccessCallback sc = std::bind1st(std::mem_fun(&CampcomMhdReceiver::ccSuccessCallback), this);
            _ccclient->subscribe(campcom::Type_Image, dc, sc);
        }
        else {
            LWARNING("Could not connect to CAMPCom server.");
            delete _ccclient;
            _ccclient = 0;
        }
    }

    void CampcomMhdReceiver::ccReceiveImage(std::vector<campcom::Byte>& msg) {
        // Deserialize payload
        campcom::MHDImageData return_payload;
        campcom::Header header;
        campcom::TypeHandler<campcom::MHDImageData>::deserializePayload(&msg[0], msg.size(), return_payload);

        if (campcom::MHDImageData::isValid(return_payload)) {
            LINFO("New valid MHDImageData received! Pushing it to the DataContainer...");

            // putting the image into the DataContainer has to be done asynchroneously, because we
            // don't know the DataContainer here... :/
            // So copy the image one more time, but it into _incomingMhd and invalidate the processor.
            // Use atomic fetch and store because at the same time we may convert the last received image!
            campcom::MHDImageData* copy = new campcom::MHDImageData(return_payload);

            // delete old image not yet converted (if present)
            campcom::MHDImageData* toDelete = _incomingMhd.fetch_and_store(copy);
            delete toDelete;

            invalidate(INVALID_RESULT);
        }
        else {
            LWARNING("New MHDImageData received but it isn't valid!");
        }

    }

    void CampcomMhdReceiver::ccSuccessCallback(bool b) {
        LINFO("CAMPCom subscribe callback: " << b);
    }

}