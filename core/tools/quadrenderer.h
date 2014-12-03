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

#ifndef QUADRENDERER_H__
#define QUADRENDERER_H__

#include "cgt/singleton.h"
#include "cgt/cgt_gl.h"

#include "core/coreapi.h"

#include <memory>

namespace campvis {
    class FaceGeometry;

    /**
     * Small helper singleton class for rendering the default [-1, 1]^2 quad.
     * Texture coordinates are [0, 1]^2.
     */
    class CAMPVIS_CORE_API QuadRenderer : public cgt::Singleton<QuadRenderer> {
        friend class cgt::Singleton<QuadRenderer>;

    public:
        /**
         * Virtual Destructor
         */
        virtual ~QuadRenderer();

        /**
         * DEPRECATED: Renders a [-1, 1]^2 quad in the XY plane having texture coordinates [0, 1]^2.
         * \deprecated  Only there fore backwards compatibility, directly calls renderQuad11(mode).
         * \param   mode    OpenGL rendering mode (defaults to GL_TRIANGLE_FAN).
         */
        inline void renderQuad(GLenum mode = GL_TRIANGLE_FAN) {
            renderQuad11(mode);
        }

        /**
         * Renders a [-1, 1]^2 quad in the XY plane having texture coordinates [0, 1]^2.
         * \param   mode    OpenGL rendering mode (defaults to GL_TRIANGLE_FAN).
         */
        void renderQuad11(GLenum mode = GL_TRIANGLE_FAN);

        /**
         * Renders a [-1, 1]^2 quad in the XY plane having texture coordinates [0, 1]^2.
         * \param   mode    OpenGL rendering mode (defaults to GL_TRIANGLE_FAN).
         */
        void renderQuad01(GLenum mode = GL_TRIANGLE_FAN);

    private:
        /// Private Constructor, must be called from valid OpenGL context.
        QuadRenderer();

        std::unique_ptr<FaceGeometry> _quad11;  ///< The FaceGeometry that renders the [-1, 1] quad.
        std::unique_ptr<FaceGeometry> _quad01;  ///< The FaceGeometry that renders the [0, 1] quad.
    };

#define QuadRdr cgt::Singleton<campvis::QuadRenderer>::getRef()

}

#endif // QUADRENDERER_H__
