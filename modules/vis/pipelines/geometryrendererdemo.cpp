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

#include "geometryrendererdemo.h"

#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/datastructures/geometrydatafactory.h"

namespace campvis {

    GeometryRendererDemo::GeometryRendererDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _geometryReader()
        , _gr(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_gr);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_geometryReader);
        addProcessor(&_gr);
    }

    GeometryRendererDemo::~GeometryRendererDemo() {
        delete _trackballEH;
    }

    void GeometryRendererDemo::init() {
        AutoEvaluationPipeline::init();
        
        _geometryReader.s_validated.connect(this, &GeometryRendererDemo::onProcessorValidated);

        _camera.addSharedProperty(&_gr.p_camera);
        _gr.p_renderTargetID.setValue("combine");
        _gr.p_renderMode.selectById("triangles");
        _renderTargetID.setValue("combine");

        _geometryReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/vis/sampledata/left_ventricle_mesh.vtk");
        _geometryReader.p_targetImageID.setValue("reader.output");
        _geometryReader.p_targetImageID.addSharedProperty(&_gr.p_geometryID);

        MeshGeometry* cube = GeometryDataFactory::createCube(tgt::Bounds(tgt::vec3(0.f), tgt::vec3(1.f)), tgt::Bounds(tgt::vec3(0.f), tgt::vec3(1.f)));
        getDataContainer().addData("reader.output", cube);
        _trackballEH->reinitializeCamera(cube);
    }

    void GeometryRendererDemo::deinit() {
        _geometryReader.s_validated.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void GeometryRendererDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_geometryReader) {
            // update camera
            ScopedTypedData<IHasWorldBounds> img(*_data, _geometryReader.p_targetImageID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }


}