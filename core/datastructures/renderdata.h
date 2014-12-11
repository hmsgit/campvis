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

#ifndef RENDERDATA_H__
#define RENDERDATA_H__

#include "core/datastructures/datahandle.h"
#include "core/datastructures/abstractdata.h"

#include <string>
#include <vector>

namespace cgt {
    class FramebufferObject;
    class Shader;
    class TextureUnit;
}

namespace campvis {
    class ImageData;

    /**
     * Class storing render data (i.e. rendering results).
     * RenderData itself is only a container for multiple ImageData objects representing color
     * and/or depth textures.
     * 
     * \note    Write access to RenderData is \b not thread-safe!
     * \note    RenderData does not care whether its textures have the same size and their formats
     *          make sense.
     */
    class CAMPVIS_CORE_API RenderData : public AbstractData {
    public:
        /**
         * Constructor, creates empty RenderData.
         */
        RenderData();

        /**
         * Constructur, creates RenderData from all textures attached to \a fbo.
         * \note    Must be called from a valid OpenGL context.
         * \param   fbo     FBO to use textures from.
         */
        explicit RenderData(const cgt::FramebufferObject* fbo);

        /**
         * Destructor
         */
        virtual ~RenderData();

        /// \see AbstractData::clone()
        virtual RenderData* clone() const;
        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;
        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;
        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;


        /**
         * Gets the number of color textures in this RenderData.
         * \return  _colorTextures.size()
         */
        size_t getNumColorTextures() const;

        /**
         * Gets the color texture in this RenderData.
         * \param   index   Index of the color texture to return.
         * \return  _colorTextures[index], 0 if index out of bounds.
         */
        const ImageData* getColorTexture(size_t index = 0) const;

        /**
         * Gets the DataHandle with the color texture in this RenderData.
         * \param   index   Index of the color texture to return.
         * \return  _colorTextures[index], 0 if index out of bounds.
         */
        DataHandle getColorDataHandle(size_t index = 0) const;

        /**
         * Returns, whether this RenderData has a depth texture.
         * \return  _depthTexture != 0;
         */
        bool hasDepthTexture() const;

        /**
         * Gets the depth texture in this RenderData, if present.
         * Returns 0 if no depth texture is present.
         * \return  _depthTexture, may be 0.
         */
        const ImageData* getDepthTexture() const;

        /**
         * Gets the DataHandle with the depth texture in this RenderData, if present.
         * Returns 0 if no depth texture is present.
         * \return  _depthTexture, may be 0.
         */
        DataHandle getDepthDataHandle() const;

        /**
         * Adds \a texture to this RenderData.
         * \note    RenderData takes ownership of \a texture.
         * \param   texture     Color texture to add.
         */
        void addColorTexture(ImageData* texture);

        /**
         * Sets \a texture as depth texture of this RenderData.
         * \note    RenderData takes ownership of \a texture.
         * \param   texture     New depth texture.
         */
        void setDepthTexture(ImageData* texture);


        
        /**
         * Binds the color texture with index \a index of this render target
         * and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param colorTexUnit              Color texture unit.
         * \param colorTexUniform           Name for color texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         * \param index                     Index of the color texture to bind.
         */
        void bindColorTexture(
            cgt::Shader* shader,
            const cgt::TextureUnit& colorTexUnit, 
            const std::string& colorTexUniform = "_colorTexture",
            const std::string& texParamsUniform = "_texParams",
            size_t index = 0) const;
        
        /**
         * Binds the depth texture of this render target and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param depthTexUnit              Depth texture unit.
         * \param depthTexUniform           Name for depth texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         */
        void bindDepthTexture(
            cgt::Shader* shader,
            const cgt::TextureUnit& depthTexUnit,
            const std::string& depthTexUniform = "_depthTexture", 
            const std::string& texParamsUniform = "_texParams") const;

        /**
         * Binds the color texture with index \a index and the depth texture of this render target
         * and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param colorTexUnit              Color texture unit.
         * \param depthTexUnit              Depth texture unit.
         * \param colorTexUniform           Name for color texture sampler.
         * \param depthTexUniform           Name for depth texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         * \param index                     Index of the color texture to bind.
         */
        void bind(
            cgt::Shader* shader,
            const cgt::TextureUnit& colorTexUnit, 
            const cgt::TextureUnit& depthTexUnit, 
            const std::string& colorTexUniform = "_colorTexture",
            const std::string& depthTexUniform = "_depthTexture",
            const std::string& texParamsUniform = "_texParams",
            size_t index = 0) const;

    protected:
        std::vector<DataHandle> _colorTextures;     ///< color textures
        DataHandle _depthTexture;                   ///< depth texture

        static const std::string loggerCat_;

    };

}

#endif // RENDERDATA_H__
