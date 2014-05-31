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

#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/campvisapplication.h"
#include "application/gui/datacontainerinspectorwidget.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/mdi/mdidockablewindow.h"
#include "application/gui/qtdatahandle.h"
#include "core/datastructures/datacontainer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/tools/stringutils.h"
#include "modules/pipelinefactory.h"

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
        , _dcInspectorWindow(0)
        , _btnExecute(0)
        , _btnShowDataContainerInspector(0)
        , _selectedPipeline(0)
        , _selectedProcessor(0)
        , _selectedDataContainer(0)
        , _logViewer(0)
        , _scriptingConsoleWidget(nullptr)
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

#ifdef CAMPVIS_HAS_SCRIPTING
        _scriptingConsoleWidget = new ScriptingWidget(this);
        ui.scriptingConsoleDock->setWidget(_scriptingConsoleWidget);
        connect(_scriptingConsoleWidget, SIGNAL(s_commandExecuted(const QString&)), this, SLOT(onLuaCommandExecuted(const QString&)));
#endif

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
        fileMenu->addAction(tr("&Rebuild all Shaders from File"), this, SLOT(onRebuildShadersClicked()), QKeySequence(Qt::CTRL + Qt::Key_F5));
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
        std::vector<AbstractPipeline*> pipelines;
        std::for_each(_application->_pipelines.begin(), _application->_pipelines.end(), [&] (const CampVisApplication::PipelineRecord& pr) { pipelines.push_back(pr._pipeline); });

        emit updatePipelineWidget(_application->_dataContainers, pipelines);
    }

    void MainWindow::onDataContainersChanged() {
        std::vector<AbstractPipeline*> pipelines;
        std::for_each(_application->_pipelines.begin(), _application->_pipelines.end(), [&] (const CampVisApplication::PipelineRecord& pr) { pipelines.push_back(pr._pipeline); });

        emit updatePipelineWidget(_application->_dataContainers, pipelines);
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
            if (_dcInspectorWindow == 0) {
                _dcInspectorWindow = _mdiArea->addWidget(_dcInspectorWidget);
                _dcInspectorWindow->setWindowTitle(tr("Data Container Inspector"));
            }

            _dcInspectorWidget->setDataContainer(&(_selectedPipeline->getDataContainer()));
            _dcInspectorWindow->show();
            _dcInspectorWindow->activateWindow();
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
        MdiDockableWindow* dockableWindow = _mdiArea->addWidget(canvas);
        QString windowTitle = QString::fromStdString(name);
        dockableWindow->setWindowTitle(windowTitle);
        dockableWindow->show();
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

    void MainWindow::onRebuildShadersClicked() {
        _application->rebuildAllShadersFromFiles();
    }

    void MainWindow::onLuaCommandExecuted(const QString& cmd) {
        // FIXME: so far just a hack
        if (! _application->_pipelines.empty()){
            _application->_pipelines.front()._luaVmState->execString(cmd.toStdString());
        }
    }

}
