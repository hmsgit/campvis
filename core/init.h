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

#ifndef CAMPVIS_H__
#define CAMPVIS_H__

#include "cgt/singleton.h"
#include "core/coreapi.h"

#include <string>
#include <vector>

namespace cgt {
    class GLCanvas;
}

namespace campvis {

    /**
     * Initializes the CAMPVis core stuff.
     * This includes all cgt, sigslot, and campvis singletons. Make sure to call this function 
     * before using any other CAMPVis methods/classes.
     * \param   backgroundGlContext     OpenGL context to use by CAMPVis for background OpenGL tasks. Must not be 0.
     * \param   searchPaths             Seach paths to use for shaders.
     */
    CAMPVIS_CORE_API void init(cgt::GLCanvas* backgroundGlContext, const std::vector<std::string>& searchPaths = std::vector<std::string>());

    /**
     * Deinitializes the CAMPVis core stuff.
     */
    CAMPVIS_CORE_API void deinit();


    /**
     * Searches in all search paths for the given file and returns valid filename including complete path.
     * If file is not found in search path, an empty string is returned.
     * \note    The function is just a proxy for ShaderManager::completePath.
     * \param   filename    The file name to search for in search paths.
     */
    CAMPVIS_CORE_API std::string completePath(const std::string& filename);
}


#endif // CAMPVIS_H__