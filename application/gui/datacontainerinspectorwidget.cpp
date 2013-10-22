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

#include "datacontainerinspectorwidget.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/datastructures/fiberdata.h"
#endif

#include "application/gui/datacontainertreewidget.h"
#include "application/gui/qtdatahandle.h"

#include <QFileDialog>

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
    {
        setupGUI();
    }

    DataContainerInspectorWidget::~DataContainerInspectorWidget() {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }
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

    void DataContainerInspectorWidget::onDataContainerDataAdded(const std::string& key, const DataHandle& dh) {
        // copy QtDataHandle because signal will be handled by a different thread an indefinite amount of time later:
        emit dataContainerChanged(QString::fromStdString(key), QtDataHandle(dh));
    }

    QSize DataContainerInspectorWidget::sizeHint() const {
        return QSize(800, 600);
    }


    void DataContainerInspectorWidget::setupGUI() {
        setWindowTitle(tr("DataContainer Inspector"));

        _mainLayout = new QHBoxLayout();
        _mainLayout->setSpacing(4);
        setLayout(_mainLayout);

        _dctWidget = new DataContainerTreeWidget(this);
        _dctWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        _dctWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        _mainLayout->addWidget(_dctWidget);

        _infoWidget = new QWidget(this);
        _infoWidgetLayout = new QVBoxLayout();
        _infoWidgetLayout->setSpacing(4);
        _infoWidget->setLayout(_infoWidgetLayout);

        _lblName = new QLabel(QString("Name: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblName);

        _lblLocalMemoryFootprint = new QLabel(QString("Local Memory Footprint: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblLocalMemoryFootprint);

        _lblVideoMemoryFootprint = new QLabel(QString("Video Memory Footprint: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblVideoMemoryFootprint);

        _lblTimestamp = new QLabel("Timestamp: ", _infoWidget);
        _infoWidgetLayout->addWidget(_lblTimestamp);

        _lblSize = new QLabel(tr("Size: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblSize);

		
		_wdgtColor = new QWidget(this);
		//_lblColor = new QLabel(tr("Color:"), _wdgtColor);
		_lblColorVal = new QLabel(tr("Color: n/a"), _wdgtColor);
		
		_wdgtColorVal = new QWidget(this);
		_wdgtColorVal->setAutoFillBackground(true);
		_wdgtColorVal->setFixedSize(16, 16);
		
		_wdgtColorValPalette = new QPalette(palette());
		_wdgtColorValPalette->setColor(QPalette::ColorRole::Background, Qt::gray);
		_wdgtColorVal->setPalette(*(_wdgtColorValPalette));
		
		
		_wdgtColorLayout = new QHBoxLayout();
		_wdgtColorLayout->setSpacing(0);
		_wdgtColorLayout->setMargin(0);
		//_wdgtColorLayout->setAlignment(Qt::Alignment::enum_type::AlignLeft);
		_wdgtColor->setLayout(_wdgtColorLayout);
		
		//_wdgtColorLayout->addWidget(_lblColor);
		_wdgtColorLayout->addWidget(_lblColorVal);
		_wdgtColorLayout->addWidget(_wdgtColorVal);

		_infoWidgetLayout->addWidget(_wdgtColor);
		
        _lblBounds = new QLabel(tr("World Bounds:"), _infoWidget);
        _infoWidgetLayout->addWidget(_lblBounds);

        _btnSaveToFile = new QPushButton(tr("Save to File"), _infoWidget);
        _infoWidgetLayout->addWidget(_btnSaveToFile);

        _canvas = new DataContainerInspectorCanvas(_infoWidget);
        _canvas->setMinimumSize(QSize(100, 100));
        _infoWidgetLayout->addWidget(_canvas, 1);

        _pcWidget = new PropertyCollectionWidget(_infoWidget);
        _pcWidget->updatePropCollection(_canvas, _dataContainer);
        _infoWidgetLayout->addWidget(_pcWidget);

        _mainLayout->addWidget(_infoWidget, 1);

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        connect(
            _dctWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
            this, SLOT(onDCTWidgetSelectionModelSelectionChanged(const QItemSelection&, const QItemSelection&)));
        connect(
            this, SIGNAL(dataContainerChanged(const QString&, QtDataHandle)),
            _canvas, SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
        connect(
            this, SIGNAL(dataContainerChanged(const QString&, QtDataHandle)),
            _dctWidget->getTreeModel(), SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
        connect(
            _btnSaveToFile, SIGNAL(clicked()),
            this, SLOT(onBtnSaveToFileClicked()));
    }

	void DataContainerInspectorWidget::updateColor(){

		char bufferR[10];
		char bufferG[10];
		char bufferB[10];
		tgt::Color color = _canvas->getCapturedColor();

		itoa((int)(color.r * 255), bufferR, 10);
		itoa((int)(color.g * 255), bufferG, 10);
		itoa((int)(color.b * 255), bufferB, 10);

		_lblColorVal->setText(QString("Color: R = ") + QString(bufferR) + QString(" G = ") + QString(bufferG) + QString(" B = ") + QString(bufferB));
		QColor qtColor;
		qtColor.setRgb(color.r * 255, color.g * 255, color.b * 255);
		
		_wdgtColorValPalette->setColor(QPalette::ColorRole::Background, qtColor);
		_wdgtColorVal->setPalette(*_wdgtColorValPalette);
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
                _canvas->p_transferFunction.getTF()->setImageHandle(handles.front().second);
                std::ostringstream ss;

                ss << tester->getSize();
                _lblSize->setText(tr("Size: ") + QString::fromStdString(ss.str()));

                ss.str("");
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 

            }
            else if (const GeometryData* tester = dynamic_cast<const GeometryData*>(handles.front().second.getData())) {
                _lblSize->setText(tr("Size: n/a"));

				std::ostringstream ss;
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 

				//tester->render();
            }
            else if (const RenderData* tester = dynamic_cast<const RenderData*>(handles.front().second.getData())) {
                const ImageData* id = tester->getNumColorTextures() > 0 ? tester->getColorTexture() : tester->getDepthTexture();
                if (id != 0) {
                    std::ostringstream ss;
                    ss << id->getSize();
                    _lblSize->setText(tr("Size: ") + QString::fromStdString(ss.str()));

                    ss.str("");
                    ss << id->getWorldBounds();
                    _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 
                }
                else {
                    _lblSize->setText(tr("Size: n/a"));
                    _lblBounds->setText(tr("World Bounds: n/a")); 
                }
            }
#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
            else if (const FiberData* tester = dynamic_cast<const FiberData*>(handles.front().second.getData())) {
                std::ostringstream ss;
                ss << "Size: " << tester->numFibers() << " Fibers with " << tester->numSegments() << " Segments.";
                _lblSize->setText(QString::fromStdString(ss.str()));

                ss.str("");
                ss << tester->getWorldBounds();
                _lblBounds->setText(tr("World Bounds: ") + QString::fromStdString(ss.str())); 
            }
#endif
            else {

				_lblSize->setText(tr("Size: n/a"));
                _lblBounds->setText(tr("World Bounds: n/a")); 
            }
        }
        else {
            _lblName->setText(QString::number(handles.size()) + " DataHandles selected");
            _lblTimestamp->setText("Timestamp: n/a");

            _canvas->p_transferFunction.getTF()->setImageHandle(DataHandle(0));
        }
        _lblLocalMemoryFootprint->setText("Local Memory Footprint: " + humanizeBytes(_localFootprint));
        _lblVideoMemoryFootprint->setText("Video Memory Footprint: " + humanizeBytes(_videoFootprint));

        // update DataHandles for the DataContainerInspectorCanvas
        _canvas->setDataHandles(handles);
    }

    QString DataContainerInspectorWidget::humanizeBytes(size_t numBytes) const {
        QString units[5] = { tr(" Bytes"), tr(" KB"), tr(" MB"), tr(" GB"), tr(" TB") };
        size_t index = 0;
        size_t remainder = 0;

        while (numBytes > 1024 && index < 5) {
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
            _canvas->init(this);

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
    }

    void DataContainerInspectorWidget::onDCTWidgetSelectionModelSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
        updateInfoWidget();
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

            // only consider non-empty DataHandles that are ImageData and have render target representations
            if (handle.getData() != 0) {
                if (dynamic_cast<const ImageData*>(handle.getData()) || dynamic_cast<const RenderData*>(handle.getData())) {
                    QString dialogCaption = "Export " + idxName.data(Qt::DisplayRole).toString() + " as Image";
                    QString directory = tr("");
                    const QString fileFilter = tr("*.png;;PNG images (*.png)");

                    QString filename = QFileDialog::getSaveFileName(this, dialogCaption, directory, fileFilter);

                    if (! filename.isEmpty()) {
                        // Texture access needs OpenGL context - dispatch method call:
                        GLJobProc.enqueueJob(
                            _canvas, 
                            makeJobOnHeap(&DataContainerInspectorWidget::saveToFile, handle, filename.toStdString()), 
                            OpenGLJobProcessor::SerialJob);
                    }
                }
            }
        }
    }

    void DataContainerInspectorWidget::saveToFile(DataHandle handle, std::string filename) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
        if (tgt::FileSystem::fileExtension(filename).empty()) {
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
        const ImageRepresentationGL* repGL = id->getRepresentation<ImageRepresentationGL>(false);
        if (repGL != 0) // if it's a GL texture, download it (we do not want to use the automatic conversion method here)
            wtp = repGL->getWeaklyTypedPointer();
        else {
            const ImageRepresentationLocal* repLocal = id->getRepresentation<ImageRepresentationLocal>(true);
            if (repLocal != 0)
                wtp = repLocal->getWeaklyTypedPointer();
        }

        if (wtp._pointer == 0) {
            LERROR("Could not extract image to save.");
            return;
        }


        // create Devil image from image data and write it to file
        ILuint img;
        ilGenImages(1, &img);
        ilBindImage(img);

        // put pixels into IL-Image
        tgt::ivec2 size = id->getSize().xy();
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

}