// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef VOXELTEXTUREMIPMAPPER_H__
#define VOXELTEXTUREMIPMAPPER_H__

#include <string>

#include "ext/glew/include/GL/glew.h"
#include "tgt/framebufferobject.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "core/datastructures/facegeometry.h"

namespace campvis {
    /**
     * class for generating the mip maps of the voxel texture
     *
     */
    class VoxelTexMipMapGenerator {
    public:
        /**
         * Creates a VoxelTexMipMapGenerator and prepare the framebuffer and shader for next function calls of the object.
         */
        VoxelTexMipMapGenerator();

        void init();

        /**
         * Computes the maximum level of mipmapping.
         * \param   resolution                  largest dimension of the voxelized volume.
         */
        unsigned int computeMaxLevel(unsigned int resolutionX, unsigned int resolutionY);
        
        /**
         * Attaches the mipmaps to the texture with the passed textureID and resolution.
         * \param   voxelTextureID              ID of the texture which the mip maps are going to be attached to.
         * \param   resolution                  largest dimension of the voxelized volume.
         */
        void attachMipmapsTo(tgt::Texture* texture, unsigned int resolutionX, unsigned int resolutionY);
        
        /**
         * renders the mipmaps of the texture with the passed texture object and resolution.
         * \param   voxelTextureID              texture object.
         * \param   resolution                  largest dimension of the voxelized volume.
         */
        void renderMipmapsFor(tgt::Texture* texture, unsigned int resolution, unsigned int resolutionY, unsigned int voxelSize);

        ~VoxelTexMipMapGenerator();
    private:
        tgt::Shader* _shader;           ///< Pointer to the shader object
        FaceGeometry* _quad;
    };

}

#endif // VOXELTEXTUREMIPMAPPER_H__
