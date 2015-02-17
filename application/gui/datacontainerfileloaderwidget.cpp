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

#include "datacontainerfileloaderwidget.h"

#include <QScrollBar>

namespace campvis {

    const std::string DataContainerFileLoaderWidget::loggerCat_ = "CAMPVis.application.DataContainerFileLoaderWidget";

    DataContainerFileLoaderWidget::DataContainerFileLoaderWidget(DataContainerInspectorWidget* parentDataInspector, QWidget* parent) 
        : QWidget(parent)
        , _dataContainer(0)
        , _layout(0)
        , _pipelinePropertiesScrollArea(0)
        , _fileName("fileName", "Image URL", "")
        , _propCollectionWidget(0)
    {
        _parent = parentDataInspector;
        _dataContainer = _parent->getDataContainer();
        _imgReader = new GenericImageReader();
        setupGUI();
    }

    DataContainerFileLoaderWidget::~DataContainerFileLoaderWidget() {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }
    }

    void DataContainerFileLoaderWidget::setDataContainer(DataContainer* dataContainer) {
        _dataContainer = dataContainer;
    }

    QSize DataContainerFileLoaderWidget::sizeHint() const {
        return QSize(300, 350);
    }


    void DataContainerFileLoaderWidget::setupGUI() {
        setWindowTitle(tr("Browse File"));

        _layout = new QGridLayout();
        _layout->setSpacing(2);
        setLayout(_layout);

        _pipelinePropertiesScrollArea = new QScrollArea(this);
        _pipelinePropertiesScrollArea->setWidgetResizable(true);
        _pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);
        
        _propCollectionWidget = new PropertyCollectionWidget(_pipelinePropertiesScrollArea);
        _propCollectionWidget->installEventFilter(this);
        _pipelinePropertiesScrollArea->setWidget(_propCollectionWidget);
        _propCollectionWidget->updatePropCollection(_imgReader, _dataContainer);
        _layout->addWidget(_pipelinePropertiesScrollArea, 0, 0, 1, 2);

        _btnLoadFile = new QPushButton(tr("Load"), this);
        _layout->addWidget(_btnLoadFile, 1, 0, 1, 1);
        _btnCancel = new QPushButton(tr("Cancel"), this);
        _layout->addWidget(_btnCancel, 1, 1, 1, 1);

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        connect(_btnCancel, SIGNAL(clicked()), this, SLOT(onBtnCancelClicked()));
        connect( _btnLoadFile, SIGNAL(clicked()), this, SLOT(onBtnLoadFileClicked()));
    }


    bool DataContainerFileLoaderWidget::eventFilter(QObject* watched, QEvent* event) {
        if (watched == _propCollectionWidget && event->type() == QEvent::Resize) {
            _pipelinePropertiesScrollArea->setMinimumWidth(_propCollectionWidget->minimumSizeHint().width() +
                _pipelinePropertiesScrollArea->verticalScrollBar()->width());
        }

        return false;
    }

    void DataContainerFileLoaderWidget::init() {
    }

    void DataContainerFileLoaderWidget::deinit() {
        delete _imgReader;
        _imgReader = nullptr;
    }

    void DataContainerFileLoaderWidget::onBtnCancelClicked() {
        delete _imgReader;
        _imgReader = nullptr;
        close();
    }

    void DataContainerFileLoaderWidget::onBtnLoadFileClicked() {
        _imgReader->process(*_dataContainer);
        _parent->setDataContainer(_dataContainer);
        close();
    }

}
