// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "geometryrendererdemo.h"

#include "cgt/texturereadertga.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/datastructures/geometrydatafactory.h"

namespace campvis {

    GeometryRendererDemo::GeometryRendererDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _tcp(&_canvasSize)
        , _lsp()
        , _geometryReader()
        , _lvRenderer(&_canvasSize)
        , _teapotRenderer(&_canvasSize)
        , _cubeRenderer(&_canvasSize)
        , _compositor1(&_canvasSize)
        , _compositor2(&_canvasSize)
    {
        addEventListenerToBack(&_tcp);

        addProcessor(&_tcp);
        addProcessor(&_lsp);
        addProcessor(&_geometryReader);
        addProcessor(&_teapotRenderer);
        addProcessor(&_lvRenderer);
        addProcessor(&_cubeRenderer);
        addProcessor(&_compositor1);
        addProcessor(&_compositor2);
    }

    GeometryRendererDemo::~GeometryRendererDemo() {
        _geometryReader.s_validated.disconnect(this);
    }

    void GeometryRendererDemo::init() {
        AutoEvaluationPipeline::init();

        // load textureData from file
        cgt::TextureReaderTga trt;
        cgt::Texture* campTexture = trt.loadTexture(ShdrMgr.completePath("/modules/vis/sampledata/camplogo.tga"), cgt::Texture::LINEAR);
        ImageData* textureData = new ImageData(2, campTexture->getDimensions(), campTexture->getNumChannels());
        ImageRepresentationGL::create(textureData, campTexture);
        getDataContainer().addData("CampTexture", textureData);

        // connect slots
        _geometryReader.s_validated.connect(this, &GeometryRendererDemo::onProcessorValidated);

        // create Teapot
        auto teapot = GeometryDataFactory::createTeapot();
        teapot->applyTransformationToVertices(cgt::mat4::createTranslation(cgt::vec3(5.f, 10.f, 5.f)) * cgt::mat4::createScale(cgt::vec3(16.f)));
        getDataContainer().addData("teapot", teapot.release());

        // create cube
        auto cube = GeometryDataFactory::createCube(cgt::Bounds(cgt::vec3(7.f), cgt::vec3(9.f)), cgt::Bounds(cgt::vec3(0.f), cgt::vec3(1.f)));
        getDataContainer().addData("cube", cube.release());

        // setup pipeline
        _geometryReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/left_ventricle_mesh.vtk"));
        _geometryReader.p_targetImageID.setValue("reader.output");

        _lvRenderer.p_geometryID.setValue("reader.output");
        _lvRenderer.p_renderTargetID.setValue("lv.render");
        _lvRenderer.p_renderMode.selectById("triangles");
        _lvRenderer.p_solidColor.setValue(cgt::vec4(0.8f, 0.f, 0.f, .9f));

        _teapotRenderer.p_geometryID.setValue("teapot");
        _teapotRenderer.p_renderTargetID.setValue("teapot.render");
        _teapotRenderer.p_renderMode.selectById("trianglestrip");
        _teapotRenderer.p_showWireframe.setValue(false);
        _teapotRenderer.p_solidColor.setValue(cgt::vec4(1.f, 0.5f, 0.f, 1.f));
    
        _cubeRenderer.p_geometryID.setValue("cube");
        _cubeRenderer.p_renderTargetID.setValue("cube.render");
        _cubeRenderer.p_renderMode.selectById("trianglefan");
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
            ScopedTypedData<IHasWorldBounds> lv(*_dataContainer, _geometryReader.p_targetImageID.getValue());
            ScopedTypedData<IHasWorldBounds> teapot(*_dataContainer, "teapot");
            ScopedTypedData<IHasWorldBounds> cube(*_dataContainer, "cube");
            if (lv != 0 && teapot != 0) {
                cgt::Bounds unionBounds;
                unionBounds.addVolume(lv->getWorldBounds());
                unionBounds.addVolume(teapot->getWorldBounds());
                unionBounds.addVolume(cube->getWorldBounds());

                _tcp.reinitializeCamera(unionBounds);
            }
        }
    }


}
