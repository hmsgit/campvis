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

#include "demoworkflow.h"


namespace campvis {
namespace workflowdemo {


    DemoWorkflow::DemoWorkflow() 
        : AbstractWorkflow("DemoWorkflow")
    {
        _loaderPipeline = new ImageLoading(*_dataContainer);
        _filterPipeline = new ImageFiltering(*_dataContainer);
        _visPipeline = new ImageVis(*_dataContainer);

        typedef std::vector< std::pair<AbstractPipeline*, bool> > VisVec;
        typedef std::vector<AbstractProperty*> PropVec;

        VisVec vvl;
        vvl.push_back(std::make_pair(_loaderPipeline, false));
        vvl.push_back(std::make_pair(_filterPipeline, false));
        vvl.push_back(std::make_pair(_visPipeline, false));
        PropVec pvl;
        pvl.push_back(&_loaderPipeline->_imageReader.p_url);
        addStage(LOADING_STAGE, "Load Image", vvl, pvl);

        VisVec vvf;
        vvf.push_back(std::make_pair(_loaderPipeline, false));
        vvf.push_back(std::make_pair(_filterPipeline, true));
        vvf.push_back(std::make_pair(_visPipeline, false));
        PropVec pvf;
        pvf.push_back(&_filterPipeline->_glCrop.p_llf);
        pvf.push_back(&_filterPipeline->_glCrop.p_urb);
        addStage(FILTERING_STAGE, "Filter Image", vvf, pvf);

        VisVec vvv;
        vvv.push_back(std::make_pair(_loaderPipeline, false));
        vvv.push_back(std::make_pair(_filterPipeline, false));
        vvv.push_back(std::make_pair(_visPipeline, true));
        PropVec pvv;
        pvv.push_back(_visPipeline->_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::KappaS"));
        pvv.push_back(_visPipeline->_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::KappaT"));
        addStage(VIS_STAGE, "Context-preserving Visualization", vvv, pvv);

        addStageTransition(LOADING_STAGE, FILTERING_STAGE);
        addStageTransition(FILTERING_STAGE, VIS_STAGE);
    }

    DemoWorkflow::~DemoWorkflow() {

    }

    std::vector<AbstractPipeline*> DemoWorkflow::getPipelines() {
        std::vector<AbstractPipeline*> toReturn;
        toReturn.push_back(_loaderPipeline);
        toReturn.push_back(_filterPipeline);
        toReturn.push_back(_visPipeline);
        return toReturn;
    }

    void DemoWorkflow::init() {
        _dataContainer->s_dataAdded.connect(this, &DemoWorkflow::onDataContainerDataAdded);
        this->s_stageChanged.connect(this, &DemoWorkflow::onStageChanged);

        // nothing special to do here apart from calling base method
        AbstractWorkflow::init();
    }

    void DemoWorkflow::deinit() {
        _dataContainer->s_dataAdded.disconnect(this);
        this->s_stageChanged.disconnect(this);
    }

    bool DemoWorkflow::isStageAvailable(int stage) const {
        switch (stage) {
            case LOADING_STAGE:
                return true;
            case FILTERING_STAGE:
                return _dataContainer->hasData("image.original");
            case VIS_STAGE:
                return _dataContainer->hasData("image.cropped");
            default:
                return false;
        }
    }

    void DemoWorkflow::onDataContainerDataAdded(std::string key, DataHandle) {
        if (key == "image.original" || key == "image.cropped")
            s_stageAvailabilityChanged.emitSignal();
    }

    void DemoWorkflow::onStageChanged(int from, int to) {
        switch (to) {
            case LOADING_STAGE:
                _loaderPipeline->setEnabled(true);
                _filterPipeline->setEnabled(false);
                _visPipeline->setEnabled(false);
                break;
            case FILTERING_STAGE:
                _loaderPipeline->setEnabled(false);
                _filterPipeline->setEnabled(true);
                _visPipeline->setEnabled(false);
                break;
            case VIS_STAGE:
                _loaderPipeline->setEnabled(false);
                _filterPipeline->setEnabled(false);
                _visPipeline->setEnabled(true);
                break;
        }

    }

}
}
