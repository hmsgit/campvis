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

#ifndef CAMPVISPAINTER_H__
#define CAMPVISPAINTER_H__

#include "sigslot/sigslot.h"

#include <ext/threading.h>
#include <tbb/atomic.h>

#include "tgt/logmanager.h"
#include "tgt/painter.h"

namespace tgt {
    class Shader;
    class QtThreadedCanvas;
    class Texture;
}

namespace campvis {
    class AbstractPipeline;

    /**
     * Painter class for CAMPVis, rendering the render target of an AbstractPipeline.
     */
    class CampVisPainter : public tgt::Painter, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new CampVisPainter rendering the render target of \a pipeline on \a canvas.
         * \param   canvas      Canvas to render on
         * \param   pipeline    Pipeline to render
         */
        CampVisPainter(tgt::GLCanvas* canvas, AbstractPipeline* pipeline);

        /**
         * Destructor, stops and waits for the rendering thread if it's still running.
         */
        virtual ~CampVisPainter();

        /**
         * Schedule a repaint job for the pipeline's render target
         */
        virtual void repaint();

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
        void setPipeline(AbstractPipeline* pipeline);

        void setErrorTexture(tgt::Texture* texture);

        /**
         * Slot being notified when the pipeline's render target changed.
         */
        void onRenderTargetChanged();

    private:
        /**
         * Performs the actual rendering of the pipeline's render target
         */
        virtual void paint();

        static const std::string loggerCat_;

        AbstractPipeline* _pipeline;                        ///< Pipeline to render
        tgt::Shader* _copyShader;                           ///< Shader for copying the render target to the framebuffer.
        tbb::atomic<bool> _dirty;                           ///< Flag whether render result is dirty and needs to be rerendered.
        std::condition_variable _renderCondition;           ///< conditional wait condition for rendering

        tgt::Texture* _errorTexture;
    };

}

#endif // CAMPVISPAINTER_H__
