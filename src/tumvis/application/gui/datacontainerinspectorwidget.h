// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef DATACONTAINERINSPECTORWIDGET_H__
#define DATACONTAINERINSPECTORWIDGET_H__

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#include "core/tools/opengljobprocessor.h"

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

class QModelIndex;

namespace tgt {
    class Texture;
}

namespace TUMVis {
    class AbstractPipeline;
    class DataContainer;
    class DataContainerTreeWidget;
    class DataHandle;

    class DataContainerInspectorWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        DataContainerInspectorWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~DataContainerInspectorWidget();

        /**
         * Set the DataContainer this widget is inspecting.
         * \param   dataContainer   The DataContainer this widget shall inspect, may be 0.
         */
        void setDataContainer(DataContainer* dataContainer);

        /**
         * Slot called when _dataContainer has changed and emitted the s_changed signal.
         */
        void onDataContainerChanged();

        /**
         * Size hint for the default window size
         * \return QSize(640, 480)
         */
        QSize sizeHint() const;

    private slots:
        /**
         * Slot to be called by the DataContainerTreeWidget when the selected item changes.
         * \param   index   Index of the selected item
         */
        void onDCTWidgetItemClicked(const QModelIndex& index);

    protected:
        /**
         * Setup the GUI stuff
         */
        void setupGUI();

        /**
         * Updates _infoWidget
         */
        void updateInfoWidget();

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting
        const DataHandle* _selectedDataHandle;          ///< The currently selected DataHandle
        QString _selectedDataHandleName;                ///< The name of the currently selected DataHandle

        DataContainerTreeWidget* _dctWidget;            ///< The TreeWidget showing the DataHandles in _dataContainer
        tgt::QtThreadedCanvas* _canvas;                 ///< The OpenGL canvas for rendering the DataContainer's contents

        QHBoxLayout* _mainLayout;                       ///< Layout for this widget
        QWidget* _infoWidget;                           ///< Widget showing the information about the selected DataHandle
        QVBoxLayout* _infoWidgetLayout;                 ///< Layout for the _infoWidget

        QLabel* _lblName;
        QLabel* _lblTimestamp;
    };
}

#endif // DATACONTAINERINSPECTORWIDGET_H__