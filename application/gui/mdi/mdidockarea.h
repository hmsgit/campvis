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

#include "mdidockedwindow.h"
#include "mdifloatingwindow.h"

#include <QMdiArea>

class QSignalMapper;

namespace campvis {

    /**
     * MDI area whose subwindows can be docked and undocked.
     *
     * MdiDockArea takes care of creating all necessary representations (docked and floating window)
     * of the widgets passed to \ref addSubWindow and seamlessly switching between them in response
     * to the user's actions (window dragging, key presses, etc).
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
         * Wrap \p widget in an MDI window and dock it in the MDI area.
         *
         * This method creates a MdiDockedWindow wrapping the widget, and adds it to the MDI area.
         *
         * \param widget the widget to add to the MDI area
         * \param windowFlags flags used to customize the frame of the created subwindow
         * \return the PipelineMdiSubWindow instance that was added to the MDI area
         */
        MdiDockedWindow* addSubWindow(QWidget* widget, Qt::WindowFlags windowFlags = 0);

        QMenu* menu();

    private slots:
        /**
         * Track the position of a floating MDI window and dock it if necessary.
         *
         * This slot is invoked when the position of a floating MDI window changes.
         */
        void trackFloatingWindowsPosition(MdiFloatingWindow* floatingWindow, const QPoint& newPos);

        /**
         * Track the position of a docked MDI window and detach it if necessary.
         *
         * This slot is invoked when the position of an MDI subwindow changes.
         */
        void trackMdiSubWindowsPosition(MdiDockedWindow* mdiSubWindow, const QPoint& newPos);

        /**
         * Display docked windows as sub-windows with window frames.
         */
        void switchToTiledDisplay();

        /**
         * Display docked windows with tabs in a tab bar.
         */
        void switchToTabbedDisplay();

        /**
         * Depending on the state of \p actionObject, show or hide its associated sub-window.
         *
         * \param actionObject the visibility action whose corresponding sub-window should be shown or hidden
         */
        void toggleSubWindowVisibility(QObject* actionObject);

    private:
        /**
         * Add the given MdiDockedWindow to the MDI area.
         *
         * \param dockedWindow the docked window to add
         */
        void addDockedWindow(MdiDockedWindow* dockedWindow);

        QMenu* _menu;                   ///< Menu with actions for controlling the MDI area and its subwindows.
        QSignalMapper* _signalMapper;   ///< Helper used to pass extra information to toggleSubWindowVisibility.

    };
}

#endif // MDIDOCKAREA_H__
