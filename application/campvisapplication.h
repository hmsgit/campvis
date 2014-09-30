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

#ifndef CAMPVISAPPLICATION_H__
#define CAMPVISAPPLICATION_H__

//#include <vld.h> // Uncomment this for using Visual Leak Detector

#include "sigslot/sigslot.h"
#include <QApplication>
#include <QDockWidget>
#include <utility>
#include <vector>

#include "core/datastructures/datacontainer.h"


#ifdef CAMPVIS_HAS_SCRIPTING
#include "scripting/glue/luavmstate.h"
#endif

namespace tgt {
    class GLCanvas;
    class QtThreadedCanvas;
    class Texture;
}

namespace campvis {
    class AbstractPipeline;
    class MainWindow;
    class CampVisPainter;
    class MdiDockableWindow;
    class LuaVmState;

    /**
     * The CampVisApplication class wraps Pipelines, Evaluators and Painters all together and takes
     * care about correctly handling all those instances.
     * 
     * Intended usage is:
     *  1. Create your CampVisApplication
     *  2. Add Pipelines and Visualizations as needed
     *  3. call init()
     *  4. call run()
     *  5. call deinit()
     *  6. You can now safely destroy your CampVisApplication
     */
    class CampVisApplication : public QApplication {
    friend class MainWindow;

    public:
        /// Record storing a pipeline together with its painter and Lua VM state.
        struct PipelineRecord {
            AbstractPipeline* _pipeline;
            CampVisPainter* _painter;
        };

        /**
         * Creates a new CampVisApplication.
         * \param   argc        number of passed arguments
         * \param   argv        vector of arguments
         */
        CampVisApplication(int& argc, char** argv);

        /**
         * Destructor, make sure to call deinit() first.
         */
        ~CampVisApplication();

        /**
         * Initializes tgt, OpenGL, and all pipelines, evaluators and painters.
         * Make sure to have everything setup before calling init().
         */
        void init();

        /**
         * Deinitializes tgt, OpenGL, and all pipelines, evaluators and painters.
         */
        void deinit();

        /**
         * Adds a pipeline to this CAMPVis application.
         * Each pipeline will automatically get its own OpenGL context, the corresponding CampvisPainter
         * and all necessary connections will be created.
         * 
         * \param   name    Name of the OpenGL context to create for the pipeline.
         * \param   vp      AbstractPipeline to add.
         */
        void addPipeline(const std::string& name, AbstractPipeline* pipeline);

        /**
         * Adds a dock widget to the main window.
         * The dock widget will be automatically docked in the specified area.
         *
         * \param   area    Area of the main window to which the dock widget should be added.
         * \param   dock    The dock widget to add to the main window.
         */
        void registerDockWidget(Qt::DockWidgetArea area, QDockWidget* dock);

        /**
         * Runs the actual application.
         * Make sure to call init() before.
         * \return  0 on success.
         */
        int run();


        /**
         * Creates a new DataContainer with the given name.
         * \param   name    Name of the new DataContainer
         * \return  The newly created DataContainer
         */
        DataContainer* createAndAddDataContainer(const std::string& name);

        /**
         * Reloads all GLSL shaders from file and rebuilds them.
         */
        void rebuildAllShadersFromFiles();


#ifdef CAMPVIS_HAS_SCRIPTING
        /**
         * Returns the global LuaVmState of this application.
         */
        LuaVmState* getLuaVmState();
#endif


        /// Signal emitted when the collection of pipelines has changed.
        sigslot::signal0 s_PipelinesChanged;

        /// Signal emitted when the collection of DataContainers has changed.
        sigslot::signal0 s_DataContainersChanged;

    private:
        void initGlContextAndPipeline(tgt::GLCanvas* canvas, AbstractPipeline* pipeline);


        /// All pipelines 
        std::vector<PipelineRecord> _pipelines;

        std::map<AbstractPipeline *, MdiDockableWindow *> _pipelineWindows;

        /// All DataContainers
        std::vector<DataContainer*> _dataContainers;

        /**
         * Triggers the ShaderManager to rebuild all shaders from file and then 
         * invalidates all VisualizationProcessors.
         */
        void triggerShaderRebuild();

        /// A local OpenGL context used for initialization
        tgt::GLCanvas* _localContext;
        /// Main window hosting GUI stuff
        MainWindow* _mainWindow;

        /// Error texture to show if there is no output found
        tgt::Texture* _errorTexture;

        /// the global LuaVmState of this application
        LuaVmState* _luaVmState;

        /// Flag, whether CampVisApplication was correctly initialized
        bool _initialized;

        int _argc;
        char** _argv;

        static const std::string loggerCat_;
    };
}

#endif // CAMPVISAPPLICATION_H__
