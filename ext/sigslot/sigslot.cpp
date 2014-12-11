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

#include <memory>
#include <new>

#include "sigslot.h"

namespace sigslot {

    signal_manager::signal_manager() 
        : _handlingMode(DEFAULT)
    {

    }

    signal_manager::~signal_manager() {
        _signalPool.recycle();
    }

    void signal_manager::triggerSignalImpl(_signal_handle_base* signal) {
        if (_handlingMode == FORCE_QUEUE) {
            queueSignalImpl(signal);
            return;
        }

        signal->processSignal();
        delete signal;
    }

    bool signal_manager::queueSignalImpl(_signal_handle_base* signal) {
        if (_handlingMode == FORCE_DIRECT) {
            triggerSignalImpl(signal);
            return true;
        }

        cgtAssert(signal != 0, "Signal must not be 0.");
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
            _signal_handle_base* signal = nullptr;
            if (_signalQueue.try_pop(signal)) {
                signal->processSignal();
                delete signal;
            }
            else {
                // there currently is no event in this queue -> go sleep
                _evaluationCondition.wait(lock);
            }
        }
    }

    bool signal_manager::isCurrentThreadSignalManagerThread() const {
        return std::this_thread::get_id() == _this_thread_id;
    }

    signal_manager::SignalHandlingMode signal_manager::getSignalHandlingMode() const {
        return _handlingMode;
    }

    void signal_manager::setSignalHandlingMode(SignalHandlingMode mode) {
        _handlingMode = mode;
    }


    const std::string signal_manager::loggerCat_;


    // Implementation inspired by http://stackoverflow.com/questions/7194127/how-should-i-write-iso-c-standard-conformant-custom-new-and-delete-operators/7194149#7194149
    void* _signal_handle_base::operator new(std::size_t size) throw(std::bad_alloc) {
        if (size == 0)
            size = 1;

        while (true) {
            void* toReturn = signal_manager::getRef()._signalPool.malloc(size);

            if (toReturn != nullptr)
                return toReturn;

            //allocation was unsuccessful; find out what the current new-handling function is (see below)
            std::new_handler globalHandler = std::set_new_handler(0);
            std::set_new_handler(globalHandler);

            if (globalHandler)             //If new_hander is registered call it
                (*globalHandler)();
            else 
                throw std::bad_alloc();   //No handler is registered throw an exception
        }
    }

    void _signal_handle_base::operator delete(void* rawMemory, std::size_t size) throw() {
        if (rawMemory == nullptr)
            return;

        signal_manager::getRef()._signalPool.free(rawMemory);
        return;
    }

}
