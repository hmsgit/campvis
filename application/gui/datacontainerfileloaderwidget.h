// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#ifndef DATACONTAINERFILELOADERWIDGET_H
#define DATACONTAINERFILELOADERWIDGET_H

#include "sigslot/sigslot.h"

#include "application/gui/properties/propertycollectionwidget.h"
#include "modules/io/processors/genericimagereader.h"
#include "application/gui/datacontainerinspectorwidget.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>

namespace campvis {

    class DataContainerFileLoaderWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerFileLoaderWidget.
         * \param   treeModel       Parent DataContainerInspectorWidget. Method Overridden to keep the widget floating.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit DataContainerFileLoaderWidget(DataContainerInspectorWidget* treeModel, QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~DataContainerFileLoaderWidget();

        /**
         * Set the DataContainer this widget is inspecting.
         * \param   dataContainer   The DataContainer this widget shall inspect, may be 0.
         */
        void setDataContainer(DataContainer* dataContainer);

        /**
         * Size hint for the default window size
         * \return QSize(300, 350)
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

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting
        QGridLayout* _layout;                       ///< Layout for the _infoWidget
        QScrollArea* _pipelinePropertiesScrollArea;     ///< Scroll area for _pipelinePropertiesWidget

    protected:
        QPushButton* _btnCancel;
        QPushButton* _btnLoadFile;
        StringProperty _fileName;
        PropertyCollectionWidget* _propCollectionWidget;    ///< Widget for browsing the PropertyCollection of the selected pipeline/processor

        GenericImageReader *_imgReader;
        DataContainerInspectorWidget* _parent;
        
        static const std::string loggerCat_;

    private:

    };
}

#endif // DATACONTAINERFILELOADERWIDGET_H
