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

#ifndef CGT_QTCANVAS_H
#define CGT_QTCANVAS_H

#include "cgt/glcanvas.h"
#include "cgt/types.h"

#include <QGLWidget>
#include <QGLFormat>
#include <QMouseEvent>
#include <QKeyEvent>

#include <deque>

namespace cgt {

/**
 * Qt implementation of GLCanvas. Inherits QGLWidget and combines the Qt methods and cgt methods.
 */
class CGT_API QtCanvas : public GLCanvas, public QGLWidget {
public:
    /**
     * The constructor. Allows the user to specify a shared widget that this canvas will share
     * the OpenGL context with. Also, it is possible to specify whether or not a custom
     * event-loop has to be used.
     *
     * @param parent The parent widget of this canvas.
     * @param shared If this is true, this canvas will share its OpenGL context with the static \a shareWidget_.
     * @param f Qt::Wflags can be passed to this constructor to control the qt features, like stereo-buffering.
     * @param useCustomEventloop Are the incoming events supposed to be handled by the standard callbacks or not?
     * @param name A name can be passed for debugging purposes.
     */
    QtCanvas(const std::string& title = "",
             const ivec2& size = ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT),
             const Buffers buffers = RGBADD,
             QWidget* parent = 0, bool shared = false, Qt::WFlags f = 0, char* name = 0);

    QtCanvas(QWidget* parent, bool shared = false, Qt::WFlags f = 0, char* name = 0);

    /**
     * Destructor. Closes window (if canvas is a window).
     */
    virtual ~QtCanvas();


    /**
     * Called by the Qt framework once at the beginning, before rendering starts.
     * This function calls the corresponding GLCanvas method \a initialize.
     */
    virtual void initializeGL();

    /**
     * This is called by the Qt framework every time the canvas is resized.
     * This function calls the corresponding GLCanvas method \a sizeChanged.
     *
     * @param w The new width of the canvas.
     * @param h The new height of the canvas.
     */
    virtual void resizeGL(int w, int h);

    /*
     * Return the recommended size for the canvas.
     *
     * This method is called by Qt to determine how much space to allocate to display the canvas.
     * The returned size is the same as reported by getSize().
     */
    virtual QSize sizeHint() const;

    virtual void setSize(ivec2 newSize);

    /**
     * Called by Qt if there is a paint event; it uses the \a painter_ to paint() something.
     */
    virtual void paintGL();

    /**
     * If you manually want to cause a paint-event, use this function. It will call paintGL()
     * via updateGL(). This will cause immediate repainting.
     */
    virtual void repaint();

    /**
     * If you manually want to cause a paint-event, use this function. It will call QWidget::update()
     * and repaint when entering main loop next time.
     */
    virtual void update();


    /// Acqures this canvas as current context
    virtual void acquireAsCurrentContext();
    /// Releases this canvas as current context
    virtual void releaseAsCurrentContext();

    /// swap buffers
    virtual void swap();

    virtual void toggleFullScreen();

    virtual void enterEvent(QEvent* e);
    virtual void leaveEvent(QEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent (QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent*  e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void timerEvent(QTimerEvent* e);

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

    ///
    /// Helpers used to generate cgt-Events out of qt-Events
    ///

    // map one Qt-mousebutton to one cgt-mousebutton
    static cgt::MouseEvent::MouseButtons getButton(QMouseEvent* e);
    // map a set of Qt-mousebuttons to a set of cgt-mousebuttons
    static cgt::MouseEvent::MouseButtons getButtons(QMouseEvent* e);
    static cgt::Event::Modifier getModifier(QInputEvent* e);
    static KeyEvent::KeyCode getKey(int key);
    static QGLFormat getQGLFormat(const Buffers buffers);

protected:
    static QGLWidget* shareWidget_;     ///< widget that this canvas shares the OpenGL context with

};

} // namespace

#endif // CGT_QTCANVAS_H
