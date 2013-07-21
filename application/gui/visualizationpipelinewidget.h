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

#ifndef VISUALIZATIONPIPELINEWIDGET_H__
#define VISUALIZATIONPIPELINEWIDGET_H__

#include <QWidget>

namespace campvis {

    /**
     * Specialised widget for visualization pipelines.
     *
     * VisualizationPipelineWidget can be seamlessly used with MDI subwindows, as well as
     * a top-level floating window.
     */
    class VisualizationPipelineWidget : public QWidget {

        Q_OBJECT;

    public:
        /**
         * Construct a floating widget for a visualization pipeline.
         *
         * \param canvas the pipeline's canvas
         * \param parent the widget's parent
         */
        VisualizationPipelineWidget(QWidget* canvas, QWidget* parent = 0);

    public slots:
        /**
         * Enter the widget into forced drag mode.
         *
         * This slot is invoked after the the widget has been detached from an MDI area and become
         * a floating window. It lets the user drag the window as if it was still the same widget
         * (MdiSubWindow) that has just been "pulled out" of the MDI area.
         */
        void forceWindowDrag();

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
        bool _dragActive;            ///< Is the widget currently being dragged?
        QPoint _lastMousePos;        ///< Last reported mouse position

    };
}

#endif // VISUALIZATIONPIPELINEWIDGET_H__
