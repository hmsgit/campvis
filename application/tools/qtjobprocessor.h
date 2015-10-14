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

#ifndef QTJOBPROCESSOR_H__
#define QTJOBPROCESSOR_H__

#include "cgt/job.h"
#include "cgt/singleton.h"

#include "application/applicationapi.h"

#include <QWidget>
#include <functional>

namespace campvis {

    /**
     * This job processor singleton can be used to execute jobs (asynchronously) from inside the Qt GUI thread.
     * This can result in simpler and better understandable code when only a few lines of code need
     * to be executed in the context, as it removes the need to introduce a new signal and slot just
     * to transition into the Qt GUI thread.
     * 
     * Example:
     * \code
     * QtJobProc.enqueueJob([=](){
     *     _application->_mainWindow->statusBar()->showMessage(QString::fromStdString(status), timeout);
     * });
     * \endcode
     */
    class CAMPVIS_APPLICATION_API QtJobProcessor : public QWidget, public cgt::Singleton<QtJobProcessor> {
        Q_OBJECT;

    public:
        
        QtJobProcessor();
                
        virtual ~QtJobProcessor();

        /**
         Enqueues a new job to be processed by the job processor
         */
        void enqueueJob(cgt::AbstractJob* job);

        /**
         Convenience function to simplify the code, removing the necessity for a makeJobOnHeap() call
         */
        void enqueueJob(std::function<void(void)> fn);

    signals:
        void newJobSignal(cgt::AbstractJob* job);

    private slots:
        void onJobArrived(cgt::AbstractJob* job);
    };

}

#define QtJobProc cgt::Singleton<campvis::QtJobProcessor>::getRef()


#endif // QTJOBPROCESSOR_H__
