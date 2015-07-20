/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
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

#include "qtthreadedcanvas.h"
#include "cgt/assert.h"
#include "cgt/painter.h"

namespace cgt {
    
    QtThreadedCanvas::QtThreadedCanvas(const std::string& title /*= ""*/, const ivec2& size /*= ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= GLCanvas::RGBADD*/, QWidget* parent /*= 0*/, bool shared /*= false*/, Qt::WFlags f /*= 0*/, char* name /*= 0*/)
        : QtCanvas(title, size, buffers, parent, shared, f, name)
    {
    }

    QtThreadedCanvas::~QtThreadedCanvas() {

    }

    void QtThreadedCanvas::repaint() {
        // skip QtCanvas's repaint implementation and generate a paint event with QWidget's repaint instead
        QWidget::repaint();
    }

    void QtThreadedCanvas::paint() {
        // all painting done in threaded painter
    }

    void QtThreadedCanvas::paintEvent(QPaintEvent* /* event */) {
        if (painter_ != nullptr)
            painter_->repaint();
    }

}
