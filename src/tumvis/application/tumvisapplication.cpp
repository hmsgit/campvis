#include "tumvisapplication.h"

#include "tgt/assert.h"
#include "tgt/exception.h"
#include "tgt/glcontext.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tbb/include/tbb/compat/thread"

#include "application/tumvispainter.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/pipeline/pipelineevaluator.h"

namespace TUMVis {

    TumVisApplication::TumVisApplication(int argc, char** argv) 
        : QApplication(argc, argv)
        , _localContext(0)
        , _initialized(false)
        , _argc(argc)
        , _argv(argv)
    {
        tgt::QtContextManager::init();

    }

    TumVisApplication::~TumVisApplication() {
        tgtAssert(_initialized == false, "Destructing initialized TumVisApplication, deinitialize first!");

        // delete everything in the right order:
        for (std::vector<PipelineEvaluator*>::iterator it = _pipelineEvaluators.begin(); it != _pipelineEvaluators.end(); ++it) {
            delete *it;
        }
        for (std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            delete it->second;
        }
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            delete *it;
        }

    }

    void TumVisApplication::init() {
        tgtAssert(_initialized == false, "Tried to initialize TumVisApplication twice.");

        // Init TGT
        tgt::InitFeature::Features featureset = tgt::InitFeature::ALL;
        tgt::init(featureset);
        LogMgr.getConsoleLog()->addCat("", true);

        // create a local OpenGL context and init GL
        _localContext = CtxtMgr.createContext("AppContext", "", tgt::ivec2(16, 16));
        tgtAssert(_localContext != 0, "Could not create local OpenGL context");
        tgt::initGL(featureset);
        LGL_ERROR;

        if (_argc > 0) {
            // ugly hack
            std::string programPath(_argv[0]);
            programPath = tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(programPath)));
            ShdrMgr.addPath(programPath);
            ShdrMgr.addPath(programPath + "/core/glsl");
        }

        // init pipeline first
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            (*it)->init();
        }

        // Now init painters:
        for (std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->init();
        }

        _initialized = true;
    }

    void TumVisApplication::deinit() {
        tgtAssert(_initialized, "Tried to deinitialize uninitialized TumVisApplication.");

        // Deinit pipeline first
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            (*it)->deinit();
        }

        // Now deinit painters:
        for (std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->deinit();
        }

        // deinit OpenGL and tgt
        tgt::deinitGL();
        tgt::QtContextManager::deinit();
        tgt::deinit();

        _initialized = false;
    }

    int TumVisApplication::run() {
        tgtAssert(_initialized, "Tried to run uninitialized TumVisApplication.");

        // disconnect OpenGL context from this thread so that the other threads can acquire an OpenGL context.
        CtxtMgr.releaseCurrentContext();

        // Start evaluator/render threads
        for (std::vector<PipelineEvaluator*>::iterator it = _pipelineEvaluators.begin(); it != _pipelineEvaluators.end(); ++it) {
            (*it)->start();
        }
        for (std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->start();
        }

        // Start QApplication
        int toReturn = QApplication::exec();

        // QApplication has returned -> Stop evaluator/render threads
        for (std::vector<PipelineEvaluator*>::iterator it = _pipelineEvaluators.begin(); it != _pipelineEvaluators.end(); ++it) {
            (*it)->stop();
        }
        for (std::vector< std::pair<VisualizationPipeline*, TumVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->stop();
        }

        return toReturn;
    }

    void TumVisApplication::addPipeline(AbstractPipeline* pipeline) {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");
        _pipelines.push_back(pipeline);

        PipelineEvaluator* pe = new PipelineEvaluator(pipeline);
        _pipelineEvaluators.push_back(pe);
    }

    void TumVisApplication::addVisualizationPipeline(const std::string& name, VisualizationPipeline* vp) {
        tgtAssert(vp != 0, "Pipeline must not be 0.");

        // TODO: is there a more leightweight method to create a context for the pipeline (just performing off-screen rendering)?
        tgt::QtThreadedCanvas* evaluationContext = CtxtMgr.createContext(name + "_eval", "", tgt::ivec2(512, 512));
        vp->setCanvas(evaluationContext);
        addPipeline(vp);

        // create canvas and painter for the VisPipeline and connect all together
        tgt::QtThreadedCanvas* canvas = CtxtMgr.createContext(name, "TUMVis", tgt::ivec2(512, 512));
        canvas->init();

        TumVisPainter* painter = new TumVisPainter(canvas, vp);
        canvas->setPainter(painter, false);
        CtxtMgr.releaseCurrentContext();

        _visualizations.push_back(std::make_pair(vp, painter));
    }

}