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

#include "init.h"

#include "sigslot/sigslot.h"

#include "cgt/init.h"
#include "cgt/glcontextmanager.h"
#include "cgt/logmanager.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/shadermanager.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "core/datastructures/imagerepresentationconverter.h"
#include "core/pipeline/pipelinefactory.h"
#include "core/pipeline/processorfactory.h"
#include "core/tools/quadrenderer.h"
#include "core/tools/simplejobprocessor.h"

namespace campvis {

    void init(cgt::GLCanvas* backgroundGlContext, const std::vector<std::string>& searchPaths) {
        // start sigslot signal manager
        sigslot::signal_manager::init();
        sigslot::signal_manager::getRef().start();
        SimpleJobProcessor::init();

        // Init CGT
        cgt::init(cgt::InitFeature::ALL, cgt::Debug);
        cgt::initGL(backgroundGlContext, cgt::InitFeature::ALL);
         
        // ensure matching OpenGL specs
        LINFOC("CAMPVis.core.init", "Using Graphics Hardware " << GpuCaps.getVendorAsString() << " " << GpuCaps.getGlRendererString() << " on " << GpuCaps.getOSVersionString());
        LINFOC("CAMPVis.core.init", "Supported OpenGL " << GpuCaps.getGlVersion() << ", GLSL " << GpuCaps.getShaderVersion());
        if (GpuCaps.getGlVersion() < cgt::GpuCapabilities::GlVersion::CGT_GL_VERSION_3_3) {
            LERRORC("CAMPVis.core.init", "Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
        }
        if (GpuCaps.getShaderVersion() < cgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
            LERRORC("CAMPVis.core.init", "Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
        }

        ShdrMgr.setDefaultGlslVersion("330");
        for (auto it = searchPaths.cbegin(); it != searchPaths.cend(); ++it) {
            ShdrMgr.addPath(*it);
            ShdrMgr.addPath(*it + "/core/glsl");
        }

        QuadRenderer::init();
        LGL_ERROR;

        GLCtxtMgr.releaseContext(backgroundGlContext, false);
        GLJobProc.setContext(backgroundGlContext);
        GLJobProc.start();
    }

    void deinit() {
        {
            // Deinit everything OpenGL related using the background context.
            cgt::GLContextScopedLock lock(GLJobProc.getContext());
            QuadRenderer::deinit();
        }

        cgt::deinitGL();
        cgt::deinit();

        SimpleJobProcessor::deinit();
        ImageRepresentationConverter::deinit();
        PipelineFactory::deinit();
        ProcessorFactory::deinit();
    }

    CAMPVIS_CORE_API std::string completePath(const std::string& filename) {
        if (! cgt::ShaderManager::isInited())
            return "";

        return ShdrMgr.completePath(filename);
    }

}
