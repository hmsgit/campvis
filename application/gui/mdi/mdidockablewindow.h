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

#ifndef MDIDOCKABLEWINDOW_H__
#define MDIDOCKABLEWINDOW_H__

#include "mdidockedwindow.h"
#include "mdifloatingwindow.h"

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
    class MdiDockableWindow : public QWidget {

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
