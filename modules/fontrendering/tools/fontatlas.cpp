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

#include "fontatlas.h"

#include "cgt/exception.h"
#include "cgt/logmanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/facegeometry.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace campvis {
    namespace fontrendering {

        const int FontAtlas::MAX_TEXTURE_WIDTH = 1024;
        const std::string FontAtlas::loggerCat_ = "CAMPVis.modules.fontrendering.FontAtlas";

        FontAtlas::FontAtlas(const std::string& fontFileName, int height) 
            : _ftLibrary(nullptr)
            , _ftFace(nullptr)
            , _texture(nullptr)
            , _shader(nullptr)
            , _height(height)
        {
            // Initialize the FreeType2 library 
            if (FT_Init_FreeType(&_ftLibrary)) {
                LERROR("Could not init freetype library.");
                throw cgt::Exception("Could not init freetype library.");
            }
            // Load a font
            if (FT_New_Face(_ftLibrary, fontFileName.c_str(), 0, &_ftFace)) {
                LERROR("Could not open font " << fontFileName);
                throw cgt::Exception("Could not open font file.");
            }

            FT_Set_Pixel_Sizes(_ftFace, 0, height);
            FT_GlyphSlot g = _ftFace->glyph;

            int roww = 0;
            int rowh = 0;
            int w = 0;
            int h = 0;
            memset(_glyphs, 0, sizeof(_glyphs));

            // Find minimum size for a texture holding all visible ASCII characters
            for (int i = 32; i < 256; i++) {
                if (FT_Load_Char(_ftFace, i, FT_LOAD_RENDER)) {
                    LERROR("Loading character " << i << " failed!");
                    continue;
                }
                if (roww + g->bitmap.width + 1 >= MAX_TEXTURE_WIDTH) {
                    w = std::max(w, roww);
                    h += rowh;
                    roww = 0;
                    rowh = 0;
                }
                roww += g->bitmap.width + 1;
                rowh = std::max(rowh, static_cast<int>(g->bitmap.rows));
            }
            w = std::max(w, roww);
            h += rowh;

            // create the texture to hold the atlas
            cgt::TextureUnit textureUnit;
            textureUnit.activate();
            _texture = new cgt::Texture(GL_TEXTURE_2D, cgt::ivec3(w, h, 1), GL_R8);
            _texture->setWrapping(cgt::Texture::CLAMP_TO_EDGE);

            // Paste all glyph bitmaps into the texture, remembering the offset
            int ox = 0;
            int oy = 0;
            rowh = 0;
            for (int i = 32; i < 256; i++) {
                if (FT_Load_Char(_ftFace, i, FT_LOAD_RENDER)) {
                    LERROR("Loading character " << i << " failed!");
                    continue;
                }
                if (ox + g->bitmap.width + 1 >= MAX_TEXTURE_WIDTH) {
                    oy += rowh;
                    rowh = 0;
                    ox = 0;
                }

                glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
                _glyphs[i].advance = cgt::vec2(static_cast<float>(g->advance.x >> 6), static_cast<float>(g->advance.y >> 6));
                _glyphs[i].bitmapSize = cgt::vec2(static_cast<float>(g->bitmap.width), static_cast<float>(g->bitmap.rows));
                _glyphs[i].bitmapOffset = cgt::vec2(static_cast<float>(g->bitmap_left), static_cast<float>(g->bitmap_top));
                _glyphs[i].offset = cgt::vec2(ox / static_cast<float>(w), oy / static_cast<float>(h));
                rowh = std::max(rowh, static_cast<int>(g->bitmap.rows));
                ox += g->bitmap.width + 1;
            }

            LDEBUG("Generated a " << w << " x "<< h << " texture atlas");
        
            _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/fontrendering/glsl/fontatlas.frag", "");
            _shader->setAttributeLocation(0, "in_Position");
            _shader->setAttributeLocation(1, "in_TexCoord");
            LGL_ERROR;
        }

        FontAtlas::~FontAtlas() {
            FT_Done_FreeType(_ftLibrary);

            delete _texture;
            ShdrMgr.dispose(_shader);
        }

        void FontAtlas::renderText(const std::string& text, const cgt::vec2& position, const cgt::vec4& color, const cgt::mat4& transformationMatrix /*= cgt::mat4::identity*/) {
            if (_shader == nullptr || _texture == nullptr) {
                LERROR("Cannot render text, FontAtlas not correctly initialized.");
                return;
            }

            cgt::TextureUnit fontUnit;
            fontUnit.activate();
            _texture->bind();

            _shader->activate();
            _shader->setUniform("_fontTexture", fontUnit.getUnitNumber());
            _shader->setUniform("_color", color);
            _shader->setUniform("_viewMatrix", transformationMatrix);

            // create billboard vertices
            std::vector<cgt::vec3> vertices;
            std::vector<cgt::vec3> texCoords;
            vertices.reserve(6 * text.length());
            texCoords.reserve(6 * text.length());

            cgt::vec2 pos = position;

            // Loop through all characters
            for (size_t i = 0; i < text.length(); ++i) {
                unsigned char p = text[i];

                // Hanlde "\n"
                if (p == 10) {
                    pos = cgt::vec2(position.x, pos.y - _height * 1.2f);
                }

                if (p >= 32 && p < 256) {
                    cgt::vec2 pos2(pos.x + _glyphs[p].bitmapOffset.x, -pos.y - _glyphs[p].bitmapOffset.y);
                    cgt::vec2 size = _glyphs[p].bitmapSize;

                    // Advance the cursor to the start of the next character
                    pos += _glyphs[p].advance;

                    // Skip glyphs that have no pixels
                    if (size.x == 0.f || size.y == 0.f)
                        continue;

                    const float tw = static_cast<float>(_texture->getWidth());
                    const float th = static_cast<float>(_texture->getHeight());

                    // Calculate the vertex and texture coordinates
                    vertices.push_back(cgt::vec3(pos2.x, -pos2.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset, 0.f));

                    vertices.push_back(cgt::vec3(pos2.x + size.x, -pos2.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset.x + _glyphs[p].bitmapSize.x / tw, _glyphs[p].offset.y, 0.f));

                    vertices.push_back(cgt::vec3(pos2.x, -pos2.y - size.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset.x, _glyphs[p].offset.y + _glyphs[p].bitmapSize.y / th, 0.f));


                    vertices.push_back(cgt::vec3(pos2.x + size.x, -pos2.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset.x + _glyphs[p].bitmapSize.x / tw, _glyphs[p].offset.y, 0.f));

                    vertices.push_back(cgt::vec3(pos2.x, -pos2.y - size.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset.x, _glyphs[p].offset.y + _glyphs[p].bitmapSize.y / th, 0.f));

                    vertices.push_back(cgt::vec3(pos2.x + size.x, -pos2.y - size.y, 0.f));
                    texCoords.push_back(cgt::vec3(_glyphs[p].offset.x + _glyphs[p].bitmapSize.x / tw, _glyphs[p].offset.y + _glyphs[p].bitmapSize.y / th, 0.f));
                }                
            }

            FaceGeometry face(vertices, texCoords);
            face.render(GL_TRIANGLES);
            _shader->deactivate();
            LGL_ERROR;
        }

        void FontAtlas::renderTextWithPickingInformation(const std::string& text, const cgt::vec2& position, const cgt::vec4& color, const cgt::vec4& pickingInfo, const cgt::mat4& transformationMatrix /*= cgt::mat4::identity*/) {
            _shader->activate();
            _shader->setUniform("_pickingColor", pickingInfo);
            _shader->deactivate();

            renderText(text, position, color, transformationMatrix);
        }

    }
}
