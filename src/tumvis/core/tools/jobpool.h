// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef JOBPOOL_H__
#define JOBPOOL_H__

#include "sigslot/sigslot.h"
#include "tbb/include/tbb/concurrent_queue.h"

namespace TUMVis {

    class AbstractJob;

    /**
     * Enumeration of the different priorities of items.
     */
    enum PriorityPoolPriority {
        Realtime = 0,   ///< Realtime items are always considered first during dequeueing.
        Normal = 1,     ///< Items with normal priorities are dequeued as soon as there are no realtime items left
        Low = 2         ///< Low priority items are only considered if there are no items in the queue with higher priority
    };

    /**
     * A PriorityPool manages multiple items of type T in queues with different priorities.
     * Similar to a priority queue but different...
     * 
     * \note    This class is to be considered as thread-safe.
     * \todo    Implement a suitable scheduling strategy to avoid starving of low priority items.
     *          This sounds like a good opportunity to take a look at the Betriebssysteme lecture slides. :)
     */
    template<class T>
    class PriorityPool {
    public:

        /**
         * Creates a new PriorityPool
         */
        PriorityPool();

        /**
         * Destructor, deletes all items which are still enqueued.
         */
        ~PriorityPool();

        /**
         * Enqueues the given Job with the given priority.
         * 
         * \note    PriorityPool takes ownership of \a item.
         * \param   item        Item to enqueue, PriorityPool takes ownership of this Job!
         * \param   priority    Priority of the item to enqueue
         */
        void enqueueJob(T* item, PriorityPoolPriority priority);

        /**
         * Dequeues the next item according to the scheduling strategy.
         * \note    The calling function takes the ownership of the returned item!
         * \todo    Develop a good scheduling strategy and describe it here.
         * \return  The next item to execute, 0 if there is currently no item to execute. The caller takes ownership of the item!
         */
        T* dequeueJob();

        /// Signal being emitted, when a item has been enqueued.
        sigslot::signal0<> s_enqueuedJob;

    protected:
        static const size_t NUM_PRIORITIES;     ///< total number of piorities, make sure that this matches the Priority enum.
        tbb::concurrent_queue<T*>* _queues;     ///< Array of item queues, one for each Priority
    };

// ================================================================================================

    template<class T>
    const size_t TUMVis::PriorityPool<T>::NUM_PRIORITIES = 3;

    template<class T>
    TUMVis::PriorityPool<T>::PriorityPool()
    {
        _queues = new tbb::concurrent_queue<T*>[NUM_PRIORITIES];
    }

    template<class T>
    TUMVis::PriorityPool<T>::~PriorityPool() {
        // delete jobs
        T* toDelete = 0;
        for (size_t i = 0; i < NUM_PRIORITIES; ++i) {
            while (_queues[i].try_pop(toDelete))
                delete toDelete;
        }

        // delete queues
        delete[] _queues;
    }

    template<class T>
    void TUMVis::PriorityPool<T>::enqueueJob(T* item, PriorityPoolPriority priority) {
        size_t i = static_cast<size_t>(priority);
        tgtAssert(i < NUM_PRIORITIES, "Item priority index must be lower than the total number or priorities.");
        tgtAssert(item != 0, "Item must not be 0");

        _queues[i].push(item);
        s_enqueuedJob();
    }

    template<class T>
    T* TUMVis::PriorityPool<T>::dequeueJob() {
        // very simple scheduling algorithm. This should be made fairer and avoid starving!
        T* toReturn = 0;
        for (size_t i = 0; i < NUM_PRIORITIES; ++i) {
            if (_queues[i].try_pop(toReturn))
                return toReturn;
        }

        return 0;
    }
}

#endif // JOBPOOL_H__