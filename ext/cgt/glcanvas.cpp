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

#include "cgt/glcanvas.h"
#include "cgt/glcontextmanager.h"
#include "cgt/painter.h"

#include <cstdlib>
#include <stdio.h>

namespace cgt {

GLCanvas::GLCanvas(const std::string& title,
                   const ivec2& size,
                   const GLCanvas::Buffers buffers )
    : title_(title),
      size_(size),
      buffers_(buffers),
      rgbaSize_(0, 0, 0, 0),
      depthSize_(0),
      stencilSize_(0),
      doubleBuffered_((buffers & DOUBLE_BUFFER) != 0),
      stereoViewing_(false),
      fullscreen_(false),
      autoFlush_(true),
      painter_(0) ,
      eventHandler_(new EventHandler())
{}

GLCanvas::~GLCanvas() {
    eventHandler_->clearEventListeners();
    delete eventHandler_;
}

void GLCanvas::sizeChanged(const ivec2& size) {
    size_ = size;
    if (painter_)
        painter_->sizeChanged(size);
}

void GLCanvas::paint() {
    GLContextScopedLock lock(this);
    if (painter_)
        painter_->repaint();
    if (autoFlush_) {
        if (doubleBuffered_) {
            swap();
        } else {
            glFlush();
        }
    }
}

void GLCanvas::init() {
    // nothing here, may be implemented in sub classes.
}

void GLCanvas::setPainter(Painter* p) {
    painter_ = p;
    painter_->setCanvas(this);
}

Painter* GLCanvas::getPainter() const {
    return painter_;
}

EventHandler* GLCanvas::getEventHandler() const {
    return eventHandler_;
}
void GLCanvas::setEventHandler(EventHandler* handler){
    delete eventHandler_;
    eventHandler_ = handler;
}

bool GLCanvas::getAutoFlush() {
 return autoFlush_;
}

const ivec4& GLCanvas::getRgbaSize() const {
    return rgbaSize_;
}

int GLCanvas::getDepthSize() const {
    return depthSize_;
}

int GLCanvas::getStencilSize() const {
    return stencilSize_;
}

ivec2 GLCanvas::getSize() const {
    return size_;
}

int GLCanvas::getWidth() const {
    return size_.x;
}

int GLCanvas::getHeight() const {
    return size_.y;
}

GLCanvas::Buffers GLCanvas::getBuffers() const {
    return buffers_;
}

} // namespace
