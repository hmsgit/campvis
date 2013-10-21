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

#ifndef DATACONTAINERINSPECTORWIDGET_H__
#define DATACONTAINERINSPECTORWIDGET_H__

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tbb/mutex.h"

#include "application/gui/qtdatahandle.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/properties/propertycollectionwidget.h"
#include "core/tools/opengljobprocessor.h"

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QPushButton>

class QModelIndex;
class QItemSelection;

namespace tgt {
    class Texture;
    class TextureUnit;
    class Shader;
}

namespace campvis {
    class AbstractPipeline;
    class DataContainer;
    class DataContainerTreeWidget;
    class FaceGeometry;

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
         * Slot called when _dataContainer has changed and emitted the s_dataAdded signal.
         */
        void onDataContainerDataAdded(const std::string&, const DataHandle&);

		void mousePressEvent(QMouseEvent*)
		{
			updateInfoWidget();
		}

        /**
         * Size hint for the default window size
         * \return QSize(640, 480)
         */
        QSize sizeHint() const;
        
        /**
         * Initializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        virtual void init();

        /**
         * Deinitializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        void deinit();

		/**
         * Updates _infoWidget
         */
        void updateInfoWidget();

    signals:
        void dataContainerChanged(const QString&, QtDataHandle);

    private slots:

        /**
         * Slot to be called when the selection in the DataContainerTreeWidget has changed.
         * \param selected      selected items
         * \param deselected    deselected items
         */
        void onDCTWidgetSelectionModelSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

        /**
         * Slot being called when the user clicks on the "Save to File" button.
         */
        void onBtnSaveToFileClicked();

    protected:
        /**
         * Setup the GUI stuff
         */
        void setupGUI();

        /**
         * Saves the Image in \a handle to the file \a filename.
         * \note    This method must be called with a valid OpenGL context!
         * \param handle    DataHandle containing the image to save. Must contain ImageData or RenderData!
         * \param filename  Filename for the file to save.
         */
        static void saveToFile(DataHandle handle, std::string filename);

        /**
         * Returns a string with \a numBytes humanized (i.e. "numBytes/1024^n [KMG]Byte")
         * \param   numBytes    Number of bytes to be converted.
         * \return  "numBytes/1024^n [KMG]Bytes
         */
        QString humanizeBytes(size_t numBytes) const;

        bool _inited;

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting

        DataContainerTreeWidget* _dctWidget;            ///< The TreeWidget showing the DataHandles in _dataContainer
        DataContainerInspectorCanvas* _canvas;                 ///< The OpenGL canvas for rendering the DataContainer's contents
        PropertyCollectionWidget* _pcWidget;

        QHBoxLayout* _mainLayout;                       ///< Layout for this widget
        QWidget* _infoWidget;                           ///< Widget showing the information about the selected QtDataHandle
        QVBoxLayout* _infoWidgetLayout;                 ///< Layout for the _infoWidget

        QLabel* _lblName;
        QLabel* _lblLocalMemoryFootprint;
        QLabel* _lblVideoMemoryFootprint;
        QLabel* _lblTimestamp;
        QLabel* _lblSize;
        QLabel* _lblBounds;
		QLabel* _lblColor;
        QPushButton* _btnSaveToFile;

        static const std::string loggerCat_;
    };
}

#endif // DATACONTAINERINSPECTORWIDGET_H__
