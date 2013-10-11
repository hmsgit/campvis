// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
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

namespace tgt {
    class GLCanvas;
    class QtThreadedCanvas;
}

namespace campvis {
    class AbstractPipeline;
    class MainWindow;
    class CampVisPainter;

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
        /**
         * Creates a new CampVisApplication.
         * \param   argc        number of passed arguments
         * \param   argv        vector of arguments
         * \param   useOpenCL   Flag, whether to use OpenCL (inits and deinits KissCL lib at the appropriate time).
         */
        CampVisApplication(int& argc, char** argv, bool useOpenCL = true);

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

        /// Signal emitted when the collection of pipelines has changed.
        sigslot::signal0<> s_PipelinesChanged;

        /// Signal emitted when the collection of DataContainers has changed.
        sigslot::signal0<> s_DataContainersChanged;

    private:
        void addPipelineImpl(tgt::QtThreadedCanvas* canvas, const std::string& name, AbstractPipeline* pipeline);

        /// All pipelines 
        std::vector<AbstractPipeline*> _pipelines;
        /// All visualisations (i.e. Pipelines with their corresponding painters/canvases)
        std::vector< std::pair<AbstractPipeline*, CampVisPainter*> > _visualizations;

        /// All DataContainers
        std::vector<DataContainer*> _dataContainers;

        /// A local OpenGL context used for initialization
        tgt::GLCanvas* _localContext;
        /// Main window hosting GUI stuff
        MainWindow* _mainWindow;

        /// Flag, whether CampVisApplication was correctly initialized
        bool _initialized;

        /// Flag, whether to use OpenCL (inits and deinits KissCL lib at the appropriate time).
        bool _useOpenCL;

        int _argc;
        char** _argv;

        static const std::string loggerCat_;
    };
}

#endif // CAMPVISAPPLICATION_H__
