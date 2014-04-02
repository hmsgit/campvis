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

#include "tgt/texturereadertga.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/datastructures/geometrydatafactory.h"

namespace campvis {

    GeometryRendererDemo::GeometryRendererDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _geometryReader()
        , _lvRenderer(&_canvasSize)
        , _teapotRenderer(&_canvasSize)
        , _cubeRenderer(&_canvasSize)
        , _compositor1(&_canvasSize)
        , _compositor2(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_geometryReader);
        addProcessor(&_teapotRenderer);
        addProcessor(&_lvRenderer);
        addProcessor(&_cubeRenderer);
        addProcessor(&_compositor1);
        addProcessor(&_compositor2);
    }

    GeometryRendererDemo::~GeometryRendererDemo() {
        _geometryReader.s_validated.disconnect(this);
        delete _trackballEH;
    }

    void GeometryRendererDemo::init() {
        AutoEvaluationPipeline::init();

        // load textureData from file
        tgt::TextureReaderTga trt;
        tgt::Texture* campTexture = trt.loadTexture(CAMPVIS_SOURCE_DIR "/modules/vis/sampledata/camplogo.tga", tgt::Texture::LINEAR);
        ImageData* textureData = new ImageData(2, campTexture->getDimensions(), campTexture->getNumChannels());
        ImageRepresentationGL::create(textureData, campTexture);
        getDataContainer().addData("CampTexture", textureData);

        // connect slots
        _geometryReader.s_validated.connect(this, &GeometryRendererDemo::onProcessorValidated);

        // create Teapot
        MultiIndexedGeometry* teapot = GeometryDataFactory::createTeapot();
        teapot->applyTransformationToVertices(tgt::mat4::createTranslation(tgt::vec3(5.f, 10.f, 5.f)) * tgt::mat4::createScale(tgt::vec3(16.f)));
        getDataContainer().addData("teapot", teapot);

        // create cube
        MeshGeometry* cube = GeometryDataFactory::createCube(tgt::Bounds(tgt::vec3(7.f), tgt::vec3(9.f)), tgt::Bounds(tgt::vec3(0.f), tgt::vec3(1.f)));
        getDataContainer().addData("cube", cube);

        // setup pipeline
        _camera.addSharedProperty(&_lvRenderer.p_camera);
        _camera.addSharedProperty(&_teapotRenderer.p_camera);
        _camera.addSharedProperty(&_cubeRenderer.p_camera);

        _geometryReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/vis/sampledata/left_ventricle_mesh.vtk");
        _geometryReader.p_targetImageID.setValue("reader.output");

        _lvRenderer.p_geometryID.setValue("reader.output");
        _lvRenderer.p_renderTargetID.setValue("lv.render");
        _lvRenderer.p_renderMode.selectById("triangles");
        _lvRenderer.p_solidColor.setValue(tgt::vec4(0.8f, 0.f, 0.f, .9f));

        _teapotRenderer.p_geometryID.setValue("teapot");
        _teapotRenderer.p_renderTargetID.setValue("teapot.render");
        _teapotRenderer.p_renderMode.selectById("trianglestrip");
        _teapotRenderer.p_showWireframe.setValue(false);
        _teapotRenderer.p_solidColor.setValue(tgt::vec4(1.f, 0.5f, 0.f, 1.f));
    
        _cubeRenderer.p_geometryID.setValue("cube");
        _cubeRenderer.p_renderTargetID.setValue("cube.render");
        _cubeRenderer.p_renderMode.selectById("polygon");
        _cubeRenderer.p_showWireframe.setValue(false);
        _cubeRenderer.p_textureID.setValue("CampTexture");
        _cubeRenderer.p_coloringMode.selectByOption(GeometryRenderer::TEXTURE_COLOR);

        _compositor1.p_firstImageId.setValue("lv.render");
        _compositor1.p_secondImageId.setValue("teapot.render");
        _compositor1.p_compositingMethod.selectById("depth");
        _compositor1.p_targetImageId.setValue("combine1");
        _compositor1.p_enableBackground.setValue(false);

        _compositor2.p_firstImageId.setValue("combine1");
        _compositor2.p_secondImageId.setValue("cube.render");
        _compositor2.p_compositingMethod.selectById("depth");
        _compositor2.p_targetImageId.setValue("combine2");

        _renderTargetID.setValue("combine2");
    }

    void GeometryRendererDemo::deinit() {
        _geometryReader.s_validated.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void GeometryRendererDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_geometryReader) {
            // update camera
            ScopedTypedData<IHasWorldBounds> lv(*_data, _geometryReader.p_targetImageID.getValue());
            ScopedTypedData<IHasWorldBounds> teapot(*_data, "teapot");
            ScopedTypedData<IHasWorldBounds> cube(*_data, "cube");
            if (lv != 0 && teapot != 0) {
                tgt::Bounds unionBounds;
                unionBounds.addVolume(lv->getWorldBounds());
                unionBounds.addVolume(teapot->getWorldBounds());
                unionBounds.addVolume(cube->getWorldBounds());

                _trackballEH->reinitializeCamera(unionBounds);
            }
        }
    }


}
