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

#ifndef VOXELHIERARCHYMAPPER_H__
#define VOXELHIERARCHYMAPPER_H__

#include <string>

#include "cgt/framebufferobject.h"
#include "cgt/shadermanager.h"
#include "core/datastructures/facegeometry.h"

namespace tgt {
    class FramebufferObject;
    class Shader;
    class Texture;
}

namespace campvis {
    class AbstractTransferFunction;
    class ImageRepresentationGL;

    /**
     * Helper class to create and manage a voxel hierarchy texture using mip maps.
     * Implementation following the Paper "Voxel-based Global Illumination" by Thiedemann et al. (2011)
     * 
     * \note    All methods (incl. constructor) need a valid OpenGL context when called.
     */
    class VoxelHierarchyMapper {
    public:
        /**
         * Creates a VoxelHierarchyMapper and prepare the framebuffer and shader for next function calls of the object.
         * \note    Needs valid OpenGL context!
         */
        VoxelHierarchyMapper();

        /// Destructor
        ~VoxelHierarchyMapper();


        void createHierarchy(const ImageRepresentationGL* image, AbstractTransferFunction* transferFunction);


        tgt::Texture* getTexture() { return _hierarchyTexture; }
        size_t getBrickSize() { return _brickSize; }
        size_t getBrickDepth() { return _brickDepth; }
        GLuint getMaxMipmapLevel() { return _maxMipmapLevel; }

    private:

        /**
         * Computes the maximum level of mipmapping.
         * \param   resolution                  largest dimension of the voxelized volume.
         */
        GLuint computeMaxLevel(size_t resolutionX, size_t resolutionY);
        
        /**
         * renders the mipmaps of the texture with the passed texture object and resolution.
         */
        void renderMipmaps();

        void createEmptyTextureWithMipMaps();

        void renderInitialHierarchy(const ImageRepresentationGL* image, AbstractTransferFunction* transferFunction);

        tgt::svec3 _dimBricks;                      ///< number of bricks in each dimension
        tgt::svec3 _dimPackedBricks;                ///< number of elements when bricks are tightly packed
        size_t _brickSize;                          ///< number of voxels a brick is covering in x, y dimension
        size_t _brickDepth;                         ///< number of voxel a brick is covering in its depth dimension
        GLuint _maxMipmapLevel;                     ///< Maximum mipmap level

        tgt::Shader* _hierarchyRendererShader;      ///< Shader to render the initial hierarchy texture
        tgt::Shader* _mimapRendererShader;          ///< Shader to render the mipmap pyramid

        tgt::FramebufferObject* _fbo;               ///< FBO used for rendering
        tgt::Texture* _hierarchyTexture;            ///< The OpenGL texture storing the voxel hierarchy

        FaceGeometry* _quad;


        static std::string loggerCat_;
    };

}

#endif // VOXELHIERARCHYMAPPER_H__
