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

#ifndef MDIDOCKAREA_H__
#define MDIDOCKAREA_H__

#include "mdidockablewindow.h"
#include "application/gui/qtcolortools.h"

#include <QMdiArea>

namespace campvis {

    /**
     * MDI area whose subwindows can be docked and undocked.
     *
     * MdiDockArea extends QMdiArea's functionality by adding support for dockable MDI windows. It
     * provides 2 APIs:
     * - addSubWindow and removeSubWindow are much like their QMdiArea's counterparts; they operate
     *   on MDI sub-windows only, which don't support docking/undocking
     * - dockable windows can be created using the addWidget method, which returns an
     *   MdiDockableWindow instance
     */
    class CAMPVIS_APPLICATION_API MdiDockArea : public QMdiArea {

        Q_OBJECT

    public:
        /**
         * Constructs an empty MDI area.
         *
         * \param parent the area's parent widget (passed to QMdiArea's constructor)
         */
        explicit MdiDockArea(QWidget* parent = 0);

        /**
         * Wrap \p widget in a dockable MDI window and add it to the MDI area.
         *
         * This method creates a MdiDockableWindow wrapping the widget, and adds it to the MDI area.
         *
         * \param widget the widget to add to the MDI area
         * \param windowFlags flags used to customize the frame of the created subwindow
         * \return the MdiDockableWindow instance that was added to the MDI area
         */
        MdiDockableWindow* addWidget(QWidget* widget, Qt::WindowFlags windowFlags = 0);

        /**
         * Add \p mdiSubWindow to the MDI area.
         *
         * \param mdiSubWindow the MDI sub-window to be added to the MDI area
         * \return the MDI sub-window that was passed in
         */
        QMdiSubWindow* addSubWindow(QMdiSubWindow* mdiSubWindow);

        /**
         * Remove \p mdiSubWindow from the MDI area.
         *
         * \param mdiSubWindow the MDI sub-window to be removed from the MDI area
         */
        void removeSubWindow(QMdiSubWindow* mdiSubWindow);

        /**
         * Return a menu that lets the user control how the area's contents are displayed.
         *
         * The menu contains actions that make it possible to change the layout and visibility of
         * the area's subwindows.
         */
        QMenu* menu() const;

    private slots:
        /**
         * Display docked windows as sub-windows with window frames.
         */
        void switchToTiledDisplay();

        /**
         * Display docked windows with tabs in a tab bar.
         */
        void switchToTabbedDisplay();

    private:
        QMenu* _menu;                   ///< Menu with actions for controlling the MDI area and its subwindows.

    };
}

#endif // MDIDOCKAREA_H__
