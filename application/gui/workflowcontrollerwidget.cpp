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

#include "workflowcontrollerwidget.h"

#include "cgt/assert.h"
#include "application/qtjobprocessor.h"

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalMapper>
#include <QVBoxLayout>

namespace campvis {

    const std::string WorkflowControllerWidget::loggerCat_ = "CAMPVis.application.WorkflowControllerWidget";


    WorkflowControllerWidget::WorkflowControllerWidget(QWidget* parent /*= nullptr*/)
        : QWidget(parent)
        , _workflow(nullptr)
        , _isBackWardsStep(false)
        , _signalMapper(nullptr)
        , _lblWorkflowStage(nullptr)
        , _propertyCollectionWidget(nullptr)
        , _btnPrevStage(nullptr)
        , _layoutNextStages(nullptr)
    {
        setupGUI();
    }

    WorkflowControllerWidget::~WorkflowControllerWidget() {
        clearNextStagesLayout();
    }

    void WorkflowControllerWidget::init() {
        if (_workflow != nullptr)
            _workflow->init();
    }

    void WorkflowControllerWidget::deinit() {
        if (_workflow != nullptr)
            _workflow->deinit();
    }

    void WorkflowControllerWidget::setWorkflow(AbstractWorkflow* workflow) {
        if (_workflow != nullptr) {
            _workflow->s_stageChanged.disconnect(this);
            _workflow->s_stageAvailabilityChanged.disconnect(this);
        }

        _workflow = workflow;
        _stageHistory.clear();

        QtJobProc.enqueueJob([&]() {
            if (_workflow != nullptr) {
                // FIXME: Taking the first of the DataContainers here is not really beautiful!
                _propertyCollectionWidget->updatePropCollection(_workflow, _workflow->getDataContainers().front());

                _workflow->s_stageChanged.connect(this, &WorkflowControllerWidget::onStageChanged);
                _workflow->s_stageAvailabilityChanged.connect(this, &WorkflowControllerWidget::onStageAvailabilityChanged);
            }
            else {
                _propertyCollectionWidget->updatePropCollection(nullptr, nullptr);
                _lblWorkflowStage->setText(tr("No Workflow Loaded."));
            }
        });
    }

    void WorkflowControllerWidget::onPrevButtonClicked() {
        if (!_stageHistory.empty()) {
            int previousStage = _stageHistory.back();
            _stageHistory.pop_back();
            _isBackWardsStep = true;
            _workflow->setCurrentStage(previousStage);
        }
    }

    void WorkflowControllerWidget::onStageSelectionButtonClicked(int id) {
        _workflow->setCurrentStage(id);
    }

    void WorkflowControllerWidget::setupGUI() {
        QGridLayout* layout = new QGridLayout(this);

        _lblWorkflowStage = new QLabel(tr("Current Stage: n/a"), this);
        layout->addWidget(_lblWorkflowStage, 0, 0, 1, 2);

        QScrollArea* sa = new QScrollArea(this);
        sa->setWidgetResizable(true);
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setFrameStyle(QScrollArea::NoFrame);

        _propertyCollectionWidget = new PropertyCollectionWidget(this);
        sa->setWidget(_propertyCollectionWidget);
        layout->addWidget(sa, 1, 0, 1, 2);

        _btnPrevStage = new QPushButton("<< n/a");
        _btnPrevStage->setEnabled(false);
        layout->addWidget(_btnPrevStage, 2, 0, 1, 1);

        _layoutNextStages = new QVBoxLayout();
        layout->addLayout(_layoutNextStages, 2, 1, 1, 1);

        _signalMapper = new QSignalMapper(this);
        populateNextStagesLayout();

        connect(_btnPrevStage, SIGNAL(clicked()), this, SLOT(onPrevButtonClicked()));
        connect(_signalMapper, SIGNAL(mapped(int)), this, SLOT(onStageSelectionButtonClicked(int)));
    }

    void WorkflowControllerWidget::clearNextStagesLayout() {
        for (auto it = _nextButtons.begin(); it != _nextButtons.end(); ++it) {
            _layoutNextStages->removeWidget(it->first);
            delete it->first;
        }
        _nextButtons.clear();
    }

    void WorkflowControllerWidget::populateNextStagesLayout() {
        // first, remove all buttons from the layout
        clearNextStagesLayout();

        // now, populate with buttons for the current stage
        if (_workflow != nullptr) {
            const AbstractWorkflow::Stage& s = _workflow->getCurrentStage();
            for (auto it = s._possibleTransitions.begin(); it != s._possibleTransitions.end(); ++it) {
                QPushButton* theButton = new QPushButton(QString::fromStdString((*it)->_title + " >>"));
                _layoutNextStages->addWidget(theButton);
                _nextButtons.push_back(std::make_pair(theButton, (*it)->_id));

                _signalMapper->setMapping(theButton, (*it)->_id);
                connect(theButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
            }

            onStageAvailabilityChanged();
        }
    }

    void WorkflowControllerWidget::onStageChanged(int previousStage, int currentStage) {
        if (!_isBackWardsStep)
            _stageHistory.push_back(previousStage);
        _isBackWardsStep = false;

        QtJobProc.enqueueJob([=]() {
            populateNextStagesLayout();

            const AbstractWorkflow::Stage& s = _workflow->getCurrentStage();
            _lblWorkflowStage->setText(QString::fromStdString(s._title));
        });

    }

    void WorkflowControllerWidget::onStageAvailabilityChanged() {
        for (auto it = _nextButtons.begin(); it != _nextButtons.end(); ++it) {
            int theStage = it->second;
            it->first->setEnabled(_workflow->isStageAvailable(theStage));
        }
    }

}
