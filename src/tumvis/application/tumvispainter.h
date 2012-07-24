#include "sigslot/sigslot.h"
#include "tbb/include/tbb/mutex.h"

#include "tgt/logmanager.h"
#include "tgt/glcanvas.h"
#include "tgt/qt/qtthreadedpainter.h"
#include "tgt/event/eventhandler.h"

#include "core/pipeline/visualizationpipeline.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {

    /**
     * Painter class for TUMVis
     * 
     * \todo 
     */
    class TumVisPainter : public tgt::QtThreadedPainter, public sigslot::has_slots<> {
    public:
        TumVisPainter(tgt::QtCanvas* canvas, VisualizationPipeline* pipeline);

        virtual void paint();  
        virtual void sizeChanged(const tgt::ivec2& size);
        virtual void init();
        virtual void deinit();

        void setPipeline(VisualizationPipeline* pipeline);
        void onPipelineInvalidated();

    private:
        static const std::string loggerCat_;

        VisualizationPipeline* _pipeline;
        bool _dirty;
        bool _currentlyRendering;
        tgt::Shader* _copyShader;                           ///< Shader for copying the render target to the framebuffer.
        tbb::mutex _localMutex;

    };

}
