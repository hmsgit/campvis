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

#ifndef FONTATLAS_H__
#define FONTATLAS_H__

#include "cgt/matrix.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/vector.h"

#include <string>

// forward declarations
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;

namespace campvis {
namespace fontrendering {

    /**
     * A font atlas providing to render text with OpenGL.
     * The FontAtlas manages a set of glyphs for each printable character of a font with given size
     * at construction. You can then use renderText() to render any text with that font into the
     * current OpenGL framebuffer.
     * 
     * ATTENTION:   Throws an exception when it cannot initialiize the font atlas (e.g. due to 
     *              missing font file).
     * 
     * Code inspired by http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02
     * 
     * \note    Current implementation only supports the ASCII-256 character set.
     * \throws  std::exception on initialization error
     */
    class FontAtlas {
    public:
        /**
         * Creates a new FontAtlas of the given font and size.
         * \note    Needs a valid OpenGL context.
         * \param   fontFileName    Filename of the font to use for rendering.
         * \param   height          Font height in pixels. This is in viewport coordinates, i.e. \b after applying the optional transformation matrix in renderText()!
         */
        FontAtlas(const std::string& fontFileName, int height);

        /**
         * Destructor of the FontAtlas, needs a valid OpenGL context.
         */
        ~FontAtlas();


        /**
         * Renders the given text with the given parameters into the current framebuffer.
         * \param   text        The text to render (currently only ASCII-256 support).
         * \param   position    Position where to start rendering the text (in normalized [-1, 1] coordinates).
         * \param   color       Color of the text to render.
         * \param   scale       Additional transformation matrix that is applied to each rendered vertex.
         */
        void renderText(const std::string& text, const cgt::vec2& position, const cgt::vec4& color, const cgt::mat4& transformationMatrix = cgt::mat4::identity);

    private:
        /// Struct storing glyph parameters of each printable ASCII character
        struct {
            cgt::vec2 advance;          ///< Advance of this glyph
            cgt::vec2 bitmapSize;       ///< Bitmap size of this glyph
            cgt::vec2 bitmapOffset;     ///< Bitmap offset of this glyph
            cgt::vec2 offset;           ///< Offset of this glyph in texture coordinates
        } _glyphs[256];

        FT_Library _ftLibrary;          ///< Pointer to the freetype2 library handle
        FT_Face _ftFace;                ///< Pointer to the font face
        cgt::Texture* _texture;         ///< OpenGL texture storing the texture atlas
        cgt::Shader* _shader;           ///< Shader used for font rendering

        static const int MAX_TEXTURE_WIDTH;
        static const std::string loggerCat_;
    };

}
}

#endif // FONTATLAS_H__
