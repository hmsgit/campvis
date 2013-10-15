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

#ifndef PIPELINEMDISUBWINDOW_H__
#define PIPELINEMDISUBWINDOW_H__

#include <QMdiSubWindow>

namespace campvis {

    /**
     * QMdiSubWindow subclass for visualization pipeline widgets.
     *
     * PipelineMdiSubWindow reports changes in its position via the s_positionChanged signal.
     * Higher-level components listen to this signal to decide when to detach the subwindow from
     * its MDI area. PipelineMdiSubWindow also implements additional methods (stopWindowDrag) that
     * should be used to coordinate this detaching with respect to grabbing/releasing the mouse
     * input.
     */
    class PipelineMdiSubWindow : public QMdiSubWindow {

        Q_OBJECT

    public:
        /**
         * Construct an MDI subwindow for a visualization pipeline.
         *
         * \param parent the window's parent
         * \param flags options customizing the frame of the subwindow
         */
        explicit PipelineMdiSubWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

        /**
         * Cancel the dragging of the window.
         *
         * This method causes the window to release the mouse grab and stop following the cursor.
         * It's supposed to be called when the window is detached from the MDI area.
         */
        void stopWindowDrag();

    signals:
        /**
         * Emitted when the subwindow's position changes.
         *
         * \param newPos the subwindow's new position
         */
        void s_positionChanged(const QPoint& newPos);

    protected:
        /**
         * Event handler that receives mouse move events for the widget.
         */
        virtual void mouseMoveEvent(QMouseEvent* event);

        /**
         * Event handler that receives mouse release events for the widget.
         */
        virtual void mouseReleaseEvent(QMouseEvent * event);

    private:
        bool _dragActive;            ///< Is the window currently being dragged?
        QPoint _lastMousePos;        ///< Last reported mouse position

    };
}

#endif // PIPELINEMDISUBWINDOW_H__
