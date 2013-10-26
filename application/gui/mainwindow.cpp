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

#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/campvisapplication.h"
#include "application/gui/datacontainerinspectorwidget.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/qtdatahandle.h"
#include "core/datastructures/datacontainer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/tools/stringutils.h"
#include "modules/pipelinefactory.h"

#include <QMdiSubWindow>
#include <QScrollBar>


namespace campvis {

    MainWindow::MainWindow(CampVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _mdiArea(0)
        , _containerWidget(0)
        , _cbPipelineFactory(0)
        , _btnPipelineFactory(0)
        , _pipelineWidget(0)
        , _propCollectionWidget(0)
        , _dcInspectorWidget(0)
        , _dcInspectorDock(0)
        , _btnExecute(0)
        , _btnShowDataContainerInspector(0)
        , _selectedPipeline(0)
        , _selectedProcessor(0)
        , _selectedDataContainer(0)
        , _logViewer(0)
    {
        tgtAssert(_application != 0, "Application must not be 0.");
        ui.setupUi(this);
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
        _application->s_DataContainersChanged.disconnect(this);
        delete _dcInspectorWidget;
    }

    void MainWindow::setup() {
        qRegisterMetaType<QtDataHandle>("QtDataHandle");

        setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
        setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

        setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);

        _mdiArea = new MdiDockArea();
        _mdiArea->tileSubWindows();
        setCentralWidget(_mdiArea);

        _containerWidget = new QWidget(this);
        QGridLayout* _cwLayout = new QGridLayout(_containerWidget);

        _cbPipelineFactory = new QComboBox(_containerWidget);
        std::vector<std::string> registeredPipelines = PipelineFactory::getRef().getRegisteredPipelines();
        for (std::vector<std::string>::const_iterator it = registeredPipelines.begin(); it != registeredPipelines.end(); ++it)
            _cbPipelineFactory->addItem(QString::fromStdString(*it));
        _cwLayout->addWidget(_cbPipelineFactory, 0, 0);

        _btnPipelineFactory = new QPushButton("Add Pipeline", _containerWidget);
        _cwLayout->addWidget(_btnPipelineFactory, 0, 1);

        _pipelineWidget = new PipelineTreeWidget(this);
        _containerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        _cwLayout->addWidget(_pipelineWidget, 1, 0, 1, 2);

        _containerWidget->setLayout(_cwLayout);
        ui.pipelineTreeDock->setWidget(_containerWidget);

        _pipelinePropertiesScrollArea = new QScrollArea(this);
        _pipelinePropertiesScrollArea->setWidgetResizable(true);
        _pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);

        _pipelinePropertiesWidget = new QWidget(_pipelinePropertiesScrollArea);
        _pipelinePropertiesWidget->installEventFilter(this);
        _pipelinePropertiesScrollArea->setWidget(_pipelinePropertiesWidget);
        ui.pipelinePropertiesDock->setWidget(_pipelinePropertiesScrollArea);

        QVBoxLayout* rightLayout = new QVBoxLayout(_pipelinePropertiesWidget);
        rightLayout->setSpacing(4);
        _pipelinePropertiesWidget->setLayout(rightLayout);

        _btnExecute = new QPushButton("Execute Selected Pipeline/Processor", _pipelinePropertiesWidget);
        rightLayout->addWidget(_btnExecute);

        _btnShowDataContainerInspector = new QPushButton("Inspect DataContainer of Selected Pipeline", _pipelinePropertiesWidget);
        rightLayout->addWidget(_btnShowDataContainerInspector);

        _propCollectionWidget = new PropertyCollectionWidget(this);
        rightLayout->addWidget(_propCollectionWidget);
        rightLayout->addStretch();

        _logViewer = new LogViewerWidget(this);
        ui.logViewerDock->setWidget(_logViewer);

        _dcInspectorWidget = new DataContainerInspectorWidget();
        this->populateMainMenu();

        connect(
            this, SIGNAL(updatePipelineWidget(const std::vector<DataContainer*>&, const std::vector<AbstractPipeline*>&)), 
            _pipelineWidget, SLOT(update(const std::vector<DataContainer*>&, const std::vector<AbstractPipeline*>&)));
        connect(
            _pipelineWidget, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(onPipelineWidgetItemClicked(const QModelIndex&)));
        connect(
            this, SIGNAL(updatePropCollectionWidget(HasPropertyCollection*, DataContainer*)),
            _propCollectionWidget, SLOT(updatePropCollection(HasPropertyCollection*, DataContainer*)));
        connect(
            _btnExecute, SIGNAL(clicked()), 
            this, SLOT(onBtnExecuteClicked()));
        connect(
            _btnShowDataContainerInspector, SIGNAL(clicked()), 
            this, SLOT(onBtnShowDataContainerInspectorClicked()));
        connect(
            _btnPipelineFactory, SIGNAL(clicked()), 
            this, SLOT(onBtnPipelineFactoryClicked()));

        _application->s_PipelinesChanged.connect(this, &MainWindow::onPipelinesChanged);
        _application->s_DataContainersChanged.connect(this, &MainWindow::onDataContainersChanged);
    }

    void MainWindow::populateMainMenu() {
        // Populate the file menu
        QMenuBar* menuBar = this->menuBar();
        QMenu* fileMenu = menuBar->addMenu(tr("&File"));
        fileMenu->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()), QKeySequence(Qt::CTRL + Qt::Key_Q));

        // Populate the visualizations menu
        QMenu* visualizationsMenu = _mdiArea->menu();
        visualizationsMenu->setTitle(tr("&Visualizations"));
        menuBar->addMenu(visualizationsMenu);

        // Populate the tools menu
        QMenu* toolsMenu = menuBar->addMenu(tr("&Tools"));
        toolsMenu->addAction(ui.pipelineTreeDock->toggleViewAction());
        toolsMenu->addAction(ui.pipelinePropertiesDock->toggleViewAction());
        toolsMenu->addAction(ui.logViewerDock->toggleViewAction());
    }

    bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
        if (watched == _pipelinePropertiesWidget && event->type() == QEvent::Resize) {
            _pipelinePropertiesScrollArea->setMinimumWidth(_pipelinePropertiesWidget->minimumSizeHint().width() +
                                                           _pipelinePropertiesScrollArea->verticalScrollBar()->width());
        }

        return false;
    }

    void MainWindow::onPipelinesChanged() {
        emit updatePipelineWidget(_application->_dataContainers, _application->_pipelines);
    }

    void MainWindow::onDataContainersChanged() {
        emit updatePipelineWidget(_application->_dataContainers, _application->_pipelines);
    }

    void MainWindow::onPipelineWidgetItemClicked(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);
            if (item.isValid()) {
                HasPropertyCollection* ptr = static_cast<HasPropertyCollection*>(item.value<void*>());

                if (AbstractPipeline* pipeline = dynamic_cast<AbstractPipeline*>(ptr)) {
                    _selectedPipeline = pipeline;
                    _selectedProcessor = 0;
                    _selectedDataContainer = &pipeline->getDataContainer();
                }
                else if (AbstractProcessor* processor = dynamic_cast<AbstractProcessor*>(ptr)) {
                    _selectedProcessor = processor;

                    QVariant parentItem = index.parent().data(Qt::UserRole);
                    HasPropertyCollection* pptr = static_cast<HasPropertyCollection*>(parentItem.value<void*>());
                    if (AbstractPipeline* pipeline = dynamic_cast<AbstractPipeline*>(pptr)) {
                        _selectedPipeline = pipeline;
                        _selectedDataContainer = &pipeline->getDataContainer();
                    }
                }

                emit updatePropCollectionWidget(ptr, &_selectedPipeline->getDataContainer());
            }
            else {
                emit updatePropCollectionWidget(0, 0);
                _selectedDataContainer = 0;
            }
        }
        else {
            emit updatePropCollectionWidget(0, 0);
            _selectedDataContainer = 0;
        }
    }

    QSize MainWindow::sizeHint() const {
        return QSize(1000, 600);
    }

    void MainWindow::onBtnExecuteClicked() {
        if (_selectedProcessor != 0 && _selectedPipeline != 0) {
            // this is not as trivial as it seems:
            // We need the pipeline, probably an OpenGL context...
            _selectedProcessor->invalidate(AbstractProcessor::INVALID_RESULT);
        }
        else if (_selectedPipeline != 0) {
            for (std::vector<AbstractProcessor*>::const_iterator it = _selectedPipeline->getProcessors().begin(); it != _selectedPipeline->getProcessors().end(); ++it) {
                (*it)->invalidate(AbstractProcessor::INVALID_RESULT);
            }
        }
    }

    void MainWindow::onBtnShowDataContainerInspectorClicked() {
        if (_selectedPipeline != 0) {
            if (_dcInspectorDock == 0) {
                _dcInspectorDock = dockPrimaryWidget("Data Container inspector", _dcInspectorWidget);
            } else {
                // Activate the dock's tab
                _dcInspectorDock->setVisible(true);
                _dcInspectorDock->raise();
            }

            _dcInspectorWidget->setDataContainer(&(_selectedPipeline->getDataContainer()));
        }
    }

    void MainWindow::init() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->init();

        _logViewer->init();
    }

    void MainWindow::deinit() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->deinit();

        _logViewer->deinit();
    }

    void MainWindow::addVisualizationPipelineWidget(const std::string& name, QWidget* canvas) {
        QMdiSubWindow* mdiSubWindow = _mdiArea->addSubWindow(canvas);
        const QString& windowTitle = QString::fromStdString(name);
        mdiSubWindow->setWindowTitle(windowTitle);
    }

    QDockWidget* MainWindow::dockPrimaryWidget(const std::string& name, QWidget* widget) {
        QDockWidget* dockWidget = new QDockWidget(QString::fromStdString(name));
        dockWidget->setWidget(widget);
        dockWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        if (_primaryDocks.empty()) {
            addDockWidget(Qt::TopDockWidgetArea, dockWidget);
        } else {
            tabifyDockWidget(_primaryDocks.back(), dockWidget);
            // Activate the dock's tab
            dockWidget->setVisible(true);
            dockWidget->raise();
        }

        _primaryDocks.push_back(dockWidget);
        return dockWidget;
    }

    void MainWindow::onBtnPipelineFactoryClicked() {
        std::string name = this->_cbPipelineFactory->currentText().toStdString();
        DataContainer* dc = _selectedDataContainer;
        if (dc == 0) {
            dc = _application->createAndAddDataContainer("DataContainer #" + StringUtils::toString(_application->_dataContainers.size() + 1));
        }
        AbstractPipeline* p = PipelineFactory::getRef().createPipeline(name, dc);
        _application->addPipeline(name, p);
    }

}
