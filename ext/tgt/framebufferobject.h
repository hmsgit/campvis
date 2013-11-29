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

#ifndef TGT_FRAMEBUFFEROBJECT_H
#define TGT_FRAMEBUFFEROBJECT_H

/// if you run out of FBO color attachments, edit this value o_O
#define TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS 6

#include "tgt/assert.h"
#include "tgt/texture.h"
#include "tgt/types.h"

#include <map>

namespace tgt {

class TGT_API FramebufferObject {
public:
    FramebufferObject();
    virtual ~FramebufferObject();

    void activate();
    static void deactivate();

    bool isComplete() const;

    bool isActive() const;

    /// Bind a texture to the "attachment" point of this FBO
    void attachTexture(Texture* texture,
                       GLenum attachment = GL_COLOR_ATTACHMENT0,
                       int mipLevel      = 0,
                       int zSlice        = 0);

    void detachTexture(GLenum attachment);

    void detachAll();

    Texture* getTextureAtAttachment(GLenum attachment);

    static GLuint getActiveObject();

    GLuint getId() const { return id_; };

    Texture* const * getAttachments() const;

    size_t getNumColorAttachments() const;

    const Texture* getColorAttachment(size_t index = 0) const;
    const Texture* getDepthAttachment() const;
    const Texture* getStencilAttachment() const;

protected:
    GLuint generateId();

    GLuint id_;

    size_t decodeAttachment(GLenum attachment) {
        switch (attachment) {
            case GL_DEPTH_ATTACHMENT:
                return TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS;
            case GL_STENCIL_ATTACHMENT:
                return TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS + 1;
            default:
                tgtAssert((attachment - GL_COLOR_ATTACHMENT0) < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS, "Color attachments out of bounds - adjust TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS define!");
                return attachment - GL_COLOR_ATTACHMENT0;
        }
    }

    union {
        struct {
            Texture* colorAttachments_[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS];
            Texture* depthAttachment_;
            Texture* stencilAttachment_;
        };

        Texture* attachments_[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS+2];
    };

    size_t numColorAttachments_;
    static const std::string loggerCat_; ///< category used in logging
};

} // namespace tgt

#endif // TGT_FRAMEBUFFEROBJECT_H
