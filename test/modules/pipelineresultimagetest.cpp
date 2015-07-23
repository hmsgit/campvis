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

#include "gtest/gtest.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL

#include "cgt/filesystem.h"
#include "cgt/opengljobprocessor.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/pipelinefactory.h"
#include "core/properties/allproperties.h"
#include "core/tools/stringutils.h"

#include "modules/devil/processors/devilimagewriter.h"
#include "modules/base/processors/trackballcameraprovider.h"

using namespace campvis;


/**
 * Test class for creating test images for visual regression test
 */

class PipelineWriteResultImageTest : public ::testing::Test {
protected:
    PipelineWriteResultImageTest() 
        : _dataContainer("Test Container")
        , _pipeline(nullptr)
        , _wroteFile(false)
    {
        _basePath = "visregtests/";
        if ( ! cgt::FileSystem::dirExists(_basePath)) {
            cgt::FileSystem::createDirectory(_basePath);
        }
        _basePath = "visregtests/testruns/";
        if ( ! cgt::FileSystem::dirExists(_basePath)) {
            cgt::FileSystem::createDirectory(_basePath);
        }
        std::vector<std::string> filelist = cgt::FileSystem::listSubDirectories(_basePath, true);
        std::string caseNo = "1/";
        cgt::FileSystem::createDirectoryRecursive(_basePath+ caseNo);
        _basePath += caseNo;
    }

    ~PipelineWriteResultImageTest() {
        delete _pipeline;
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
        if (_pipeline != nullptr) {
            _pipeline->setEnabled(false);
            _pipeline->deinit();
        }
    }

    void init() {
        // create pipeline
        _pipeline = PipelineFactory::getRef().createPipeline(_pipelineName, _dataContainer);

        if (_pipeline != nullptr) {
            // setup pipeline
            _pipeline->setCanvas(GLJobProc.getContext());
            _pipeline->init();
            _pipeline->setEnabled(true);
            _pipeline->setRenderTargetSize(cgt::ivec2(1024, 1024));

            // invalidate each processor
            std::vector<AbstractProcessor*> processors = _pipeline->getProcessors();
            for (size_t i = 0; i < processors.size(); ++i) {
                processors[i]->invalidate(AbstractProcessor::INVALID_RESULT);
            }
        }
        else {
            FAIL() << "Could not instantiate pipeline '" << _pipelineName << "'";
        }
    }

    void execute() {
        if (_pipeline != nullptr) {
            for (size_t i = 0; i < _pipeline->getProcessors().size(); ++i)
                _pipeline->executePipeline();

            // write result image
            _imageWriter.p_inputImage.setValue(_pipeline->getRenderTargetID());
            _imageWriter.p_url.setValue(_fileName);
            _imageWriter.invalidate(AbstractProcessor::INVALID_RESULT);
            _imageWriter.process(_dataContainer);

            _wroteFile = cgt::FileSystem::fileExists(_fileName);
        }
    }

protected:
    std::string _pipelineName;
    std::string _fileName;
    std::string _basePath;
    static int _prevNoCases;

    DataContainer _dataContainer;
    AbstractPipeline* _pipeline;
    DevilImageWriter _imageWriter;

    bool _wroteFile;
};
int PipelineWriteResultImageTest::_prevNoCases = 0;

TEST_F(PipelineWriteResultImageTest, VolumeExplorerDemo) {
    _pipelineName = "VolumeExplorerDemo";
    _fileName = _basePath + "volumeexplorerdemo.png";
    init();
    execute();
    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, GeometryRendererDemo) {
    _pipelineName = "GeometryRendererDemo";
    _fileName = _basePath +"geometryrendererdemo.png";
    init();
    execute();
    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, SliceVis) {
    _pipelineName = "SliceVis";
    _fileName = _basePath + "slicevis.png";
    init();
    execute();
    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, AdvancedUsVis) {
    _pipelineName = "AdvancedUsVis";
    _fileName = _basePath + "advancedusvis.png";
    init();
    execute();
    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, VectorFieldDemo) {
    _pipelineName = "VectorFieldDemo";
    _fileName = _basePath + "vectorfielddemo.png";
    init();
    execute();

    AbstractProperty* p = _pipeline->getProperty("SliceNuber");
    if (IntProperty* tester = dynamic_cast<IntProperty*>(p)) {
    	tester->setValue(64);
    }

    execute();
    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, DVRVis) {
    _pipelineName = "DVRVis";
    _fileName = _basePath + "dvrvis.png";
    init();
    execute();

    auto properties = _pipeline->getProcessors();
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        if (TrackballCameraProvider* tcp = dynamic_cast<TrackballCameraProvider*>(*it)) {
            tcp->p_position.setValue(cgt::vec3(417.f, 44.5f, -112.5f));
            tcp->p_focus.setValue(cgt::vec3(91.f, 91.f, 80.f));
            tcp->p_upVector.setValue(cgt::vec3(-0.487f, 0.142f, -0.861f));
        }
    }
    execute();

    EXPECT_TRUE(_wroteFile);
}

TEST_F(PipelineWriteResultImageTest, TensorDemo) {
    _pipelineName = "TensorDemo";
    _fileName = _basePath + "tensordemo.png";
    init();
    execute();

    AbstractProperty* p = _pipeline->getProperty("SliceNuber");
    if (IntProperty* tester = dynamic_cast<IntProperty*>(p)) {
        tester->setValue(6);
    }
    execute();

    EXPECT_TRUE(_wroteFile);
}

#endif

