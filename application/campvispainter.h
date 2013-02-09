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

#ifndef CAMPVISPAINTER_H__
#define CAMPVISPAINTER_H__

#include "sigslot/sigslot.h"
#include "tbb/atomic.h"
#include "tbb/compat/condition_variable"

#include "tgt/logmanager.h"
#include "tgt/painter.h"
#include "core/tools/runnable.h"

namespace tgt {
    class Shader;
    class QtThreadedCanvas;
}

namespace campvis {
    class VisualizationPipeline;

    /**
     * Painter class for CAMPVis, rendering the render target of a VisualizationPipeline.
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
        TumVisPainter(tgt::GLCanvas* canvas, VisualizationPipeline* pipeline);

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

#endif // CAMPVISPAINTER_H__
