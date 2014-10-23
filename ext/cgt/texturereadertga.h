/**********************************************************************
 *                                                                    *
 * cgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
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

#ifndef CGT_TEXTUREREADERTGA_H
#define CGT_TEXTUREREADERTGA_H

#include "cgt/texturereader.h"

namespace cgt {

///Texturereader to read compressed and uncompressed RGB and RGBA tga files.
///No external dependencies!
class CGT_API TextureReaderTga : public TextureReader {
    static const std::string loggerCat_;
public:
    TextureReaderTga();
    virtual Texture* loadTexture(const std::string& filename, Texture::Filter filter, bool compress = false,
                                 bool keepPixels = false, bool createOGLTex = true, bool textureRectangle = false);
};


} // namespace cgt

#endif // CGT_TEXTUREREADERTGA_H
