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

#ifndef MDIDOCKAREA_H__
#define MDIDOCKAREA_H__

#include "mdidockablewindow.h"

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
    class MdiDockArea : public QMdiArea {

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
