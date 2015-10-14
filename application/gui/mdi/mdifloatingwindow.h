// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef MDIFLOATINGWINDOW_H__
#define MDIFLOATINGWINDOW_H__

#include "application/gui/qtcolortools.h"
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
    class CAMPVIS_APPLICATION_API MdiFloatingWindow : public QWidget {

        Q_OBJECT

    public:
        /**
         * Construct a new MdiFloatingWindow.
         *
         * \param widget the widget this window is to wrap
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
        QWidget* widget() const;

    signals:
        /**
         * Emitted when the window's position changes.
         *
         * \param newPos the window's new position
         */
        void s_positionChanged(const QPoint& newPos);

        /**
         * Emitted when the window gets closed.
         */
        void s_closed();

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

        /**
         * Event handler called when Qt receives a window close request for the window.
         */
        virtual void closeEvent(QCloseEvent* event);

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
