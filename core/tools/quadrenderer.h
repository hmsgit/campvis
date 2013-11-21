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

#ifndef QUADRENDERER_H__
#define QUADRENDERER_H__

#include "tgt/singleton.h"
#include "tgt/tgt_gl.h"

namespace campvis {
    class FaceGeometry;

    /**
     * Small helper singleton class for rendering the default [-1, 1]^2 quad.
     * Texture coordinates are [0, 1]^2.
     */
    class QuadRenderer : public tgt::Singleton<QuadRenderer> {
        friend class tgt::Singleton<QuadRenderer>;

    public:
        /**
         * Virtual Destructor
         */
        virtual ~QuadRenderer();

        /**
         * Renders a [-1, 1]^2 quad in the XY plane having texture coordinates [0, 1]^2.
         * \param   mode    OpenGL rendering mode (defaults to GL_POLYGON).
         */
        void renderQuad(GLenum mode = GL_POLYGON);

    private:
        /// Private Constructor, must be called from valid OpenGL context.
        QuadRenderer();

        FaceGeometry* _quad;    ///< The FaceGeometry that renders the quad.
    };

#define QuadRdr tgt::Singleton<campvis::QuadRenderer>::getRef()

}

#endif // QUADRENDERER_H__
