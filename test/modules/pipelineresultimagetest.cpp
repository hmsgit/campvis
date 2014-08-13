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

#include "gtest/gtest.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL

#include "tgt/filesystem.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/stringutils.h"

#include "modules/pipelinefactory.h"
#include "modules/devil/processors/devilimagewriter.h"

using namespace campvis;


/**
 * Test class for StringUtils.
 */

class PipelineWriteResultImageTest : public ::testing::Test {
protected:
    PipelineWriteResultImageTest() 
        : _dataContainer("Test Container")
        , _pipeline(nullptr)
        , _wroteFile(false)
    {
        _basePath = "visregtests/";
        if ( ! tgt::FileSystem::dirExists(_basePath)) {
            tgt::FileSystem::createDirectory(_basePath);
        }
        _basePath = "visregtests/testruns/";
        if ( ! tgt::FileSystem::dirExists(_basePath)) {
            tgt::FileSystem::createDirectory(_basePath);
        }
        std::vector<std::string> filelist = tgt::FileSystem::listSubDirectories(_basePath, true);
        std::string testRunNo = "";
        std::string caseNo = "1/";
        if (_prevNoCases++ == 0) {
            testRunNo = filelist.size() > 0 ? StringUtils::toString(StringUtils::fromString<int>(filelist[filelist.size()-1])+1) : "1";
            tgt::FileSystem::createDirectoryRecursive(_basePath+testRunNo+"/"+ caseNo);
        }
        else 
            testRunNo = filelist[filelist.size()-1];
        
        _basePath += testRunNo+"/"+caseNo;
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
        _pipeline = PipelineFactory::getRef().createPipeline(_pipelineName, &_dataContainer);

        if (_pipeline != nullptr) {
            // setup pipeline
            _pipeline->setCanvas(GLJobProc.iKnowWhatImDoingGetArbitraryContext());
            _pipeline->init();
            _pipeline->setEnabled(true);
            _pipeline->setRenderTargetSize(tgt::ivec2(1024, 1024));
        }
    }

    void execute() {
        if (_pipeline != nullptr) {
            // invalidate each processor
            std::vector<AbstractProcessor*> processors = _pipeline->getProcessors();
            for (size_t i = 0; i < processors.size(); ++i) {
                processors[i]->invalidate(AbstractProcessor::INVALID_RESULT);
            }

            // execute each processor (we do this n*n times, as we might have a complex dependency graph)
            for (size_t i = 0; i < processors.size(); ++i) {
                for (size_t i = 0; i < processors.size(); ++i) {
                    processors[i]->process(_dataContainer);
                }
            }

            // write result image
            _imageWriter.p_inputImage.setValue(_pipeline->getRenderTargetID());
            _imageWriter.p_url.setValue(_fileName);
            _imageWriter.process(_dataContainer);

            _wroteFile = tgt::FileSystem::fileExists(_fileName);
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
    _fileName = _basePath +"volumeexplorerdemo.png";
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
    _fileName = _basePath +"slicevis.png";
    init();
    execute();
    EXPECT_TRUE(_wroteFile);
}

#endif
