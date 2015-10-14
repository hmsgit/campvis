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

#ifndef CAMPVISPAINTER_H__
#define CAMPVISPAINTER_H__

#include "sigslot/sigslot.h"
#include "cgt/glcanvas.h"
#include "cgt/painter.h"

#include "core/coreapi.h"

namespace cgt {
    class GLCanvas;
    class Shader;
    class Texture;
}

namespace campvis {
    class AbstractPipeline;

    /**
     * Painter class for CAMPVis, rendering the render target of an AbstractPipeline.
     */
    class CAMPVIS_CORE_API PipelinePainter : public cgt::Painter, public sigslot::has_slots {
    public:
        /**
         * Creates a new PipelinePainter rendering the render target of \a pipeline on \a canvas.
         * \param   canvas      Canvas to render on
         * \param   pipeline    Pipeline to render
         */
        PipelinePainter(cgt::GLCanvas* canvas, AbstractPipeline* pipeline);

        /**
         * Schedule a repaint job for the pipeline's render target
         */
        virtual void repaint();

        /// \see cgt::Painter::sizeChanged
        virtual void sizeChanged(const cgt::ivec2& size);

        /**
         * Initializes the painter, i.e. loads the OpenGL shader.
         */
        virtual void init();

        /**
         * Deinitializes the painter, i.e. disposes its shader.
         */
        virtual void deinit();

        virtual void setCanvas(cgt::GLCanvas* canvas) override;

        /**
         * Sets the error texture to \a texture.
         * \param   texture     The new error texture to display when no rendering can be shown, must not be 0.
         */
        void setErrorTexture(cgt::Texture* texture);


    private:
        /**
         * Performs the actual rendering of the pipeline's render target
         */
        virtual void paint();

        static const std::string loggerCat_;

        AbstractPipeline* _pipeline;                        ///< Pipeline to render
        cgt::Shader* _copyShader;                           ///< Shader for copying the render target to the framebuffer.
        cgt::Texture* _errorTexture;                        ///< Pointer to error texture
    };

}

#endif // CAMPVISPAINTER_H__
