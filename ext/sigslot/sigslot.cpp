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
        std::unique_lock<tbb::mutex> lock(_ecMutex);

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
