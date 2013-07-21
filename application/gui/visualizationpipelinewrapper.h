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

#ifndef VISUALIZATIONPIPELINEWRAPPER_H__
#define VISUALIZATIONPIPELINEWRAPPER_H__

#include "pipelinemdisubwindow.h"
#include "visualizationpipelinewidget.h"

#include <QMdiArea>

namespace campvis {

    /**
     * Display wrapper for visualization pipelines.
     *
     * VisualizationPipelineWrapper takes care of creating all necessary representations (widget,
     * MDI subwindow) of a visualization pipeline and seamlessly switching between them in
     * response to the user's actions (window dragging, key presses, etc).
     */
    class VisualizationPipelineWrapper : public QObject {

        Q_OBJECT;

    public:
        /**
         * Construct a wrapper for a visualization pipeline.
         *
         * This constructor creates a widget for the visualization pipeline it's passed. It then
         * adds it to an MDI subwindow, and places it in the specified MDI area.
         *
         * \param name the name of the visualization pipeline
         * \param canvas the pipeline's canvas
         * \param mdiArea the MDI are to which the widget should be added
         * \param parent the widget's parent
         */
        VisualizationPipelineWrapper(const std::string& name, QWidget* canvas,
                                     QMdiArea* mdiArea, QObject* parent = 0);

    private slots:
        /**
         * Re-tile subwindows in the MDI area.
         *
         * This slot is invoked whenever the pipeline's MDI subwindow is detached or reattached.
         */
        void retileMdiSubWindows();

    private:
        QMdiArea* _mdiArea;                              ///< The MDI area associated with the widget
        PipelineMdiSubWindow* _mdiSubWindow;             ///< An MDI subwindow for the pipeline
        VisualizationPipelineWidget* _pipelineWidget;    ///< A widget for the pipeline

    };
}

#endif // VISUALIZATIONPIPELINEWRAPPER_H__
