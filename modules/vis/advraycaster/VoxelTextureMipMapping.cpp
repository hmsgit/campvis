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

#include "VoxelTextureMipMapping.h"

#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/tgt_gl.h"

#include "core/datastructures/geometrydatafactory.h"

#define VOXEL_DEPTH_MIPMAPPING 32

namespace campvis {
    VoxelTexMipMapGenerator::VoxelTexMipMapGenerator() {   
        _quad = GeometryDataFactory::createQuad(tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(1.f, 1.f, 0.f), tgt::vec3(0.f, 0.f, 0.f));
        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/vis/advraycaster/glsl/MipmapVoxelTexture.frag", "", "330");        
    }

    void VoxelTexMipMapGenerator::init() {
    }

    unsigned int VoxelTexMipMapGenerator::computeMaxLevel(unsigned int resolutionX, unsigned int resolutionY) {
        unsigned int count = 0;

        unsigned int resolution = std::max(resolutionX, resolutionY);

        while(resolution){
            resolution /= 2;
            count++;
        }
        return count-1;
    }

    void VoxelTexMipMapGenerator::attachMipmapsTo(tgt::Texture* texture, unsigned int resolutionX, unsigned int resolutionY) {
        unsigned int maxLevel = computeMaxLevel(resolutionX, resolutionY);
       
        LGL_ERROR;

        // attach mipmap levels to this voxel texture
        glBindTexture(GL_TEXTURE_2D, texture->getId());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        LGL_ERROR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        LGL_ERROR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        LGL_ERROR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLevel);  

        LGL_ERROR;

        int div = 2;
        for(unsigned int level = 1; level <= maxLevel; level++){
#if (VOXEL_DEPTH_MIPMAPPING == 8)
            glTexImage2D(GL_TEXTURE_2D, level, GL_R32UI, resolutionX/div, resolutionY/div, 0, GL_RED_INTEGER_EXT, GL_UNSIGNED_BYTE, 0);
#else if (VOXEL_DEPTH_MIPMAPPING == 32)
            glTexImage2D(GL_TEXTURE_2D, level, GL_R32UI, resolutionX/div, resolutionY/div, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
#endif
            div = div << 1;
            LGL_ERROR;
        }
    }

    void VoxelTexMipMapGenerator::renderMipmapsFor(tgt::Texture* texture, unsigned int resolutionX, unsigned int resolutionY, unsigned int voxelSize) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        LGL_ERROR;

        unsigned int maxLevel = computeMaxLevel(resolutionX, resolutionY);
        
        tgt::FramebufferObject frameBuffer;

        LGL_ERROR;
        /// Activate the shader for geometry Rendering.
        _shader->activate();

       tgt::TextureUnit bbvUnit;
       bbvUnit.activate();
       texture->bind();
       LGL_ERROR;
       _shader->setIgnoreUniformLocationError(true);
           _shader->setUniform("_voxelTexture", bbvUnit.getUnitNumber());
           LGL_ERROR;
           _shader->setUniform("_voxelTextureParams._size", tgt::vec2(static_cast<float>(texture->getDimensions().r), static_cast<float>(texture->getDimensions().g)));
           LGL_ERROR;
           _shader->setUniform("_voxelTextureParams._sizeRCP", tgt::vec3(1.f) / tgt::vec3(texture->getDimensions()));
           LGL_ERROR;
           _shader->setUniform("_voxelTextureParams._numChannels", static_cast<int>(1));
           LGL_ERROR;
       _shader->setIgnoreUniformLocationError(false);
        LGL_ERROR;
       frameBuffer.activate();

       // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
       glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

       for(unsigned int i = 0; i < maxLevel; i++){  
           // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
           glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

           tgt::mat4 projection = tgt::mat4::createOrtho(0, 1, 0, 1, -1, 1);
           _shader->setUniform("_projectionMatrix", projection);

           _shader->setUniform("_level", static_cast<int>(i));
           float invResXFloat = static_cast<float>( 1.0/(resolutionX/pow(2.0,double(i))) );
           float invResYFloat = static_cast<float>( 1.0/(resolutionY/pow(2.0,double(i))) );
           _shader->setUniform("_inverseTexSizeX", static_cast<float>(invResXFloat));
           _shader->setUniform("_inverseTexSizeY", static_cast<float>(invResYFloat));
           LGL_ERROR;

           glViewport(0, 0, static_cast<GLsizei>(resolutionX/pow(2.0,double(i+1))), static_cast<GLsizei>(resolutionY/pow(2.0,double(i+1))));

           frameBuffer.attachTexture(texture, GL_COLOR_ATTACHMENT0, i+1, 0);
           frameBuffer.isComplete();

           LGL_ERROR;
           _quad->render(GL_POLYGON);
           LGL_ERROR;
        }

        _shader->deactivate();    
        frameBuffer.deactivate();

        glPopAttrib();
        LGL_ERROR;
    }

    VoxelTexMipMapGenerator::~VoxelTexMipMapGenerator() {
        ShdrMgr.dispose(_shader);
        delete _quad;
    }
}