#include "sigslot/sigslot.h"
#include "tbb/include/tbb/atomic.h"
#include "tbb/include/tbb/compat/condition_variable"

#include "tgt/logmanager.h"
#include "tgt/painter.h"
#include "core/tools/runnable.h"

namespace tgt {
    class Shader;
    class QtThreadedCanvas;
}

namespace TUMVis {
    class VisualizationPipeline;

    /**
     * Painter class for TUMVis, rendering the render target of a VisualizationPipeline.
     * This painter implements Runnable, hence, it runs in its own thread and the associated canvas
     * must be of type QtThreadedCanvas.
     * Rendering is implemented using condidional wait - hence the canvas is only updated when
     * \a pipeline emits the s_RenderTargetChanged signal.
     * 
     * \sa  Runnable, VisualizationPipeline
     */
    class TumVisPainter : public Runnable, public tgt::Painter, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new TumVisPainter rendering the render target of \a pipeline on \a canvas.
         * \param   canvas      Canvas to render on
         * \param   pipeline    Pipeline to render
         */
        TumVisPainter(tgt::QtThreadedCanvas* canvas, VisualizationPipeline* pipeline);

        /**
         * Destructor, stops and waits for the rendering thread if it's still running.
         */
        virtual ~TumVisPainter();

        /// \see Runnable::stop
        void stop();
        
        /**
         * Performs the rendering using conditional wait.
         * \sa Runnable::run
         */
        void run();

        /**
         * Performs the actual rendering of the pipeline's render target
         */
        virtual void paint();

        /// \see tgt::Painter::sizeChanged
        virtual void sizeChanged(const tgt::ivec2& size);

        /**
         * Initializes the painter, i.e. loads the OpenGL shader.
         */
        virtual void init();

        /**
         * Deinitializes the painter, i.e. disposes its shader.
         */
        virtual void deinit();

        /**
         * Sets the target canvas for rendering
         * \param   canvas  Canvas to render on, must be of type QtThreadedCanvas
         */
        virtual void setCanvas(tgt::GLCanvas* canvas);

        /**
         * Pipeline with the render target to render.
         * \param   pipeline    Pipeline to render
         */
        void setPipeline(VisualizationPipeline* pipeline);

        /**
         * Slot being notified when the pipeline's render target changed.
         */
        void onRenderTargetChanged();

    private:
        static const std::string loggerCat_;

        VisualizationPipeline* _pipeline;                   ///< Pipeline to render
        tgt::Shader* _copyShader;                           ///< Shader for copying the render target to the framebuffer.
        tbb::atomic<bool> _dirty;                           ///< Flag whether render result is dirty and needs to be rerendered.
        std::condition_variable _renderCondition;           ///< conditional wait condition for rendering
    };

}
