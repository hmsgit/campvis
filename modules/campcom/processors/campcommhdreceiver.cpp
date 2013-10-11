// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "campcommhdreceiver.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

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
        _ccclient->disconnect();
        delete _ccclient;
        _ccclient = 0;
    }

    void CampcomMhdReceiver::process(DataContainer& data) {
        // all parsing done - lets create the image:
//         ImageData* image = new ImageData(dimensionality, size, numChannels);
//         ImageRepresentationDisk::create(image, url, pt, offset, e);
//         image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue()));
//         data.addData(p_targetImageID.getValue(), image);

        validate(INVALID_RESULT);
    }

    void CampcomMhdReceiver::onBtnConnectClicked() {
        if (_ccclient) {
            _ccclient->disconnect();
            delete _ccclient;
            _ccclient = 0;
        }

        _ccclient = new campcom::CAMPComClient("Campvis", campcom::Device_CAMPVis, p_address.getValue());
        _ccclient->connect();
        if (_ccclient->isConnected()) {
            campcom::DataCallback dc;
            dc = boost::bind(&CampcomMhdReceiver::ccReceiveImage, this, _1);
            campcom::SuccessCallback sc = std::bind1st(std::mem_fun(&CampcomMhdReceiver::ccSuccessCalback), this);
            _ccclient->subscribe(campcom::Type_Image, dc, sc);
        }
        else {
            LWARNING("Could not connect to CAMPCom server.");
            delete _ccclient;
            _ccclient = 0;
        }
    }

    void CampcomMhdReceiver::ccReceiveImage(std::vector<campcom::Byte>& msg) {

    }

    void CampcomMhdReceiver::ccSuccessCalback(bool b) {
        LINFO("CAMPCom subscribe callback: " << b);
    }

}