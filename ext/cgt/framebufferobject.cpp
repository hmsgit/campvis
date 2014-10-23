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

#include "cgt/framebufferobject.h"
#include "cgt/logmanager.h"
#include "cgt/openglgarbagecollector.h"

namespace tgt {

const std::string FramebufferObject::loggerCat_("tgt.FramebufferObject");

FramebufferObject::FramebufferObject()
  : id_(0)
  , numColorAttachments_(0)
{
    memset(attachments_, 0, sizeof(Texture*) * (TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS+2));
    generateId();
}

FramebufferObject::~FramebufferObject()
{
    GLGC.addGarbageFramebufferObject(id_);
}

void FramebufferObject::activate()
{
#if TGT_DEBUG
    if (tgt::getGlInt(GL_FRAMEBUFFER_BINDING) != 0)
        LWARNING("Binding a new FBO while another FBO is bound. Do you really want to do this?");
#endif

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id_);
    LGL_ERROR;
}

void FramebufferObject::deactivate()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    LGL_ERROR;
}

void FramebufferObject::attachTexture(Texture* texture, GLenum attachment, int mipLevel, int zSlice)
{
    switch(texture->getType()) {
        case GL_TEXTURE_1D:
            glFramebufferTexture1DEXT( GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_1D, texture->getId(), mipLevel );
            break;
        case GL_TEXTURE_3D:
            glFramebufferTexture3DEXT( GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_3D, texture->getId(), mipLevel, zSlice );
            break;
        case GL_TEXTURE_2D_ARRAY:
            glFramebufferTextureLayerEXT( GL_FRAMEBUFFER_EXT, attachment, texture->getId(), mipLevel, zSlice );
            break;
        default: //GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE
            glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, attachment, texture->getType(), texture->getId(), mipLevel );
            break;
    }
    LGL_ERROR;

    size_t index = decodeAttachment(attachment);
    attachments_[index] = texture;
    if (index < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS)
        ++numColorAttachments_;

#ifdef CAMPVIS_DEBUG
    this->isComplete();
#endif
}

Texture* FramebufferObject::getTextureAtAttachment(GLenum attachment) {
    return attachments_[decodeAttachment(attachment)];
}

void FramebufferObject::detachTexture(GLenum attachment) {
    size_t index = decodeAttachment(attachment);
    if (attachments_[index] != 0) {
        switch (attachments_[index]->getType()) {
            case GL_TEXTURE_1D:
                glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_1D, 0, 0);
                break;
            case GL_TEXTURE_2D_ARRAY:
                glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, attachment, 0, 0, 0);
                break;
            case GL_TEXTURE_3D:
                glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_3D, 0, 0, 0);
                break;
            default: // GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, 0, 0);
                break;
        }
        attachments_[index] = 0;
        LGL_ERROR;

        if (index < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS)
            --numColorAttachments_;
    }
    else {
        LWARNING("Trying to detach unknown texture!");
    }
}

void FramebufferObject::detachAll() {
    for (GLenum i = 0; i < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS; ++i) {
        if (colorAttachments_[i] != 0)
            detachTexture(GL_COLOR_ATTACHMENT0 + i);
    }
    if (depthAttachment_ != 0)
        detachTexture(GL_DEPTH_ATTACHMENT);
    if (stencilAttachment_ != 0)
        detachTexture(GL_STENCIL_ATTACHMENT);

    numColorAttachments_ = 0;
}

bool FramebufferObject::isComplete() const
{
  bool complete = false;

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
  switch(status) {
      case GL_FRAMEBUFFER_COMPLETE_EXT:
          complete = true;
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_ATTACHMENT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_MISSING_ATTACHMENT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_DIMENSIONS");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_FORMATS");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_DRAW_BUFFER");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_INCOMPLETE_READ_BUFFER");
          break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
          LERROR("GL_FRAMEBUFFER_EXT_UNSUPPORTED");
          break;
      default:
          LERROR("Unknown error!");
  }
  return complete;
}

bool FramebufferObject::isActive() const {
    return ((getActiveObject() == id_) && (id_ != 0));
}

GLuint FramebufferObject::getActiveObject() {
    GLint fbo;
    glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &fbo);
    return static_cast<GLuint>(fbo);
}

GLuint FramebufferObject::generateId() {
    id_ = 0;
    glGenFramebuffersEXT(1, &id_);
    LGL_ERROR;
    return id_;
}

Texture* const * FramebufferObject::getAttachments() const {
    return attachments_;
}

const Texture* FramebufferObject::getColorAttachment(size_t index /*= 0*/) const {
    tgtAssert(index < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS, "Index out of bounds!");
    return attachments_[index];
}

const Texture* FramebufferObject::getDepthAttachment() const {
    return attachments_[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS];
}

const Texture* FramebufferObject::getStencilAttachment() const {
    return attachments_[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS+1];
}

size_t FramebufferObject::getNumColorAttachments() const {
    return numColorAttachments_;
}

} // namespace
