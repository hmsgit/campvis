// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef MDIFLOATINGWINDOW_H__
#define MDIFLOATINGWINDOW_H__

#include <QWidget>

namespace campvis {

    /**
     * Specialised widget for detached MDI subwindows.
     *
     * MdiFloatingWindow is a top-level floating window representing an undocked MDI subwindow.
     * It reports changes in its position via the s_positionChanged signal. The MDI area that
     * created it listens to this signal to decide when to scrap the floating window and dock back
     * the widget that it wraps.
     *
     * MdiFloatingWindow also implements additional methods (forceWindowDrag, stopWindowDrag)
     * that should be used to coordinate its creation/disposal with respect to grabbing/releasing
     * the mouse input.
     */
    class MdiFloatingWindow : public QWidget {

        Q_OBJECT

    public:
        /**
         * Construct a new MdiFloatingWindow.
         *
         * \param canvas the widget this window is to wrap
         * \param parent the windows's parent
         */
        explicit MdiFloatingWindow(QWidget* widget, QWidget* parent = 0);

        /**
         * Enter the window into forced drag mode.
         *
         * This method causes the window to grab the mouse input and follow the cursor.
         */
        void forceWindowDrag();

        /**
         * Cancel the dragging of the window.
         *
         * This method causes the window to release the mouse grab and stop following the cursor.
         */
        void stopWindowDrag();

        /**
         * Return the widget this window wraps.
         */
        QWidget* widget();

    signals:
        /**
         * Emitted when the window's position changes.
         *
         * \param newPos the window's new position
         */
        void s_positionChanged(MdiFloatingWindow* pipelineWidget, const QPoint& newPos);

    protected:
        /**
         * Event handler that receives mouse move events for the window.
         */
        virtual void mouseMoveEvent(QMouseEvent* event);

        /**
         * Event handler that receives mouse release events for the window.
         */
        virtual void mouseReleaseEvent(QMouseEvent * event);

        /**
         * Event handler that receives move events for the window.
         */
        virtual void moveEvent(QMoveEvent* event);

    private:
        /**
         * Moves the window so that its title bar is centered around \p cursorPos.
         */
        void snapToCursor(const QPoint& cursorPos);

        QWidget* _widget;            ///< The widget this window wraps
        bool _dragActive;            ///< Is the window currently being dragged?

    };
}

#endif // MDIFLOATINGWINDOW_H__
