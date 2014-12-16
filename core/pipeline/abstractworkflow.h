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

#ifndef ABSTRACTWORKFLOW_H__
#define ABSTRACTWORKFLOW_H__


#include "sigslot/sigslot.h"

#include "cgt/logmanager.h"

#include "core/coreapi.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/datastructures/datacontainer.h"

#include <map>
#include <vector>

namespace campvis {

    /**
     * Abstract base class for CAMPVis Workflows.
     * A workflow is a state machine represented by a graph, where each node defines one workflow 
     * stage and each edge one transition from one stage to another. The whole workflow is 
     * furthermore defined by the set of DataContainers and Pipelines active during the different 
     * states.
     * 
     * To implement your own workflow, subclass this class and populate it with your needs. 
     * Implement the pure virtual method getPipelines() accordingly in order to allow the outside 
     * world (i.e. campvis-application) access to the Pipelines, for instance to create and 
     * initialize canvases and stuff. 
     * \b Beware: This way you will transfer ownership of the returned pointers to the caller 
     * (usually the owner of the workflow object). Every owner of a workflow has to make sure to 
     * call these functions and take ownership of those pointers. Furthermore, it has to guarantee 
     * to init the pipelines \b before calling init() on the workfow object, and deinit the them 
     * the other way around.
     * 
     * \note    This is not entirely beautiful design, but since AbstractWorkflow is part of 
     *          campvis-core, it does not know anything about campvis-application and thus can not 
     *          initialize the pipelines itself. Furthermore, CampvisApplication takes ownership 
     *          of all DataContainers and Pipelines it knows, hence we have do this cumbersome 
     *          transfer of ownership. If you have an idea how to make this more beautiful in terms 
     *          of software engeneering, please go ahead!
     */
    class CAMPVIS_CORE_API AbstractWorkflow : public HasPropertyCollection {
    public:
        struct Stage {
            /// Title of this stage (to be shown in GUI)
            std::string _title;

            /// ID of this stage, used to reference it. I strongly recommend using enums for this!
            int _id;

            /// Possible workflow stages following this stage
            std::vector<Stage*> _possibleTransitions;

            /// Visibilities of the pipeline's canvases
            std::vector< std::pair<AbstractPipeline*, bool> > _pipelineCanvasVisibilities;

            /// Visible properties for this stage
            std::vector<AbstractProperty*> _visibleProperties;
        };


        /**
         * Default constructor
         */
        AbstractWorkflow(const std::string& title);

        /**
         * Default virtual destructor.
         */
        virtual ~AbstractWorkflow();

        /**
         * Returns a list of all Pipelines, which are used by this workflow and should appear 
         * in campvis-application (and thus need to be initialized). This method is to be called 
         * once by the owner of the workflow object. Implement this method to your needs when 
         * subclassing AbstractWorkflow.
         * 
         * \note    Be aware that the caller will take ownership of all pointers in the returned vector!
         * 
         * \return  A vector of all Pipelines used in this workflow, caller will take ownership of the pointers.
         */
        virtual std::vector<AbstractPipeline*> getPipelines() = 0;
                
        /**
         * Gets the name of this very workflow. To be defined by every subclass.
         * \return  The name of this workflow.
         */
        virtual const std::string getName() const = 0;

        /**
         * This method gets called by the owner of the workflow object after it has initialized
         * all the pipelines. You may overload this method in your subclass and add your own
         * init code.
         * 
         * The default implementation will only set the initial stage to the one with the lowest ID.
         */
        virtual void init();
        
        /**
         * This method gets called by the owner of the workflow object before deinitializing all 
         * the pipelines. You may overload this method in your subclass to your needs.
         * 
         * The default implementation does nothing.
         */
        virtual void deinit();

        /**
         * Returns the DataContainer of this workflow.
         * 
         * \note    Be aware that the caller will take ownership of this pointer!
         * \return  _dataContainer
         */
        DataContainer* getDataContainer();

        /**
         * Performs an additional check, whether the stage with ID \a stage is currently available.
         * This check is applied to all possible transitions and allows to implement custom checks
         * that depend on external conditions (e.g. certain data present).
         * The default implementation always returns true. You may override this method in your 
         * custom workflow to your needs.
         * \param   stage   ID of stage to check
         * \return  True if the stage with the given ID is currently available.
         */
        virtual bool isStageAvailable(int stage) const;


        /**
         * Returns the current workflow stage ID.
         * \return  _currentStage
         */
        int getCurrentStageId() const;

        /**
         * Returns a const reference to the current workflow stage.
         * \return  _stages[_currentStage]
         */
        const Stage& getCurrentStage() const;

        /**
         * Returns a const reference to the workflow stage with the given id.
         * \param   id  Id of the workflow stage to return
         * \return  _stages[id]
         */
        const Stage& getStage(int id) const;

        /**
         * Sets the current workflow stage to the one with the given ID.
         * This method does \b not do any checks whether this transition is actually possible.
         * 
         * \note    This method is not intended to be overridden by subclasses. If you want to
         *          add custom behavior, connect to the corresponding signal.
         * \param   stage   ID of the new workflow stage to set.
         */
        void setCurrentStage(int stage);



        /// Signal emitted when the current stage has changed, passes the IDs of the former and the new workflow stage.
        sigslot::signal2<int, int> s_stageChanged;

        /// Signal emitted each time the availability of any stage has changed.
        /// \note   You should emit this signal according to your overload of isStageAvailable().
        sigslot::signal0 s_stageAvailabilityChanged;

    protected:
        /**
         * Registers a new workflow stage with the given ID, title and optionally visible parameters.
         * \note    You should not call this method somewhere else than in the constructor.
         * \param   id                          Integer ID of the new stage. I \b strongly recommend to use enums for this.
         * \param   title                       Title of the stage to display in the GUI.
         * \param   pipelineCanvasVisibilities  Visibilities of the pipeline's canvases.
         * \param   visibleProperties           Vector of visible properties, when this stage is active. Defaults to an empty list.
         */
        void addStage(int id, const std::string& title, const std::vector< std::pair<AbstractPipeline*, bool> >& pipelineCanvasVisibilities, const std::vector<AbstractProperty*>& visibleProperties = std::vector<AbstractProperty*>());

        /**
         * Registers a new transition between the two given workflow stages.
         * \param   from    The ID of the start stage of the transition, must already be registered.
         * \param   to      The ID of the end stage of the transition, must already be registered.
         */
        void addStageTransition(int from, int to);

        DataContainer* _dataContainer;      ///< DataContainer of this workflow. BEWARE: You do not own this pointer!
        int _currentStage;                  ///< currently active workflow stage
        std::map<int, Stage*> _stages;      ///< Map of all workflow stages by ID

    private:

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTWORKFLOW_H__
