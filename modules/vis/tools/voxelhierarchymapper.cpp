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

#include "voxelhierarchymapper.h"

#include "tgt/assert.h"
#include "tgt/framebufferobject.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"
#include "tgt/tgt_gl.h"

#include "core/classification/abstracttransferfunction.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/geometrydatafactory.h"

#define VOXEL_DEPTH_MIPMAPPING 32
#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

namespace campvis {

    std::string VoxelHierarchyMapper::loggerCat_ = "CAMPVis.modules.vis.VoxelHierarchyMapper";

    VoxelHierarchyMapper::VoxelHierarchyMapper()
        : _hierarchyRendererShader(nullptr)
        , _mimapRendererShader(nullptr)
        , _fbo(nullptr)
        , _hierarchyTexture(nullptr)
        , _quad(nullptr)
    {  
        _hierarchyRendererShader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/vis/glsl/hierarchyrenderer.frag", "", "400");
        _mimapRendererShader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/vis/glsl/mipmaprenderer.frag", "", "400");
        _fbo = new tgt::FramebufferObject();

        _quad = GeometryDataFactory::createQuad(tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(1.f, 1.f, 0.f), tgt::vec3(0.f, 0.f, 0.f));
    }

    VoxelHierarchyMapper::~VoxelHierarchyMapper() {
        ShdrMgr.dispose(_hierarchyRendererShader);
        ShdrMgr.dispose(_mimapRendererShader);
        delete _quad;
    }

    void VoxelHierarchyMapper::createHierarchy(const ImageRepresentationGL* image, AbstractTransferFunction* transferFunction) {
        tgtAssert(image != nullptr, "Image must not be 0.");
        tgtAssert(transferFunction != nullptr, "Transfer function must not be 0.");
        
        // perform ceiling integer division:
        // z is not considered.
        _brickSize = 2;
        _dimBricks = image->getSize();
        for (int i = 0; i < 2; ++i)
            _dimBricks.elem[i] = DIV_CEIL(_dimBricks.elem[i], _brickSize);

        // set the depth of the bricks
        _brickDepth = _dimBricks.z / 32;

        // since the texture is a 2D texture and the elements store the depth  will pack VOXEL_DEPTH number of values along the z axis into one block, the _dimBricks.z is 
        _dimBricks.z = 32;

        _dimPackedBricks = _dimBricks;
        _dimPackedBricks.z = _dimPackedBricks.z / 32;

        _maxMipmapLevel = computeMaxLevel(_dimPackedBricks.x, _dimPackedBricks.y);

        tgtAssert(_dimPackedBricks.z == 1, "This should not happen!");


        // create initial texture:
        createEmptyTextureWithMipMaps();
        renderInitialHierarchy(image, transferFunction);

        // generate mipmap levels
        renderMipmaps();
    }



    GLuint VoxelHierarchyMapper::computeMaxLevel(size_t resolutionX, size_t resolutionY) {
        GLuint count = 0;
        GLuint resolution = static_cast<GLuint>(std::max(resolutionX, resolutionY));

        while (resolution) {
            resolution /= 2;
            count++;
        }
        return count-1;
    }

    void VoxelHierarchyMapper::renderMipmaps() {
        LDEBUG("Start computing the levels of the voxel object.");

        /// Activate the shader for geometry Rendering.
        _mimapRendererShader->activate();
        _mimapRendererShader->setUniform("_projectionMatrix", tgt::mat4::createOrtho(0, 1, 0, 1, -1, 1));

        tgt::TextureUnit bbvUnit;
        bbvUnit.activate();
        _hierarchyTexture->bind();
        _mimapRendererShader->setUniform("_voxelTexture", bbvUnit.getUnitNumber());
        
        _fbo->activate();

        for (GLuint level = 0; level < _maxMipmapLevel; ++level) {
            double resX = _hierarchyTexture->getWidth() / pow(2.0, static_cast<double>(level));
            double resY = _hierarchyTexture->getHeight() / pow(2.0, static_cast<double>(level));

            _mimapRendererShader->setUniform("_level", static_cast<int>(level));
            _mimapRendererShader->setUniform("_inverseTexSizeX", 1.f / static_cast<float>(resX));
            _mimapRendererShader->setUniform("_inverseTexSizeY", 1.f / static_cast<float>(resY));

            _fbo->attachTexture(_hierarchyTexture, GL_COLOR_ATTACHMENT0, level+1, 0);
            _fbo->isComplete();

            glViewport(0, 0, static_cast<GLsizei>(resX / 2.0), static_cast<GLsizei>(resY / 2.0));
            _quad->render(GL_POLYGON);
        }

        _fbo->deactivate();
        _mimapRendererShader->deactivate();    
        LGL_ERROR;

        LDEBUG("...finished computing voxel visibilities mip maps.");
    }


    void VoxelHierarchyMapper::createEmptyTextureWithMipMaps() {
        // delete old stuff
        delete _hierarchyTexture;

        tgt::TextureUnit tempUnit;
        tempUnit.activate();

        // create new texture
        _hierarchyTexture = new tgt::Texture(0, _dimPackedBricks, GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_INT, tgt::Texture::NEAREST);
        _hierarchyTexture->bind();
        _hierarchyTexture->uploadTexture();
        _hierarchyTexture->setWrapping(tgt::Texture::CLAMP);
        LGL_ERROR;

        // attach mipmaps manually (as we want special ones)
        glBindTexture(GL_TEXTURE_2D, _hierarchyTexture->getId());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _maxMipmapLevel);  

        int div = 2;
        for (GLuint level = 1; level <= _maxMipmapLevel; ++level) {
            glTexImage2D(GL_TEXTURE_2D, level, GL_R32UI, _hierarchyTexture->getWidth()/div, _hierarchyTexture->getHeight()/div, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
            div = div << 1;
        }
        LGL_ERROR;
    }

    void VoxelHierarchyMapper::renderInitialHierarchy(const ImageRepresentationGL* image, AbstractTransferFunction* transferFunction) {
        LDEBUG("Start computing voxel visibilities...");

        _hierarchyRendererShader->activate();
        _hierarchyRendererShader->setUniform("_projectionMatrix", tgt::mat4::createOrtho(0, 1, 0, 1, -1, 1));
        _hierarchyRendererShader->setUniform("_voxelDepth", static_cast<GLuint>(_brickDepth));
        _hierarchyRendererShader->setUniform("_voxelSize", static_cast<GLuint>(_brickSize));

        tgt::TextureUnit volumeUnit, tfUnit;
        image->bind(_hierarchyRendererShader, volumeUnit, "_volume", "_volumeTextureParams");
        transferFunction->bind(_hierarchyRendererShader, tfUnit, "_transferFunction", "_transferFunctionParams");

        _fbo->activate();
        _fbo->attachTexture(_hierarchyTexture, GL_COLOR_ATTACHMENT0, 0, 0);
        _fbo->isComplete();
        glViewport(0, 0, static_cast<GLsizei>(_hierarchyTexture->getWidth()), static_cast<GLsizei>(_hierarchyTexture->getHeight()));

        _quad->render(GL_POLYGON);

        _fbo->deactivate();
        _hierarchyRendererShader->deactivate();    

        LGL_ERROR;

        LDEBUG("...finished computing voxel visibilities.");
    }

}