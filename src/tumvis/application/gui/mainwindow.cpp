#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/tumvisapplication.h"

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

    }

    void MainWindow::setup() {
        _pipelineWidget = new PipelineTreeWidget(this);
        setCentralWidget(_pipelineWidget);

        connect(this, SIGNAL(updatePipelineWidget(const std::vector<AbstractPipeline*>&)), _pipelineWidget, SLOT(update(const std::vector<AbstractPipeline*>&)));
        _application->s_PipelinesChanged.connect(this, &MainWindow::onPipelinesChanged);
    }

    void MainWindow::onPipelinesChanged() {
        emit updatePipelineWidget(_application->_pipelines);
    }

}