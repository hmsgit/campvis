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

#ifndef QTTHREADEDCANVAS_H__
#define QTTHREADEDCANVAS_H__

#include "cgt/qt/qtcanvas.h"
#include "cgt/types.h"

namespace cgt {

    /**
     * Subclass of QtCanvas that supports multi-threading.
     */
    class CGT_API QtThreadedCanvas : public cgt::QtCanvas {
    public:
        QtThreadedCanvas(
            const std::string& title = "",
            const cgt::ivec2& size = cgt::ivec2(cgt::GLCanvas::DEFAULT_WINDOW_WIDTH, cgt::GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const cgt::GLCanvas::Buffers buffers = cgt::GLCanvas::RGBADD,
            QWidget* parent = 0, bool shared = true, Qt::WFlags f = 0, char* name = 0);

        virtual ~QtThreadedCanvas();

        // override Qt events so that they don't interfere with the threading.
        void resizeEvent(QResizeEvent *event) {
            sizeChanged(ivec2(event->size().width(), event->size().height()));
        }

        /**
         * If you manually want to cause a paint-event, use this function. It will call paintGL()
         * via updateGL(). This will cause immediate repainting.
         */
        virtual void repaint();

    protected:
        // override Qt events so that they don't interfere with the threading.
        virtual void paintEvent(QPaintEvent *event);

        // override the paint method so that it doesn't interfere with the threading.
        virtual void paint();

    };

}

#endif // QTTHREADEDCANVAS_H__
