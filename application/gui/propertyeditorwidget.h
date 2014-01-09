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

#ifndef PROPERTYEDITORWIDGET_H_
#define PROPERTYEDITORWIDGET_H_

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
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QPushButton>
#include <QMetaObject>

class QModelIndex;
class QItemSelection;

namespace tgt {
    class Texture;
    class TextureUnit;
    class Shader;
}

namespace campvis {
    class AbstractPipeline;
    //class DataContainer;
    //class DataContainerTreeWidget;
    //class FaceGeometry;

    class PropertyEditorWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit PropertyEditorWidget(QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~PropertyEditorWidget();

        /**
         * Set the DataContainer this widget is inspecting.
         * \param   dataContainer   The DataContainer this widget shall inspect, may be 0.
         */
        void setDataContainer(DataContainer* dataContainer);

        /**
         * Slot called when _dataContainer has changed and emitted the s_dataAdded signal.
         */
        void onDataContainerDataAdded(const std::string&, const DataHandle&);

        //void mousePressEvent(QMouseEvent*) {
        //    updateInfoWidget();
        //}

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
         * Slot being called when the user clicks on the "Cancel" button.
         */
        void onBtnCancelClicked();

        /**
         * Slot being called when the user clicks on the "Load File" button.
         */
        void onBtnLoadFileClicked();

    protected:
        /**
         * Setup the GUI stuff
         */
        void setupGUI();

    protected:
        /**
         * Returns a string with \a numBytes humanized (i.e. "numBytes/1024^n [KMG]Byte")
         * \param   numBytes    Number of bytes to be converted.
         * \return  "numBytes/1024^n [KMG]Bytes
         */
        QString humanizeBytes(size_t numBytes) const;

        bool _inited;

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting

        //DataContainerTreeWidget* _dctWidget;            ///< The TreeWidget showing the DataHandles in _dataContainer
        DataContainerInspectorCanvas* _canvas;          ///< The OpenGL canvas for rendering the DataContainer's contents
        //PropertyCollectionWidget* _pcWidget;

        QVBoxLayout* _mainLayout;                       ///< Layout for this widget
        //QWidget* _infoWidget;                           ///< Widget showing the information about the selected QtDataHandle
        QVBoxLayout* rightLayout;                 ///< Layout for the _infoWidget

        //QWidget* _pipelinePropertiesWidget;                 ///< Widget showing the selected pipeline's properties
        QScrollArea* _pipelinePropertiesScrollArea;         ///< Scroll area for _pipelinePropertiesWidget
    public:
        PropertyCollectionWidget* _propCollectionWidget;    ///< Widget for browsing the PropertyCollection of the selected pipeline/processor
    protected:
        QPushButton* _btnCancel;
        QPushButton* _btnLoadFile;

        static const std::string loggerCat_;
    };
}

#endif // PROPERTYEDITORWIDGET_H_
