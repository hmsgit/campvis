#include "application/tumvisapplication.h"
#include "modules/pipelines/slicevis.h"

using namespace TUMVis;

/**
 * TUMVis main function, application entry point
 *
 * \param   argc    number of passed arguments
 * \param   argv    vector of arguments
 * \return  0 if program exited successfully
 **/
int main(int argc, char** argv) {
    TumVisApplication app(argc, argv);
    app.addVisualizationPipeline("SliceVis", new SliceVis());

    app.init();
    int toReturn = app.run();
    app.deinit();

    return toReturn;
}
