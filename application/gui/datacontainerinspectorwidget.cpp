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

#include "datacontainerinspectorwidget.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/filesystem.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/renderdata.h"

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/datastructures/fiberdata.h"
#endif

#include "application/gui/datacontainertreewidget.h"
#include "application/gui/qtdatahandle.h"
#include "application/gui/datacontainerfileloaderwidget.h"
#include "modules/io/processors/genericimagereader.h"
#include "modules/io/processors/mhdimagewriter.h"

#include <QFileDialog>
#include <QScrollArea>

namespace campvis {

    const std::string DataContainerInspectorWidget::loggerCat_ = "CAMPVis.application.DataContainerInspectorWidget";

    DataContainerInspectorWidget::DataContainerInspectorWidget(QWidget* parent) 
        : QWidget(parent)
        , _inited(false)
        , _dataContainer(0)
        , _dctWidget(0)
        , _canvas(0)
        , _pcWidget(0)
        , _mainLayout(0)
        , _infoWidget(0)
        , _infoWidgetLayout(0)
        , _lblName(0)
        , _lblNumChannels(0)
        , _lblLocalMemoryFootprint(0)
        , _lblVideoMemoryFootprint(0)
        , _lblTimestamp(0)
        , _lblSize(0)
        , _lblBounds(0)
        , _colorWidget(0)
        , _colorWidgetLayout(0)
        , _lblColorVal(0)
        , _colorValWidget(0)
        , _btnLoadFile(0)
        , _btnSaveToFile(0)
        , _propEditorWid(0)
    {
        setupGUI();
    }

    DataContainerInspectorWidget::~DataContainerInspectorWidget() {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        delete _pcWidget;
        _pcWidget = nullptr;
    }

    void DataContainerInspectorWidget::setDataContainer(DataContainer* dataContainer) {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        _dataContainer = dataContainer;
        _dctWidget->update(dataContainer);
        updateInfoWidget();

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.connect(this, &DataContainerInspectorWidget::onDataContainerDataAdded);
        }
    }

    DataContainer* DataContainerInspectorWidget::getDataContainer() {
        return _dataContainer;
    }

    void DataContainerInspectorWidget::onDataContainerDataAdded(std::string key, DataHandle dh) {
        // copy QtDataHandle because signal will be handled by a different thread an indefinite amount of time later:
        emit dataContainerChanged(QString::fromStdString(key), QtDataHandle(dh));
    }

    QSize DataContainerInspectorWidget::sizeHint() const {
        return QSize(800, 600);
    }


    void DataContainerInspectorWidget::setupGUI() {
        setWindowTitle(tr("DataContainer Inspector"));

        _mainLayout = new QGridLayout();
        _mainLayout->setSpacing(4);
        setLayout(_mainLayout);

        // left column
        _dctWidget = new DataContainerTreeWidget(this);
        _dctWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        _dctWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        _dctWidget->setMinimumWidth(256);
        _mainLayout->addWidget(_dctWidget, 0, 0);

        _btnLoadFile = new QPushButton(tr("Load File"), _infoWidget);
        _mainLayout->addWidget(_btnLoadFile, 1, 0);

#ifdef CAMPVIS_HAS_MODULE_DEVIL
        _btnSaveToFile = new QPushButton(tr("Save to File"), _infoWidget);
        _btnSaveToFile->setDisabled(true);
        _mainLayout->addWidget(_btnSaveToFile, 2, 0);

        connect(_btnSaveToFile, SIGNAL(clicked()), this, SLOT(onBtnSaveToFileClicked()));
#endif


        // right column
        _infoWidget = new QWidget(this);
        _infoWidgetLayout = new QGridLayout();
        _infoWidgetLayout->setSpacing(4);
        _infoWidget->setLayout(_infoWidgetLayout);

        _lblName = new QLabel(QString("Name: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblName, 0, 0);

        _lblTimestamp = new QLabel("Timestamp: ", _infoWidget);
        _infoWidgetLayout->addWidget(_lblTimestamp, 0, 1);

        _lblNumChannels = new QLabel("Number of Channels: ", _infoWidget);
        _infoWidgetLayout->addWidget(_lblNumChannels, 1, 0);

        _lblLocalMemoryFootprint = new QLabel(QString("Local Memory: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblLocalMemoryFootprint, 1, 1);

        _lblSize = new QLabel(tr("Size: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblSize, 2, 0);

        _lblVideoMemoryFootprint = new QLabel(QString("Video Memory: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblVideoMemoryFootprint, 2, 1);

        _lblBounds = new QLabel(tr("World Bounds:"), _infoWidget);
        _infoWidgetLayout->addWidget(_lblBounds, 3, 0, 1, 2);

        _colorWidget = new QWidget(this);
        _lblColorVal = new QLabel(tr("Color: n/a"), _colorWidget);
        
        _colorValWidget = new QWidget(_colorWidget);
        _colorValWidget->setAutoFillBackground(true);
        _colorValWidget->setFixedSize(16, 16);
        
        _colorValWidgetPalette = QPalette(palette());
        _colorValWidgetPalette.setColor(QPalette::Background, Qt::gray);
        _colorValWidget->setPalette(_colorValWidgetPalette);
        
        _colorWidgetLayout = new QHBoxLayout();
        _colorWidgetLayout->setSpacing(0);
        _colorWidgetLayout->setMargin(0);
        _colorWidget->setLayout(_colorWidgetLayout);
        
        _colorWidgetLayout->addWidget(_lblColorVal);
        _colorWidgetLayout->addWidget(_colorValWidget);

        _infoWidgetLayout->addWidget(_colorWidget, 4, 0, 1, 2);

        _canvas = new DataContainerInspectorCanvas(_infoWidget);
        _canvas->setMinimumSize(QSize(100, 100));
        _infoWidgetLayout->addWidget(_canvas, 5, 0, 1, 2);
        _infoWidgetLayout->setRowStretch(5, 2);

        QScrollArea* _pipelinePropertiesScrollArea = new QScrollArea(_infoWidget);
        _pipelinePropertiesScrollArea->setWidgetResizable(true);
        _pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);
        _pipelinePropertiesScrollArea->setMinimumHeight(224);

        _pcWidget = new PropertyCollectionWidget(_pipelinePropertiesScrollArea);
        _pcWidget->updatePropCollection(_canvas, _dataContainer);
        _pipelinePropertiesScrollArea->setWidget(_pcWidget);

        _infoWidgetLayout->addWidget(_pipelinePropertiesScrollArea, 6, 0, 1, 2);

        _mainLayout->addWidget(_infoWidget, 0, 1, 3, 1);

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        qRegisterMetaType<cgt::vec4>("tgt_vec4");
        connect(
            _dctWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
            this, SLOT(onDCTWidgetSelectionModelSelectionChanged(const QItemSelection&, const QItemSelection&)));
        connect(
            this, SIGNAL(dataContainerChanged(const QString&, QtDataHandle)),
            _canvas, SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
        connect(
            _canvas, SIGNAL(s_colorChanged(const cgt::vec4&)),
            this, SLOT(onColorChanged(const cgt::vec4&)));
        connect(
            _canvas, SIGNAL(s_depthChanged(float)),
            this, SLOT(onDepthChanged(float)));
        connect(
            this, SIGNAL(dataContainerChanged(const QString&, QtDataHandle)),
            _dctWidget->getTreeModel(), SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
        connect(
            _btnLoadFile, SIGNAL(clicked()),
            this, SLOT(onBtnLoadFileClicked()));
    }

    void DataContainerInspectorWidget::updateInfoWidget() {
        if (!_inited)
            return;

        // get the selection from the tree widget
        const QModelIndexList& indices = _dctWidget->selectionModel()->selectedRows();
        std::vector< std::pair<QString, QtDataHandle> > handles;
        float _localFootprint = 0.f;
        float _videoFootprint = 0.f;

        // iterate through the indices of the selection
        for (QModelIndexList::const_iterator index = indices.begin(); index != indices.end(); ++index) {
            if (! index->isValid())
                continue;
            // get DataHandle and Handle name
            QVariant item = index->data(Qt::UserRole);
            QtDataHandle handle = item.value<QtDataHandle>();
            QModelIndex idxName = index->sibling(index->row(), 0);

            // only consider non-empty DataHandles
            if (handle.getData() != 0) {
                handles.push_back(std::make_pair(idxName.data(Qt::DisplayRole).toString(), handle));
                _localFootprint += handle.getData()->getLocalMemoryFootprint();
                _videoFootprint += handle.getData()->getVideoMemoryFootprint();
            }
        }

        // update labels
        if (handles.size() == 1) {
            _lblName->setText("Name: " + handles.front().first);
            _lblTimestamp->setText("Timestamp: " + QString::number(handles.front().second.getTimestamp()));

            if (const ImageData* tester = dynamic_cast<const ImageData*>(handles.front().second.getData())) {
                _canvas->p_transferFunction.setImageHandle(handles.front().second);

                _lblNumChannels->setText(tr("Number of Channels: ") + QString::number(tester->getNumChannels()));

                std::ostringstream ss;

                ss << tester->getSize();
                _lblSize->setText(tr("Size: ") + QString::fromStdString(ss.str()));

                ss.str("");
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 

                _canvas->p_currentSlice.setVisible(tester->getDimensionality() == 3);
                _canvas->p_transferFunction.setVisible(true);
                _canvas->p_renderRChannel.setVisible(true);
                _canvas->p_renderGChannel.setVisible(true);
                _canvas->p_renderBChannel.setVisible(true);
                _canvas->p_renderAChannel.setVisible(true);
                _canvas->p_geometryRendererProperties.setVisible(false);
            }
            else if (const GeometryData* tester = dynamic_cast<const GeometryData*>(handles.front().second.getData())) {
                _lblSize->setText(tr("Size: n/a"));
                _lblNumChannels->setText(tr("Number of Channels: n/a"));

                std::ostringstream ss;
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str()));

                _canvas->p_currentSlice.setVisible(false);
                _canvas->p_transferFunction.setVisible(false);
                _canvas->p_renderRChannel.setVisible(false);
                _canvas->p_renderGChannel.setVisible(false);
                _canvas->p_renderBChannel.setVisible(false);
                _canvas->p_renderAChannel.setVisible(false);
                _canvas->p_geometryRendererProperties.setVisible(true);
            }
            else if (const RenderData* tester = dynamic_cast<const RenderData*>(handles.front().second.getData())) {
                const ImageData* id = tester->getNumColorTextures() > 0 ? tester->getColorTexture() : tester->getDepthTexture();
                if (id != 0) {
                    _lblNumChannels->setText(tr("Number of Channels: ") + QString::number(id->getNumChannels()));

                    std::ostringstream ss;
                    ss << id->getSize();
                    _lblSize->setText(tr("Size: ") + QString::fromStdString(ss.str()));

                    ss.str("");
                    ss << id->getWorldBounds();
                    _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 
                }
                else {
                    _lblNumChannels->setText(tr("Number of Channels: n/a"));
                    _lblSize->setText(tr("Size: n/a"));
                    _lblBounds->setText(tr("World Bounds: n/a")); 
                }

                _canvas->p_currentSlice.setVisible(false);
                _canvas->p_transferFunction.setVisible(true);
                _canvas->p_renderRChannel.setVisible(true);
                _canvas->p_renderGChannel.setVisible(true);
                _canvas->p_renderBChannel.setVisible(true);
                _canvas->p_renderAChannel.setVisible(true);
                _canvas->p_geometryRendererProperties.setVisible(false);
            }
#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
            else if (const FiberData* tester = dynamic_cast<const FiberData*>(handles.front().second.getData())) {
                _lblNumChannels->setText(tr("Number of Channels: n/a"));
                std::ostringstream ss;
                ss << "Size: " << tester->numFibers() << " Fibers with " << tester->numSegments() << " Segments.";
                _lblSize->setText(QString::fromStdString(ss.str()));

                ss.str("");
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 
            }
#endif
            else {
                _lblNumChannels->setText(tr("Number of Channels: n/a"));
                _lblSize->setText(tr("Size: n/a"));
                _lblBounds->setText(tr("World Bounds: n/a")); 
            }
        }
        else {
            _lblName->setText(QString::number(handles.size()) + " DataHandles selected");
            _lblTimestamp->setText("Timestamp: n/a");

            _canvas->p_transferFunction.setImageHandle(DataHandle(0));
        }
        _lblLocalMemoryFootprint->setText("Local Memory: " + humanizeBytes(_localFootprint));
        _lblVideoMemoryFootprint->setText("Video Memory: " + humanizeBytes(_videoFootprint));

        // update DataHandles for the DataContainerInspectorCanvas
        _canvas->setDataHandles(handles);
    }

    QString DataContainerInspectorWidget::humanizeBytes(size_t numBytes) const {
        QString units[5] = { tr(" Bytes"), tr(" KB"), tr(" MB"), tr(" GB"), tr(" TB") };
        size_t index = 0;
        size_t remainder = 0;

        while (numBytes > 1024 && index < 4) {
            remainder = numBytes % 1024;
            numBytes /= 1024;
            ++index;
        }

        if (remainder != 0)
            return QString::number(numBytes) + "." + QString::number(remainder) + units[index];
        else
            return QString::number(numBytes) + units[index];
    }

    void DataContainerInspectorWidget::init() {
        if (_canvas != 0)
            _canvas->init();

        _inited = true;
    }

    void DataContainerInspectorWidget::deinit() {
        _inited = false;
        if (_canvas != 0)
            _canvas->deinit();

        _pcWidget->updatePropCollection(0, 0);

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        _dataContainer = 0;
        _dctWidget->update(0);

        if(_propEditorWid != nullptr)
            _propEditorWid->deinit();
    }

    void DataContainerInspectorWidget::onDCTWidgetSelectionModelSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
        updateInfoWidget();

#ifdef CAMPVIS_HAS_MODULE_DEVIL
        // get the selection from the tree widget
        const QModelIndexList& indices = _dctWidget->selectionModel()->selectedRows();

        // iterate through the indices of the selection
        for (QModelIndexList::const_iterator index = indices.begin(); index != indices.end(); ++index) {
            if (index->isValid()) {
                _btnSaveToFile->setDisabled(false);
                return;
            }
        }
        _btnSaveToFile->setDisabled(true);
#endif
    }

    void DataContainerInspectorWidget::onBtnSaveToFileClicked() {
        // get the selection from the tree widget
        const QModelIndexList& indices = _dctWidget->selectionModel()->selectedRows();

        // iterate through the indices of the selection
        for (QModelIndexList::const_iterator index = indices.begin(); index != indices.end(); ++index) {
            if (! index->isValid())
                continue;

            // get DataHandle and Handle name
            QVariant item = index->data(Qt::UserRole);
            DataHandle handle = item.value<QtDataHandle>();
            QModelIndex idxName = index->sibling(index->row(), 0);
            QString name = idxName.data(Qt::DisplayRole).toString();

            // only consider non-empty DataHandles that are ImageData and have render target representations
            if (handle.getData() != 0) {
                if (dynamic_cast<const ImageData*>(handle.getData()) && dynamic_cast<const ImageData*>(handle.getData())->getDimensionality() == 3) {
                    QString dialogCaption = "Export " + name + " as MHD Image";
                    QString directory = tr("");
                    const QString fileFilter = tr("*.mhd;;MHD images (*.mhd)");

                    QString filename = QFileDialog::getSaveFileName(this, dialogCaption, directory, fileFilter);

                    MhdImageWriter writer;
                    writer.p_fileName.setValue(filename.toStdString());
                    writer.p_inputImage.setValue(name.toStdString());
                    writer.invalidate(AbstractProcessor::INVALID_RESULT);
                    writer.process(*_dataContainer);
                }
                else if (dynamic_cast<const ImageData*>(handle.getData()) || dynamic_cast<const RenderData*>(handle.getData())) {
                    QString dialogCaption = "Export " + name + " as Image";
                    QString directory = tr("");
                    const QString fileFilter = tr("*.png;;PNG images (*.png)");

                    QString filename = QFileDialog::getSaveFileName(this, dialogCaption, directory, fileFilter);

                    if (! filename.isEmpty()) {
                        // Texture access needs OpenGL context - dispatch method call:
                        cgt::GLContextScopedLock lock(_canvas);
                        saveToFile(handle, filename.toStdString());
                    }
                }
            }
        }
    }

    void DataContainerInspectorWidget::saveToFile(DataHandle handle, std::string filename) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
        if (cgt::FileSystem::fileExtension(filename).empty()) {
            LERRORC("CAMPVis.application.DataContainerInspectorWidget", "Filename has no extension");
            return;
        }

        // get the ImageData object (either directly or from the RenderData)
        const ImageData* id = 0;
        if (const RenderData* tester = dynamic_cast<const RenderData*>(handle.getData())) {
            id = tester->getColorTexture(0);
        }
        else if (const ImageData* tester = dynamic_cast<const ImageData*>(handle.getData())) {
            id = tester;
        }
        else {
            LERROR("Could not extract image to save.");
            return;
        }

        // extract the data
        WeaklyTypedPointer wtp(WeaklyTypedPointer::UINT8, 1, 0);
        const ImageRepresentationLocal* repLocal = id->getRepresentation<ImageRepresentationLocal>(true);
        if (repLocal != 0)
            wtp = repLocal->getWeaklyTypedPointer();

        if (wtp._pointer == 0) {
            LERROR("Could not extract image to save.");
            return;
        }


        // create Devil image from image data and write it to file
        ILuint img;
        ilGenImages(1, &img);
        ilBindImage(img);

        // put pixels into IL-Image
        cgt::ivec2 size = id->getSize().xy();
        ilTexImage(size.x, size.y, 1, static_cast<ILubyte>(wtp._numChannels), wtp.getIlFormat(), wtp.getIlDataType(), wtp._pointer);
        ilEnable(IL_FILE_OVERWRITE);
        ilResetWrite();
        ILboolean success = ilSaveImage(filename.c_str());
        ilDeleteImages(1, &img);

        if (!success) {
            LERRORC("CAMPVis.application.DataContainerInspectorWidget", "Could not save image to file: " << ilGetError());
        }
#else
        return;
#endif

    }

    void DataContainerInspectorWidget::onBtnLoadFileClicked() {
        // delete previous PropertyEditor, then create a new one
        // the final one will be deleted with deinit()
        if(nullptr != _propEditorWid)
            _propEditorWid->deinit();

        _propEditorWid = new DataContainerFileLoaderWidget(this, nullptr);
        _propEditorWid->setVisible(true);
        
    }

    void DataContainerInspectorWidget::onColorChanged(const cgt::vec4& color) {
        _lblColorVal->setText(QString("Color: [%1, %2, %3, %4]").arg(QString::number(color.r), QString::number(color.g), QString::number(color.b), QString::number(color.a)));

        cgt::ivec4 clamped(cgt::clamp(color * 255.f, 0.f, 255.f));
        _colorValWidgetPalette.setColor(QPalette::Background, QColor(clamped.r, clamped.g, clamped.b, clamped.a));
        _colorValWidget->setPalette(_colorValWidgetPalette);
    }

    void DataContainerInspectorWidget::onDepthChanged(float depth) {
        _lblColorVal->setText(QString("Depth: %1").arg(QString::number(depth)));

        cgt::ivec4 clamped(cgt::clamp(depth * 255.f, 0.f, 255.f));
        _colorValWidgetPalette.setColor(QPalette::Background, QColor(clamped.r, clamped.g, clamped.b, clamped.a));
        _colorValWidget->setPalette(_colorValWidgetPalette);
    }

}