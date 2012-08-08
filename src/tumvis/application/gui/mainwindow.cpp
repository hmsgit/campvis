#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/tumvisapplication.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"

namespace TUMVis {

    MainWindow::MainWindow(TumVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _pipelineWidget(0)
    {
        tgtAssert(_application != 0, "Application must not be 0.");
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
    }

    void MainWindow::setup() {
        _centralWidget = new QWidget(this);
        QHBoxLayout* boxLayout = new QHBoxLayout();
        boxLayout->setSpacing(4);

        _pipelineWidget = new PipelineTreeWidget(_centralWidget);
        boxLayout->addWidget(_pipelineWidget);

        _propCollectionWidget = new PropertyCollectionWidget(_centralWidget);
        boxLayout->addWidget(_propCollectionWidget);

        _centralWidget->setLayout(boxLayout);
        setCentralWidget(_centralWidget);

        connect(
            this, SIGNAL(updatePipelineWidget(const std::vector<AbstractPipeline*>&)), 
            _pipelineWidget, SLOT(update(const std::vector<AbstractPipeline*>&)));
        connect(
            _pipelineWidget, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(onPipelineWidgetItemClicked(const QModelIndex&)));
        connect(
            this, SIGNAL(updatePropCollectionWidget(HasPropertyCollection*)),
            _propCollectionWidget, SLOT(updatePropCollection(HasPropertyCollection*)));
        _application->s_PipelinesChanged.connect(this, &MainWindow::onPipelinesChanged);
    }

    void MainWindow::onPipelinesChanged() {
        emit updatePipelineWidget(_application->_pipelines);
    }

    void MainWindow::onPipelineWidgetItemClicked(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);
            HasPropertyCollection* ptr = static_cast<HasPropertyCollection*>(item.value<void*>());
            emit updatePropCollectionWidget(ptr);
        }
        else {
            emit updatePropCollectionWidget(0);
        }
    }

    QSize MainWindow::sizeHint() const {
        return QSize(800, 450);
    }

}