// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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
    class PropertyEditorWidget;

    class DataContainerInspectorWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit DataContainerInspectorWidget(QWidget* parent = 0);

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
         * Get the DataContainer this widget is inspecting.
         */
        DataContainer* getDataContainer();

        /**
         * Slot called when _dataContainer has changed and emitted the s_dataAdded signal.
         */
        void onDataContainerDataAdded(const std::string&, const DataHandle&);

        void mousePressEvent(QMouseEvent*) {
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
         * Updates color of the info widget
         */
        void updateColor();

        /**
         * Updates depth of the info widget
         */
        void updateDepth();

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

        
        /**
         * Slot being called when the user clicks on the "Load File" button.
         */
        void onBtnLoadFileClicked();

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
        

    protected:
        static void saveToFile(DataHandle handle, std::string filename);

        /**
         * Returns a string with \a numBytes humanized (i.e. "numBytes/1024^n [KMG]Byte")
         * \param   numBytes    Number of bytes to be converted.
         * \return  "numBytes/1024^n [KMG]Bytes
         */
        QString humanizeBytes(size_t numBytes) const;

        /**
         * Updates _infoWidget
         */
        void updateInfoWidget();

        bool _inited;

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting

        DataContainerTreeWidget* _dctWidget;            ///< The TreeWidget showing the DataHandles in _dataContainer
        DataContainerInspectorCanvas* _canvas;          ///< The OpenGL canvas for rendering the DataContainer's contents
        PropertyCollectionWidget* _pcWidget;

        QHBoxLayout* _mainLayout;                       ///< Layout for this widget
        QWidget* _infoWidget;                           ///< Widget showing the information about the selected QtDataHandle
        QVBoxLayout* _infoWidgetLayout;                 ///< Layout for the _infoWidget

        QLabel*  _lblName;
        QLabel*  _lblLocalMemoryFootprint;
        QLabel*  _lblVideoMemoryFootprint;
        QLabel*  _lblTimestamp;
        QLabel*  _lblSize;
        QLabel*  _lblBounds;
        QWidget* _colorWidget;                          ///< The widget use to show the color value and the color in a single window
        QHBoxLayout* _colorWidgetLayout;                ///< Layout for the following widget
        QLabel*  _lblColorVal;                          ///< Color Label Value in text
        QWidget* _colorValWidget;                       ///< Widget that shows the color value in color
        QPalette* _ColorValWidgetPalette;               ///< Palette which will be used to colorize the color widget
        QPushButton* _btnSaveToFile;
        
        // Added by Hossain Mahmud  <mahmud@in.tum.de>
        // Date: January 02, 2014
        QPushButton* _btnLoadFile;
        PropertyEditorWidget* _propEditorWid;

        static const std::string loggerCat_;
    };
}

#endif // DATACONTAINERINSPECTORWIDGET_H__
