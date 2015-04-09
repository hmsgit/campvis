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

#ifndef CGT_TEXTURE_H
#define CGT_TEXTURE_H

#include <string>
#include "cgt/cgt_gl.h"
#include "cgt/types.h"
#include "cgt/vector.h"

namespace cgt {

/**
 * OpenGL Texture
 */
class CGT_API Texture {
public:
    friend class TextureManager;
    friend class TextureReader;

    enum Filter {
        NEAREST,
        LINEAR,
        MIPMAP,
        ANISOTROPIC
    };

    enum Wrapping {
        REPEAT = GL_REPEAT,
        CLAMP = GL_CLAMP,
        CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
        CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
        MIRRORED_REPEAT = GL_MIRRORED_REPEAT
    };

    /**
     * Creates an empty texture with the given parameters.
     * 
     * \param   type            OpenGL texture type
     * \param   dimensions      Size of the texture
     * \param   internalFormat  Internal storage format
     * \param   filter          Filter to apply during sampling (defaults to linear)
     */
    Texture(GLenum type, const cgt::ivec3& dimensions, GLint internalFormat, Filter filter = LINEAR);

    /**
     * Creates a new OpenGL texture and initializes it with the given data.
     * \note    Works the same as constructing with the non-uploading ctor and then 
     *          calling uploadTexture(data, format, dataType).
     * \param type              OpenGL texture type
     * \param dimensions        Size of the texture
     * \param internalformat    Internal storage format
     * \param data              Pointer to data to upload, may be 0, but then you could use the other ctor directly...
     * \param format            OpenGL Format of the data in \a data.
     * \param dataType          OpenGL data type of the data in \a data.
     * \param filter            Filter to apply during sampling (defaults to linear)
     */
    Texture(GLenum type, const cgt::ivec3& dimensions, GLint internalFormat, 
            GLubyte* data, GLint format, GLenum dataType, Filter filter = LINEAR);

    /**
    * The destructor deletes the Texture in OpenGL.
    * Handled by the texturemanager!
    */
    virtual ~Texture();


    /// calculates the bytes per pixel from format dataType and dataType
    static int calcBpp(GLint format, GLenum dataType);

    /// calculates the bytes per pixel from the internal format
    static int calcBpp(GLint internalformat);

    /// calculates the number of channels from the passed internal format
    static int calcNumChannels(GLint internalFormat);

    static GLint calcMatchingFormat(GLint internalFormat);
    static GLenum calcMatchingDataType(GLint internalFormat);

    /**
     * Determines the best-matching internal format for the given OpenGL format and data type
     * \param   format      OpenGL Format of the data
     * \param   dataType    OpenGL data type of the data
     */
    static GLint calcInternalFormat(GLint format, GLenum dataType);

    /// calculates size on the GPU (using internalformat)
    int getSizeOnGPU() const;

    /**
     * Bind the texture to the active texture unit and target.
     *
     * Note: This does not enable texturing (use enable()).
     */
    void bind() const {
        glBindTexture(type_ , id_);
    }

    /**
     * unbind the current texture from the active texture unit and target.
     */
    void unbind() const {
        glBindTexture(type_, 0);
    }


    /**
     *   Return OpenGL texture ID
     */
    GLuint getId() const { return id_; }

    /**
     *   Generate OpenGL texture ID
     *   @return The generated ID
     */
    GLuint generateId() {
        id_ = 0;
        glGenTextures(1, &id_);
        return id_;
    }

    std::string getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    /**
    *   Returns type of the texture (GL_TEXTURE_1D...3D, GL_TEXTURE_RECTANGLE_ARB)
    */
    GLenum getType() const { return type_; }

    cgt::ivec3 getDimensions() const { return dimensions_;}
    int getWidth() const { return dimensions_.x; };
    int getHeight() const { return dimensions_.y; }
    int getDepth() const { return dimensions_.z; }
    GLint getInternalFormat() const { return internalformat_; }
    Filter getFilter() const { return filter_; }
    size_t getNumChannels() const { return calcNumChannels(internalformat_); }


    /**
    *   Returns the Bytes Per Pixel used,
    *   usually 3 or 4 for 24/32 bits per pixel.
    */
    GLubyte getBpp() const { return bpp_; }

    /**
    *   Set Priority of this texture in GL
    *   @param p Priority, [0...1]
    */
    void setPriority(GLclampf p);

    /**
    *   Check if texture is in resident GL memory
    */
    bool isResident() const;

    /**
     *   Sets Filtering for Texture. Binds the texture.
     */
    void setFilter(Filter filter);

    /**
     * Applies the stored filter once again. Binds the texture.
     */
    void applyFilter();

    /// Set texture wrapping mode. Binds the texture.
    void setWrapping(Wrapping w);

    /// Applies the textur wrapping mode once againg. Binds the texture.
    void applyWrapping();

    Wrapping getWrapping() const { return wrapping_; }

    /**
     * Upload the given data to the texture. Binds the texture.
     *
     * \param data              Pointer to data to upload, may be 0, but then this is a NOP.
     * \param format            OpenGL Format of the data in \a data.
     * \param dataType          OpenGL data type of the data in \a data.
     */
    void uploadTexture(const GLubyte* data, GLint format, GLenum dataType);
    
    /**
     * Download texture from the GPU to a newly allocated buffer with
     * the passed format/data type and the texture's dimensions.
     */
    GLubyte* downloadTextureToBuffer(GLint format, GLenum dataType) const;

    /**
     * Returns, whether texture is a depth texture.
     * \return  internalformat_ == GL_DEPTH_COMPONENT
     */
    bool isDepthTexture() const;

protected:
    cgt::ivec3 dimensions_;
    GLint internalformat_;  ///< GL_RGB...
    Filter filter_;
    Wrapping wrapping_;

    GLuint id_;             ///< OpenGL texture id

    GLenum type_;           ///< 1D, 2D, 3D
    GLubyte bpp_;           ///< bytes per pixel

    std::string name_;      ///< optional, e.g. for storing texture file name

    // used internally in the constructors
    void init();
};

} // namespace cgt

#endif // CGT_TEXTURE_H
