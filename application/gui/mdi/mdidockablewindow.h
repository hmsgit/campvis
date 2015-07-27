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

#ifndef MDIDOCKABLEWINDOW_H__
#define MDIDOCKABLEWINDOW_H__

#include "mdidockedwindow.h"
#include "mdifloatingwindow.h"

#include "application/gui/qtcolortools.h"
#include <QWidget>

namespace campvis {

    class MdiDockArea;

    /**
     * MDI window that be docked and undocked.
     *
     * MdiDockableWindow takes care of creating all necessary representations (docked and floating
     * window) of the widget it's passed and seamlessly switching between them in response to the
     * user's actions (window dragging, key presses, etc).
     */
    class CAMPVIS_APPLICATION_API MdiDockableWindow : public QWidget {

        Q_OBJECT

    public:
        /**
         * Construct a new MdiDockableWindow.
         *
         * \param widget the widget this window is to wrap
         * \param mdiArea the MDI area the window should dock in
         * \param windowFlags flags used to customize the frame of the window
         */
        MdiDockableWindow(QWidget* widget, MdiDockArea* mdiArea, Qt::WindowFlags windowFlags = 0);

        /**
         * Set the title of this window.
         *
         * \param title the window's new title
         */
        void setWindowTitle(const QString& title);

        /**
         * Set this window to be the active window.
         *
         * Calling this function causes the window to get the keyboard input focus.
         */
        void activateWindow();

        /**
         * Change the window's visibility.
         *
         * \param visible should the window be visible?
         */
        virtual void setVisible(bool visible);

        /**
         * Returns a checkable action that can be used to show or hide this window.
         *
         * The action's text is set to this window's title.
         */
        QAction* toggleViewAction() const;

    private slots:
        /**
         * Track the position of the associated floating MDI window and dock it if necessary.
         *
         * This slot is invoked when the position of the floating MDI window changes.
         */
        void trackFloatingWindowPosition(const QPoint& newPos);

        /**
         * Track the position of the associated docked MDI window and detach it if necessary.
         *
         * This slot is invoked when the position of the docked MDI window changes.
         */
        void trackDockedWindowPosition(const QPoint& newPos);

        /**
         * Depending on the state of _toggleViewAction, show or hide the window.
         *
         * \param on true if _toggleViewAction is checked, false otherwise
         */
        void toggleWindowVisibility(bool on);

        /**
         * Hide the closed MdiDockedWindow/MdiFloatingWindow and update the visibility action.
         */
        void handleWindowClosing();

    private:
        /**
         * Create and return an MdiDockedWindow wrapping the \p widget.
         *
         * \param widget the widget the new docked window should wrap
         */
        MdiDockedWindow* newDockedWindow(QWidget* widget);

        bool _docked;                         ///< Is the window currently docked?
        MdiDockArea* _mdiArea;                ///< The MDI this window docks in.
        MdiDockedWindow* _dockedWindow;       ///< The window's docked representation.
        MdiFloatingWindow* _floatingWindow;   ///< The window's floating representation.
        QAction* _toggleViewAction;           ///< A checkable action that can be used to show or hide this window.

    };
}

#endif // MDIDOCKABLEWINDOW_H__
