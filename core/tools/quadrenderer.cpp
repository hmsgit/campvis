// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "quadrenderer.h"
#include "core/datastructures/facegeometry.h"

namespace campvis {


    QuadRenderer::QuadRenderer()
        : tgt::Singleton<QuadRenderer>()
        , _quad(0)
    {
        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3(-1.f, -1.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f, -1.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f,  1.f, 0.f));
        vertices.push_back(tgt::vec3(-1.f,  1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 1.f, 0.f));

        _quad = new FaceGeometry(vertices, texCorods);
        _quad->createGLBuffers();
    }

    QuadRenderer::~QuadRenderer() {
        delete _quad;
    }

    void QuadRenderer::renderQuad() {
        _quad->render();
    }


}