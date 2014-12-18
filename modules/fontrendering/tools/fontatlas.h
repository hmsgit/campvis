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

#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/vector.h"

#include <string>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;

namespace campvis {
namespace fontrendering {

    /**
     * A font atlas, inspired by http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02
     */
    class FontAtlas {
    public:
        FontAtlas(const std::string& fontFileName, int height);

        ~FontAtlas();


        void renderText(const std::string& text, const cgt::vec2& position, const cgt::vec4& color, const cgt::vec2& scale = cgt::vec2(1.f));

    private:
        /// Struct storing glyph parameters of each printable ASCII character
        struct {
            cgt::vec2 advance;          ///< Advance of this glyph
            cgt::vec2 bitmapSize;       ///< Bitmap size of this glyph
            cgt::vec2 bitmapOffset;     ///< Bitmap offset of this glyph
            cgt::vec2 offset;           ///< Offset of this glyph in texture coordinates
        } _glyphParameters[128];

        FT_Library _ftLibrary;
        FT_Face _ftFace;
        cgt::Texture* _texture;
        cgt::Shader* _shader;

        static const int MAX_TEXTURE_WIDTH;
        static const std::string loggerCat_;
    };

}
}

#endif // FONTATLAS_H__
