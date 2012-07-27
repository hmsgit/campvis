#ifndef TUMVISAPPLICATION_H__
#define TUMVISAPPLICATION_H__

#include "sigslot/sigslot.h"
#include <QApplication>
#include <utility>
#include <vector>

namespace tgt {
    class QtThreadedCanvas;
}

namespace TUMVis {
    class AbstractPipeline;
    class MainWindow;
    class PipelineEvaluator;
    class TumVisPainter;
    class VisualizationPipeline;

    /**
     * The TumVisApplication class wraps Pipelines, Evaluators and Painters all together and takes
     * care about correctly handling all those instances.
     * 
     * Intended usage is:
     *  1) Create your TumVisApplication
     *  2) Add Pipelines and Visualizations as needed
     *  3) call init()
     *  4) call run()
     *  5) call deinit()
     *  6) You can now safely destroy your TumVisApplication
     */
    class TumVisApplication : QApplication {
    friend class MainWindow;

    public:
        /**
         * Creates a new TumVisApplication.
         * \param   argc    number of passed arguments
         * \param   argv    vector of arguments
         */
        TumVisApplication(int argc, char** argv);

        /**
         * Destructor, make sure to call deinit() first.
         */
        ~TumVisApplication();

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
         * Adds a pipeline which doesn't need visualization (OpenGL) support.
         * Each pipeline will automatically get its own evaluator.
         * 
         * \note    If you want to add a pipeline that needs a valid OpenGL context, use 
         *          addVisualizationPipeline() instead.
         * \param   pipeline    Pipeline to add, must not need OpenGL support.
         */
        void addPipeline(AbstractPipeline* pipeline);

        /**
         * Adds a visualization pipeline (i.e. a pipeline that needs a OpenGL context).
         * For each added pipeline, two OpenGL contexts will be created (one for the evaluation, 
         * one for the rendering).
         * 
         * \note    You do \emph not need to call addPipeline.
         * \param   name    Name of the OpenGL context to create for the pipeline.
         * \param   vp      VisualizationPipeline to add.
         */
        void addVisualizationPipeline(const std::string& name, VisualizationPipeline* vp);

        /**
         * Runs the actual application.
         * Make sure to call init() before.
         * \return  0 on success.
         */
        int run();

        sigslot::signal0<> s_PipelinesChanged;

    private:
        /// All pipelines (incuding VisualizationPipelines)
        std::vector<AbstractPipeline*> _pipelines;
        /// All pipeline evaluators (separated from _pipelines because we probably want multiple pipelines per evaluator later)
        std::vector<PipelineEvaluator*> _pipelineEvaluators;
        /// All visualisations (i.e. VisualizationPipelines with their corresponding painters/canvases)
        std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> > _visualizations;

        /// A local OpenGL context used for initialization
        tgt::QtThreadedCanvas* _localContext;
        /// Main window hosting GUI stuff
        MainWindow* _mainWindow;

        /// Flag, whether TumVisApplication was correctly initialized
        bool _initialized;

        int _argc;
        char** _argv;

        static const std::string loggerCat_;
    };
}

#endif // TUMVISAPPLICATION_H__
