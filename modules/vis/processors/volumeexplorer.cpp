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

#include "volumeexplorer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"


#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string VolumeExplorer::loggerCat_ = "CAMPVis.modules.vis.VolumeExplorer";

    VolumeExplorer::VolumeExplorer(IVec2Property& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera")
        , p_xSlice("XSlice", "Slice in YZ Plane", 0, 0, 0)
        , p_ySlice("YSlice", "Slice in XZ Plane", 0, 0, 0)
        , p_zSlice("ZSlice", "Slice in XY Plane", 0, 0, 0)
        , p_transferFunction("TransferFunction", "Transfer Function", new SimpleTransferFunction(128))
        , p_outputImage("OutputImage", "Output Image", "ve.output", DataNameProperty::WRITE)
        , _raycaster(canvasSize)
        , _sliceExtractor(canvasSize)
    {
        addProperty(&p_inputVolume);
        addProperty(&p_camera);
        addProperty(&p_xSlice);
        addProperty(&p_ySlice);
        addProperty(&p_zSlice);
        addProperty(&p_inputVolume);
        addProperty(&p_outputImage);

        p_inputVolume.addSharedProperty(&_raycaster.p_inputVolume);
        p_inputVolume.addSharedProperty(&_sliceExtractor.p_sourceImageID);
        p_camera.addSharedProperty(&_raycaster.p_camera);
    }

    VolumeExplorer::~VolumeExplorer() {

    }

    void VolumeExplorer::init() {
        VisualizationProcessor::init();
        _raycaster.init();
        _sliceExtractor.init();
    }

    void VolumeExplorer::deinit() {
        _raycaster.deinit();
        _sliceExtractor.deinit();
        VisualizationProcessor::deinit();
    }

    void VolumeExplorer::process(DataContainer& data) {
        validate(INVALID_RESULT);
    }

}

