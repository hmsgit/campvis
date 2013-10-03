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

#include "pipelinemdisubwindow.h"
#include "visualizationpipelinewidget.h"

#include <QMdiArea>

namespace campvis {

    /**
     * MDI area whose subwindows can be docked and undocked.
     *
     * MdiDockArea takes care of creating all necessary representations (floating window, MDI
     * subwindow) of the widgets passed to \ref addSubWindow and seamlessly switching between them
     * in response to the user's actions (window dragging, key presses, etc).
     */
    class MdiDockArea : public QMdiArea {

        Q_OBJECT

    public:
        /**
         * Wrap \p widget in an MDI subwindow and dock it in the MDI area.
         *
         * This method creates a PipelineMdiSubWindow wrapping the widget it's passed, and adds it
         * to the MDI area.
         *
         * \param widget the widget to add to the MDI area
         * \param windowFlags flags used to customize the frame of the created subwindow
         * \return the PipelineMdiSubWindow instance that was added to the MDI area
         */
        PipelineMdiSubWindow* addSubWindow(QWidget* widget, Qt::WindowFlags windowFlags = 0);

    private slots:
        /**
         * Track the position of the pipeline's widget and dock it if necessary.
         *
         * This slot is invoked when the pipeline's widget is floating and its position changes.
         */
        void trackFloatingWindowsPosition(VisualizationPipelineWidget* pipelineWidget, const QPoint& newPos);

        /**
         * Track the position of the pipeline's MDI subwindow and detach it if necessary.
         *
         * This slot is invoked when the position of the pipeline's MDI subwindow changes.
         */
        void trackMdiSubWindowsPosition(PipelineMdiSubWindow* mdiSubWindow, const QPoint& newPos);

    };
}

#endif // MDIDOCKAREA_H__
