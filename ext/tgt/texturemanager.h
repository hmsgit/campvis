/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_TEXTUREMANAGER_H
#define TGT_TEXTUREMANAGER_H

#include <set>

#include "tgt/tgt_gl.h"
#include "tgt/manager.h"
#include "tgt/vector.h"
#include "tgt/singleton.h"
#include "tgt/texture.h"
#include "tgt/types.h"

namespace tgt {

class TextureReader;

class ContainerFile;

class TextureManager;
#ifdef DLL_TEMPLATE_INST
template class TGT_API ResourceManager<Texture>;
#endif
#ifdef DLL_TEMPLATE_INST
template class TGT_API Singleton<TextureManager>;
#endif

/**
*   Texture Manager
*/
class TGT_API TextureManager : public ResourceManager<Texture>, public Singleton<TextureManager> {
private:
    static const std::string loggerCat_;

    std::map<std::string, TextureReader*> readers_;
    std::set<TextureReader*> readerSet_; // for easy destruction

    std::string getEnding(const std::string& filename) const;

    ContainerFile* container_;

public:

    /**
     *   Init texturemanager.
     */
    TextureManager();
    virtual ~TextureManager();

    /**
    *   Register TextureReader for use in Manager
    */
    void registerReader(TextureReader* r);

    /**
    *   Loads a texture from file.
    *   @param filename Texture Filename
    *   @param filter Texture filter to be used for this texture (NEAREST, LINEAR, MIPMAP, ANISOTROPIC)
    *   @param compress Should texture-compression be used on this texture (only if available)
    *   @param keepPixels Should the pixels be kept in memory after creating the texture?
    *   @param createOGLTex Create a OpenGL texture from data?
    *   @param useCache Check if texture is already loaded and use old texture?
    *   @param textureRectangle Should texture be loaded as texture rectangle (extension GL_ARB_texture_rectangle)?
            (use only if available!)
    */
    Texture* load(const std::string& filename, Texture::Filter filter = Texture::LINEAR, bool compress = false,
        bool keepPixels = false, bool createOGLTex = true, bool useCache = true);

    /**
    *   The same as load(...), but without using the intern path
    *   @param completeFilename Texture Filename (with the path)
    *   @param filter Texture filter to be used for this texture (NEAREST, LINEAR, MIPMAP, ANISOTROPIC)
    *   @param compress Should texture-compression be used on this texture (only if available)
    *   @param keepPixels Should the pixels be kept in memory after creating the texture?
    *   @param createOGLTex Create a OpenGL texture from data?
    *   @param useCache Check if texture is already loaded and use old texture?
    *   @param textureRectangle Should texture be loaded as texture rectangle (extension GL_ARB_texture_rectangle)?
            (use only if available!)
    */
    Texture* loadIgnorePath(const std::string& completeFilename, Texture::Filter filter = Texture::LINEAR,
        bool compress = false, bool keepPixels = false, bool createOGLTex = true,
        bool useCache = true);

    /**
    *   Loads a texture from supplied pointer.
    *   supports only 16bit 3D textures for now.
    *   @param filter Texture filter to be used for this texture (NEAREST, LINEAR, MIPMAP, ANISOTROPIC)
    *   @param compress Should texture-compression be used on this texture (only if available)
    */
    Texture* loadFromMemory(Texture* t, Texture::Filter filter = Texture::LINEAR,
                            bool compress = false, bool createOGLTex = true);

    void setContainer(ContainerFile* container) { container_ = container; }
};


} // namespace tgt

#define TexMgr tgt::Singleton<tgt::TextureManager>::getRef()

#endif //TGT_TEXTUREMANAGER_H
