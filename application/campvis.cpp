// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "application/campvisapplication.h"

using namespace campvis;

/**
 * CAMPVis main function, application entry point
 *
 * \param   argc    number of passed arguments
 * \param   argv    vector of arguments
 * \return  0 if program exited successfully
 **/
int main(int argc, char** argv) {
    // Make Xlib and GLX thread safe under X11
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    CampVisApplication app(argc, argv);
    app.init();
    int toReturn = app.run();
    app.deinit();

    return toReturn;
}
