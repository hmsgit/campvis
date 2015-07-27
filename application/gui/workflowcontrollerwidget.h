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

#ifndef WORKFLOWCONTROLLERWIDGET_H__
#define WORKFLOWCONTROLLERWIDGET_H__

#include "core/pipeline/abstractworkflow.h"

#include "application/applicationapi.h"
#include "application/gui/properties/propertycollectionwidget.h"

#include <QLabel>
#include <QWidget>

#include <list>
#include <utility>
#include <vector>

class QPushButton;
class QSignalMapper;
class QVBoxLayout;

namespace campvis {
    class CampVisApplication;

    /**
     * This widget is used to offer a user interface to an \a AbstractWorkflow instance.
     * 
     * It has two buttons to navigate through the workflow and displays the properties
     * of the \a AbstractWorkflow using a \a PropertyCollectionWidget.
     */
    class CAMPVIS_APPLICATION_API WorkflowControllerWidget : public QWidget, public sigslot::has_slots {
        Q_OBJECT;

    public:

        /**
         * Creates a new WorkflowControllerWidget.
         * \param   application     Pointer to the CampvisApplication owning this widget, must not be 0.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit WorkflowControllerWidget(CampVisApplication* application, QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~WorkflowControllerWidget();

        /// dummy. may be useful at some point
        void init();

        /// dummy. may be useful at some point
        void deinit();

        QSize sizeHint() const;

        /**
         * Sets the AbstractWorkflow that should be handled by this widget.
         * Sets up the signal/slot connections.
         */
        void setWorkflow(AbstractWorkflow* workflow);
    
    public slots:
        void onPrevButtonClicked();
        void onStageSelectionButtonClicked(int id);

    private:
        /// Setup the the workflow controller's GUI.
        void setupGUI();

        void clearNextStagesLayout();
        void populateNextStagesLayout();

        /// connects to \a AbstractWorkflow::s_stageChanged signal
        void onStageChanged(int previousStage, int currentStage);

        /// connects to \a AbstractWorkflow::s_stageAvailabilityChanged signal
        void onStageAvailabilityChanged();

        CampVisApplication* _application;       ///< Pointer to the CampvisApplication owning this widget, must not be 0.
        AbstractWorkflow* _workflow;            ///< the currently associated \a WorkflowController
        std::list<int> _stageHistory;           ///< History of the visited stages, used to provide "go back" functionality
        bool _isBackWardsStep;                  ///< Flag whether current stage selection is a backwards step (then don't add previous stage to history)

        /// All existing buttons to move to next stage paired with the stage they advance to.
        std::vector< std::pair<QPushButton*, int> > _nextButtons;   

        QSignalMapper* _signalMapper;
        QLabel* _lblWorkflowStage;
        PropertyCollectionWidget* _propertyCollectionWidget;
        QPushButton* _btnPrevStage;
        QVBoxLayout* _layoutNextStages;

        static const std::string loggerCat_;
    };
}

#endif // WORKFLOWCONTROLLERWIDGET_H__
