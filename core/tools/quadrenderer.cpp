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

#include "quadrenderer.h"
#include "core/datastructures/geometrydatafactory.h"

namespace campvis {


    QuadRenderer::QuadRenderer()
        : cgt::Singleton<QuadRenderer>()
        , _quad11(GeometryDataFactory::createQuad(cgt::vec3(-1.f, -1.f, 0.f), cgt::vec3(1.f), cgt::vec3(0.f), cgt::vec3(1.f)))
        , _quad01(GeometryDataFactory::createQuad(cgt::vec3(0.f), cgt::vec3(1.f), cgt::vec3(0.f), cgt::vec3(1.f)))
    {}

    QuadRenderer::~QuadRenderer() {
    }

    void QuadRenderer::renderQuad11(GLenum mode) {
        _quad11->render(mode);
    }

    void QuadRenderer::renderQuad01(GLenum mode /*= GL_TRIANGLE_FAN*/) {
        _quad01->render(mode);
    }


}