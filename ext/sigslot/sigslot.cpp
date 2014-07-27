// ================================================================================================
// 
// sigslot.h/sigslot.cpp - Signal/Slot classes:
// 
// Original siglsot implementation written by Sarah Thompson (sarah@telergy.com) 2002 and 
// published under public domain. <http://sigslot.sourceforge.net/>
// 
// This version of the sigslot library is heavily modified, C++ compliant, inherently thread-safe,
// and offers a manager class that allows to queue and asynchronously dispatch signals.
// 
// Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "sigslot.h"

namespace sigslot {

    signal_manager::signal_manager() {

    }

    signal_manager::~signal_manager() {

    }

    void signal_manager::triggerSignal(_signal_handle_base* signal) const {
        signal->emitSignal();
        delete signal;
    }

    bool signal_manager::queueSignal(_signal_handle_base* signal) {
        tgtAssert(signal != 0, "Signal must not be 0.");
        if (signal == 0)
            return false;

        _signalQueue.push(signal);
        _evaluationCondition.notify_all();
        return true;
    }

    void signal_manager::run() {
        _this_thread_id = std::this_thread::get_id();
        std::unique_lock<std::mutex> lock(_ecMutex);

        while (! _stopExecution) {
            // try pop the next event from the event queue
            _signal_handle_base* signal;
            if (_signalQueue.try_pop(signal)) {
                signal->emitSignal();
                delete signal;
            }
            else {
                // there currently is no event in this queue -> go sleep
                _evaluationCondition.wait(lock);
            }
        }
    }

    void signal_manager::stop() {
        _evaluationCondition.notify_all(); 
        tgt::Runnable::stop();
    }

    bool signal_manager::isCurrentThreadSignalManagerThread() const {
        return std::this_thread::get_id() == _this_thread_id;
    }


    const std::string signal_manager::loggerCat_;

}
