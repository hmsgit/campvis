#ifndef TUMVISMAINWINDOW_H__
#define TUMVISMAINWINDOW_H__

#include "sigslot/sigslot.h"
#include "application/tumvisapplication.h"
#include "application/gui/pipelinetreewidget.h"

#include <QMainWindow>
#include <vector>

namespace TUMVis {
    class MainWindow : public QMainWindow, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        MainWindow(TumVisApplication* application);

        ~MainWindow();


        void onPipelinesChanged();

    signals:
        void updatePipelineWidget(const std::vector<AbstractPipeline*>&);


    private:
        void setup();

        TumVisApplication* _application;            ///< Pointer to the application hosting the whole stuff

        PipelineTreeWidget* _pipelineWidget;        ///< Widget for browsing the active pipelines

    };
}

#endif // TUMVISMAINWINDOW_H__
