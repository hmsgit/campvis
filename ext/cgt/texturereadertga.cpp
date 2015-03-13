/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
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

#include "cgt/texturereadertga.h"

#include "cgt/logmanager.h"
#include "cgt/filesystem.h"

#include <cstring>

namespace cgt {

//------------------------------------------------------------------------------
// TextureReaderTga
//------------------------------------------------------------------------------

const std::string TextureReaderTga::loggerCat_("cgt.Texture.Reader.Tga");

TextureReaderTga::TextureReaderTga() {
    name_ = "TGA Reader";
    extensions_.push_back("tga");
}

Texture* TextureReaderTga::loadTexture(const std::string& filename, Texture::Filter filter,
                                       bool compress, bool keepPixels, bool createOGLTex,
                                       bool textureRectangle)
{

    GLubyte TGAheader[12];
    GLubyte header[6];
//    GLuint  bytesPerPixel;//FIXME: this is never initialized!

    File* file = FileSys.open(filename);

    // Check if file is open
    if (!file) {
        LERROR("Failed to open file " << filename);
        return 0;
    }

    if (!file->isOpen()) {
        LERROR("Failed to open file " << filename);
        delete file;
        return 0;
    }

    size_t len = file->size();

    // check if file is empty
    if (len == 0) {
        delete file;
        return 0;
    }

    if (file->read(reinterpret_cast<char*>(&TGAheader), sizeof(TGAheader)) != sizeof(TGAheader) ||
        file->read(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
    {
        delete file;
        LERROR("Failed to read header! file: " << filename);
        return 0;
    }

    ivec3 dimensions;
    dimensions.x = header[1] * 256 + header[0];           // determine the TGA width  (highbyte*256+lowbyte)
    dimensions.y = header[3] * 256 + header[2];           // determine the TGA height (highbyte*256+lowbyte)
    dimensions.z = 1;
    LDEBUG("Image dimensions: " << dimensions);

    if (dimensions.x <= 0 || dimensions.y <= 0) {
        delete file;
        LERROR("wrong dimensions: " << dimensions << " file: " << filename);
        return 0;
    }

    if (header[4] != 24 && header[4] != 32) {
        delete file;
        LERROR("Illegal bpp! file: " << filename);
        return 0;
    }

    size_t bpp = static_cast<size_t>(header[4]);
    bpp /= 8;  // divide by 8 to get the bytes per pixel

    GLint format = GL_RGBA;
    switch (bpp) {
        case 1:
            format = GL_RED;
            LDEBUG("GL_RED");
            break;
        case 3:
            format = GL_RGB;
            LDEBUG("RGB");
            break;
        case 4:
            format = GL_RGBA;
            LDEBUG("RGBA");
            break;

        default:
            LERROR("unsupported bpp " << filename);
            break;
    }

    GLenum dataType = GL_UNSIGNED_BYTE;

    size_t numBytes = static_cast<size_t>(bpp * cgt::hmul(dimensions));
    GLubyte* buffer = new GLubyte[numBytes];

    if (TGAheader[2] == 2) {
        // file is not compressed
        LDEBUG("Reading uncompressed TGA file...");
        if (file->read(buffer, numBytes) != numBytes) {
            LERROR("Failed to read uncompressed image! file: " << filename);
            delete file;
            delete [] buffer;
            return 0;
        }
    } 
    else {
        // file is compressed
        LDEBUG("Reading compressed TGA file " << filename << " ...");

        //TODO: error handling
        unsigned char chunk[4];
        unsigned char* at = buffer;
        for (size_t bytesDone = 0; bytesDone < numBytes; /* nothing here */) {
            unsigned char packetHead;
            file->read(reinterpret_cast<char*>(&packetHead), 1);
            if (packetHead > 128) {
                //RLE
                packetHead -= 127;
                file->read(reinterpret_cast<char*>(&chunk), bpp);
                for (unsigned char b=0; b < packetHead; b++) {
                        std::memcpy(at, chunk, bpp);
                        bytesDone += bpp;
                        at += bpp;
                }
            } else {
                //RAW
                packetHead++;
                file->read(reinterpret_cast<char*>(at), bpp * packetHead);
                bytesDone += packetHead * bpp;
                at += packetHead * bpp;
            }
        }
    }

    file->close();
    delete file;

    // switch r & b
    if (bpp >= 3) {
        GLubyte* at = buffer;
        while (at - buffer < static_cast<ptrdiff_t>(numBytes)) {
            std::swap(at[0], at[2]);
            at += bpp;
        }
    }

    Texture* t = new Texture(GL_TEXTURE_2D, dimensions, Texture::calcInternalFormat(format, dataType), filter);
    t->uploadTexture(buffer, format, dataType);
    t->setName(filename);

    delete [] buffer;
    return t;
}

Texture* TextureReaderTga::loadTextureArray(const std::vector<std::string>& filenames, Texture::Filter filter) {
    GLubyte* data = nullptr;
    GLint format;
    GLint internalFormat;
    GLenum dataType = GL_UNSIGNED_BYTE;
    size_t numBytesPerTexture = 0;
    cgt::ivec3 dimensions;

    // quick and dirty hack with bad performance (unnecessary copies)
    for (size_t i = 0; i < filenames.size(); ++i) {
        Texture* tmp = loadTexture(filenames[i], filter);

        if (data == nullptr) {
            switch (tmp->getNumChannels()) {
                case 1:
                    format = GL_RED;
                    LDEBUG("GL_RED");
                    break;
                case 3:
                    format = GL_RGB;
                    LDEBUG("RGB");
                    break;
                case 4:
                    format = GL_RGBA;
                    LDEBUG("RGBA");
                    break;
                default:
                    cgtAssert(false, "Should not reach this! Wrong number of channels.");
                    break;
            }

            internalFormat = tmp->getInternalFormat();
            dimensions = tmp->getDimensions();
            numBytesPerTexture = cgt::hmul(dimensions) * tmp->getNumChannels();
            data = new GLubyte[filenames.size() * numBytesPerTexture];
        }
        else {
            if (dimensions != tmp->getDimensions() || internalFormat != tmp->getInternalFormat()) {
                LERROR("Dimensions of texture array textures or internal formats mismatch, aborting!");
                delete [] data;
                delete tmp;
                return nullptr;
            }
        }

        GLubyte* buffer = tmp->downloadTextureToBuffer(format, dataType);
        memcpy(data + (i * numBytesPerTexture), buffer, numBytesPerTexture);
        delete [] buffer;
        delete tmp;
    }

    Texture* toReturn = new Texture(GL_TEXTURE_2D_ARRAY, cgt::ivec3(dimensions.xy(), int(filenames.size())), internalFormat, data, format, dataType, filter);
    delete data;
    return toReturn;
}

} // namespace cgt
