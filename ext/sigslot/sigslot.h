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

#ifndef SIGSLOT_H
#define SIGSLOT_H


#ifdef CAMPVIS_DYNAMIC_LIBS
    #ifdef SIGSLOT_BUILD_DLL
        // building library -> export symbols
        #ifdef WIN32
            #define SIGSLOT_API __declspec(dllexport)
        #else
            #define SIGSLOT_API
        #endif
    #else
        // including library -> import symbols
        #ifdef WIN32
            #define SIGSLOT_API __declspec(dllimport)
        #else
            #define SIGSLOT_API
        #endif
    #endif
#else
    // building/including static library -> do nothing
    #define SIGSLOT_API
#endif


#include <set>
#include <list>

#include "cgt/assert.h"
#include <ext/threading.h>

#define TBB_PREVIEW_MEMORY_POOL 1
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <tbb/memory_pool.h>
#include <tbb/spin_mutex.h>

#include "ext/cgt/runnable.h"
#include "ext/cgt/singleton.h"

namespace sigslot {
    
    /**
     * List-like container allowing thread-safe bidirectional iteration, insertion and removal of elements. 
     * ATTENTION: Since removed items are internally stored as nullptr, the element type is constrained 
     * to be a pointer type (or at least have pointer semantics) and never be 0. Use with caution!
     */
    template<typename T>
    class concurrent_pointer_list {
    public:
        /// Typedef for internal storage of elements
        typedef tbb::concurrent_vector<T*> StorageType;

        /// Iterator for concurrent_pointer_list
        class concurrent_pointer_list_iterator : public std::iterator<std::bidirectional_iterator_tag, T*> {
        public:
            concurrent_pointer_list_iterator() {};

            concurrent_pointer_list_iterator(StorageType& storage, typename StorageType::iterator position)
                : _begin(storage.begin())
                , _position(position)
                , _end(storage.end())
            {
                if (_position != _end && *_position == nullptr)
                    operator++();
            }

            concurrent_pointer_list_iterator& operator++() {
                do {
                    ++_position;
                } while (_position < _end && *_position == nullptr);
                return *this;
            }

            concurrent_pointer_list_iterator operator++(int) {
                concurrent_pointer_list_iterator toReturn = *this;
                operator++();
                return toReturn;
            }

            concurrent_pointer_list_iterator& operator--() {
                do {
                    --_position;
                } while (_position > _begin && *_position == nullptr);
                return *this;
            }

            concurrent_pointer_list_iterator operator--(int) {
                concurrent_pointer_list_iterator toReturn = *this;
                operator--();
                return toReturn;
            }

            T*& operator*() const {
                return *_position;
            }

            T* operator->() const {
                return &*_position;
            }

            bool operator==(const concurrent_pointer_list_iterator& rhs) const {
                return (_position == rhs._position);
            }

            bool operator!=(const concurrent_pointer_list_iterator& rhs) const {
                return (_position != rhs._position);
            }

        protected:
            typename StorageType::iterator _begin;
            typename StorageType::iterator _position;
            typename StorageType::iterator _end;
        };

        // TODO: As I am currently too lazy to correctly implement const_iterator: I hacked this collection to have a standard iterator as const_iterator.
        typedef concurrent_pointer_list_iterator iterator; ///< iterator typedef
        typedef concurrent_pointer_list_iterator const_iterator; ///< const_iterator typedef 

        /**
         * Creates a new concurrent_pointer_list object of default size
         */
        concurrent_pointer_list() {};

        /**
         * Creates a new concurrent_pointer_list object of default size
         * \param   initalSize  Initial size fo the internal storage
         */
        concurrent_pointer_list(typename StorageType::size_type initalSize) 
            : _storage(initalSize)
        {};


        /// Return an iterator to the beginning of the collection
        iterator begin() { return iterator(_storage, _storage.begin()); };
        /// Return an iterator to the beginning of the collection
        const_iterator begin() const { 
            StorageType& s = const_cast<StorageType&>(_storage);  // const_cast neccessary, because we did not implement const_iterator
            return const_iterator(s, s.begin()); 
        };

        /// Return an iterator to the end of the collection
        iterator end() { return iterator(_storage, _storage.end()); };
        /// Return an iterator to the end of the collection
        const_iterator end() const  { 
            StorageType& s = const_cast<StorageType&>(_storage);  // const_cast neccessary, because we did not implement const_iterator
            return const_iterator(s, s.end()); 
        };

        void push_back(T* element) { insert(element); };
        void insert(T* element);

        size_t erase(T* element);
        void erase(const const_iterator& it) { *it = nullptr; };
        void erase(const const_iterator& begin, const const_iterator& end);

        bool empty() const;

    protected:
        StorageType _storage;
    };

// ================================================================================================

    template<typename T>
    void concurrent_pointer_list<T>::insert(T* element) {
        iterator it = begin();
        iterator itEnd = end();

        // to ensure non-degrading performance, we first fill the gaps.
        for (/* nothing here */; it != itEnd; ++it) {
            if (*it == nullptr) {
                *it = element;
                return;
            }
        }

        // we did not find an empty element -> we add a new one at the end
        _storage.push_back(element);
    }

    template<typename T>
    size_t concurrent_pointer_list<T>::erase(T* element) {
        size_t count = 0;
        iterator it = begin();
        iterator itEnd = end();

        for (/* nothing here */; it != itEnd; ++it) {
            if (*it == element) {
                *it = nullptr;
                ++count;
            }
        }

        return count;
    }

    template<typename T>
    void concurrent_pointer_list<T>::erase(const typename concurrent_pointer_list<T>::const_iterator& begin, const typename concurrent_pointer_list<T>::const_iterator& end) {
        for (const_iterator it = begin; it != end; ++it) {
            *it = nullptr;
        }
    }

    template<typename T>
    bool sigslot::concurrent_pointer_list<T>::empty() const {
        iterator it = begin();
        iterator itEnd = end();

        for (/* nothing here */; it != itEnd; ++it) {
            if (*it != nullptr) {
                return false;
            }
        }

        return true;
    }

// ================================================================================================

    /// Base class for signal handles that provides an interface to emit the signal.
    class SIGSLOT_API _signal_handle_base {
    public:
        // default constructor
        _signal_handle_base()
#ifdef CAMPVIS_DEBUG
            : _callingLine(0)
#endif
        {};

        /// Virtual destructor
        virtual ~_signal_handle_base() {};

        /// Emits the signal of this signal handle.
        virtual void processSignal() const = 0;

        /**
         * Overloading the new operator to create signal handles in signal_manager's memory pool.
         * \param   size    Number of bytes to allocate.
         */
        static void* operator new(std::size_t size) throw(std::bad_alloc);

        /**
         * Overloading the delete operator to correctly remove signal handles from signal_manager's memory pool.
         * \param   rawMemory   Pointer to object to delete
         * \param   size        Number of bytes
         */
        static void operator delete(void* rawMemory, std::size_t size) throw();

#ifdef CAMPVIS_DEBUG
        // This is debug information only, automatically removed from release builds
        std::string _callingFunction;   ///< Function that emitted this signal
        std::string _callingFile;       ///< File which emitted this signal
        int _callingLine;               ///< Line where this signal was emitted
#endif
    };

// ================================================================================================

    /**
     * Singleton class that takes care of queuing and asynchronously dispatching signals.
     * 
     * The signal_manager implements the Runnable interface, i.e. runs in it's own thread once it
     * was launched. The signal_manager takes care of dispatching signals to their connections.
     * This is either done synchrnously by using triggerSignalImpl() or asynchronously by using 
     * queueSignalImpl(). Furthermore it allows to check, whether the current thread is the 
     * signal_manager thread. This allows the default signal emitting method operator() to 
     * automatically decide on the dispatch type based on the emitting thread.
     * 
     * signal_manager can be considered as thread-safe.
     */
    class SIGSLOT_API signal_manager : public cgt::Singleton<signal_manager>, public cgt::RunnableWithConditionalWait {
        friend class cgt::Singleton<signal_manager>;    ///< CRTP
        friend class _signal_handle_base;               ///< so the custom new/delete operator can access the memory pool

    public:
        /// Enumeration of signal handling modes for the signal_manager
        enum SignalHandlingMode {
            DEFAULT,        ///< Default mode is that all signals are queued unless they are emitted from the signal_manager thread.
            FORCE_DIRECT,   ///< Force all signals being directly handled by the emitting thread.
            FORCE_QUEUE     ///< Force all signals being queued and handled by the signal_manager thread.
        };

        /**
         * Returns the signal handling mode of this signal_manager.
         */
        SignalHandlingMode getSignalHandlingMode() const;

        /**
         * Sets the signal handling mode of this signal_manager to \mode.
         * \param   mode    New signal handling mode to set.
         */
        void setSignalHandlingMode(SignalHandlingMode mode);

        /**
         * Directly dispatches the signal \a signal to all currently registered listeners.
         * \note    For using threaded signal dispatching use queueSignalImpl()
         * \param   signal   signal to dispatch
         */
        void triggerSignalImpl(_signal_handle_base* signal);

        /**
         * Enqueue \a signal into the list of signals to be dispatched.
         * \note    Dispatch will be performed in signal_mananger thread. For direct dispatch in
         *          caller thread use triggerSignalImpl().
         * \param   signal   signal to dispatch
         * \return  True, if \a signal was successfully enqueued to signal queue.
         */
        bool queueSignalImpl(_signal_handle_base* signal);

        /**
         * Checks whether calling thread is signal_manager thread.
         * \return  std::this_thread::get_id() == _this_thread_id
         */
        bool isCurrentThreadSignalManagerThread() const;

        /// \see Runnable:run
        virtual void run();

    private:
        /// Private constructor only for singleton
        signal_manager();
        /// Private destructor only for singleton
        ~signal_manager();

        /// Typedef for the signal queue
        typedef tbb::concurrent_queue<_signal_handle_base*> SignalQueue;

        SignalHandlingMode _handlingMode;               ///< Mode for handling signals
        SignalQueue _signalQueue;                       ///< Queue for signals to be dispatched

        std::mutex _ecMutex;                            ///< Mutex for protecting _evaluationCondition
        std::thread::id _this_thread_id;

        typedef std::allocator<_signal_handle_base> pool_allocator_t;
        tbb::memory_pool<pool_allocator_t> _signalPool; ///< Memory pool for the signals

        static const std::string loggerCat_;
    };

// ================================================================================================

    
    class SIGSLOT_API has_slots;
    
    
    class SIGSLOT_API _connection_base0
    {
    public:
        virtual ~_connection_base0() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal() = 0;
        virtual _connection_base0* clone() = 0;
        virtual _connection_base0* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type>
    class _connection_base1
    {
    public:
        virtual ~_connection_base1() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal(arg1_type) = 0;
        virtual _connection_base1<arg1_type>* clone() = 0;
        virtual _connection_base1<arg1_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type>
    class _connection_base2
    {
    public:
        virtual ~_connection_base2() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal(arg1_type, arg2_type) = 0;
        virtual _connection_base2<arg1_type, arg2_type>* clone() = 0;
        virtual _connection_base2<arg1_type, arg2_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type>
    class _connection_base3
    {
    public:
        virtual ~_connection_base3() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal(arg1_type, arg2_type, arg3_type) = 0;
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* clone() = 0;
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _connection_base4
    {
    public:
        virtual ~_connection_base4() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal(arg1_type, arg2_type, arg3_type, arg4_type) = 0;
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* clone() = 0;
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _connection_base5
    {
    public:
        virtual ~_connection_base5() {}
        virtual has_slots* getdest() const = 0;
        virtual void processSignal(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) = 0;
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* clone() = 0;
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
// ================================================================================================
    
    
    class SIGSLOT_API _signal_base
    {
    public:
        virtual void slot_disconnect(has_slots* pslot) = 0;
        virtual void slot_duplicate(has_slots const* poldslot, has_slots* pnewslot) = 0;

#ifdef CAMPVIS_DEBUG
    protected:
        // This is debug information only, automatically removed from release builds
        // As we're using local variables, we have to protect them with the mutex. This is neither
        // beautiful nor efficient, but as it's for debug-only code, we should be able to live with it.
        // In a side note: This mutex is sufficient to avoid segfaults or other crashes. However, it
        //                 does not guarantee the correct order of execution. Hence, there exist cases
        //                 where the debug information stored in the signal handle is not correct. 
        //                 This is a risk we can take, as the chance that the *very same* signal 
        //                 gets emitted concurrently form different threads is very low.
        tbb::spin_mutex _debugMutex;    ///< Mutex to protect these three variables
        
        std::string _callingFunction;   ///< Function that emitted the last signal
        std::string _callingFile;       ///< File which emitted the last signal
        int _callingLine;               ///< Line where the last signal was emitted

        // In debug mode, this macro writes the caller information to the signal handle
        #define writeDebugInfoToSignalHandle(_the_signal_handle) \
            { \
            tbb::spin_mutex::scoped_lock lock(this->_debugMutex); \
            _the_signal_handle->_callingFunction = this->_callingFunction; \
            _the_signal_handle->_callingFile = this->_callingFile; \
            _the_signal_handle->_callingLine = this->_callingLine; \
            }

        // In debug mode, check that there is no connection to _target_slot so far.
        #define assertSingleConnectionToDestination(_connection_type, _target_object, _target_func) \
            typename connections_list::const_iterator it = this->m_connected_slots.begin(); \
            typename connections_list::const_iterator itEnd = this->m_connected_slots.end(); \
            while (it != itEnd) { \
                cgtAssert((*it)->getdest() != _target_object || static_cast<_connection_type*>(*it)->getfunc() != _target_func, "Connected the same slot twice, you should not do this."); \
                ++it; \
            }            

#else
        // In release mode, these macros expand to a NOP.
        #define writeDebugInfoToSignalHandle(_the_signal_handle)
        #define assertSingleConnectionToDestination(_connection_type, _target_object, _target_func)
#endif
    };

    class SIGSLOT_API has_slots
    {
    private:
        typedef concurrent_pointer_list<_signal_base> sender_set;
        typedef sender_set::const_iterator const_iterator;
        
    public:
        has_slots() {}
        
        has_slots(has_slots const& hs)
        {
            const_iterator it = hs.m_senders.begin();
            const_iterator itEnd = hs.m_senders.end();
            
            while (it != itEnd) {
                (*it)->slot_duplicate(&hs, this);
                m_senders.insert(*it);
                ++it;
            }
        } 
        
        void signal_connect(_signal_base* sender)
        {
            m_senders.insert(sender);
        }
        
        void signal_disconnect(_signal_base* sender)
        {
            m_senders.erase(sender);
        }
        
        virtual ~has_slots()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            const_iterator it = m_senders.begin();
            const_iterator itEnd = m_senders.end();
            
            while (it != itEnd) {
                (*it)->slot_disconnect(this);
                ++it;
            }
            
            m_senders.erase(m_senders.begin(), m_senders.end());
        }
        
    private:
        sender_set m_senders;
    };
    
    
    class _signal_base0 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base0 >  connections_list;
        
        _signal_base0() {}
        
        _signal_base0(_signal_base0 const& s) : _signal_base(s)
        {
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
            }
        }
        
        ~_signal_base0()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            connections_list::const_iterator it = m_connected_slots.begin();
            connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            connections_list::iterator it = m_connected_slots.begin();
            connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            connections_list::iterator it = m_connected_slots.begin();
            connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }
        
        void slot_duplicate(has_slots const* oldtarget, has_slots* newtarget)
        {
            connections_list::iterator it = m_connected_slots.begin();
            connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
    protected:
        connections_list m_connected_slots;   
    };
    
    template<class arg1_type>
    class _signal_base1 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base1<arg1_type> >  connections_list;
        
        _signal_base1() {}
        
        _signal_base1(_signal_base1<arg1_type> const& s) : _signal_base(s)
        {
            typename connections_list::const_iterator it = s.m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
            }
        }
        
        void slot_duplicate(has_slots const* oldtarget, has_slots* newtarget)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }
        
        ~_signal_base1()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            typename connections_list::const_iterator it = m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                typename connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
        
    protected:
        connections_list m_connected_slots;   
    };
    
    template<class arg1_type, class arg2_type>
    class _signal_base2 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base2<arg1_type, arg2_type> >  connections_list;
        
        _signal_base2() {}
        
        _signal_base2(_signal_base2<arg1_type, arg2_type> const& s) : _signal_base(s)
        {
            typename connections_list::const_iterator it = s.m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
            }
        }
        
        void slot_duplicate(has_slots const* oldtarget, has_slots* newtarget)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }
        
        ~_signal_base2()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            typename connections_list::const_iterator it = m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                typename connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
    protected:
        connections_list m_connected_slots;   
    };
    
    template<class arg1_type, class arg2_type, class arg3_type>
    class _signal_base3 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base3<arg1_type, arg2_type, arg3_type> >  connections_list;
         
        _signal_base3() {}
        
        _signal_base3(_signal_base3<arg1_type, arg2_type, arg3_type> const& s)
        : _signal_base(s)
        {
            typename connections_list::const_iterator it = s.m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
            }
        }
        
        void slot_duplicate(has_slots const* oldtarget, has_slots* newtarget)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }
        
        ~_signal_base3()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            typename connections_list::const_iterator it = m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                typename connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
    protected:
        connections_list m_connected_slots;   
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _signal_base4 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base4<arg1_type, arg2_type, arg3_type, arg4_type> >  connections_list;
        
        _signal_base4() {}
        
        _signal_base4(const _signal_base4<arg1_type, arg2_type, arg3_type, arg4_type>& s)
        : _signal_base(s)
        {
            typename connections_list::const_iterator it = s.m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                
                ++it;
            }
        }
        
        void slot_duplicate(const has_slots* oldtarget, has_slots* newtarget)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }
        
        ~_signal_base4()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            typename connections_list::const_iterator it = m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                typename connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
    protected:
        connections_list m_connected_slots;   
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _signal_base5 : public _signal_base
    {
    public:
        typedef concurrent_pointer_list<_connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> >  connections_list;
        
        _signal_base5() {}
        
        _signal_base5(const _signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& s)
        : _signal_base(s)
        {
            typename connections_list::const_iterator it = s.m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                
                ++it;
            }
        }
        
        void slot_duplicate(const has_slots* oldtarget, has_slots* newtarget)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == oldtarget) {
                    m_connected_slots.push_back((*it)->duplicate(newtarget));
                }
                
                ++it;
            }
        }
        
        ~_signal_base5()
        {
            disconnect_all();
        }
        
        void disconnect_all()
        {
            typename connections_list::const_iterator it = m_connected_slots.begin();
            typename connections_list::const_iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;
                
                ++it;
            }
            
            m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
        }
        
        void disconnect(has_slots* pclass)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    m_connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }
                
                ++it;
            }
        }
        
        void slot_disconnect(has_slots* pslot)
        {
            typename connections_list::iterator it = m_connected_slots.begin();
            typename connections_list::iterator itEnd = m_connected_slots.end();
            
            while (it != itEnd) {
                typename connections_list::iterator itNext = it;
                ++itNext;
                
                if ((*it)->getdest() == pslot) {
                    m_connected_slots.erase(it);
                    //          delete *it;
                }
                
                it = itNext;
            }
        }

        bool has_connections() const
        {
            return !m_connected_slots.empty();
        }
        
    protected:
        connections_list m_connected_slots;   
    };
    
// ================================================================================================

    template<class dest_type>
    class _connection0 : public _connection_base0
    {
    public:
        typedef void (dest_type::*function_ptr_t)();

        _connection0()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection0(dest_type* pobject, void (dest_type::*pmemfun)())
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base0* clone()
        {
            return new _connection0<dest_type>(*this);
        }
        
        virtual _connection_base0* duplicate(has_slots* pnewdest)
        {
            return new _connection0<dest_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal()
        {
            (m_pobject->*m_pmemfun)();
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }

        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t m_pmemfun;
    };
    
    template<class dest_type, class arg1_type>
    class _connection1 : public _connection_base1<arg1_type>
    {
    public:
        typedef void (dest_type::*function_ptr_t)(arg1_type);

        _connection1()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection1(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type))
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base1<arg1_type>* clone()
        {
            return new _connection1<dest_type, arg1_type>(*this);
        }
        
        virtual _connection_base1<arg1_type>* duplicate(has_slots* pnewdest)
        {
            return new _connection1<dest_type, arg1_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal(arg1_type a1)
        {
            (m_pobject->*m_pmemfun)(a1);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }

        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t  m_pmemfun;
    };
    
    template<class dest_type, class arg1_type, class arg2_type>
    class _connection2 : public _connection_base2<arg1_type, arg2_type>
    {
    public:
        typedef void (dest_type::* function_ptr_t)(arg1_type, arg2_type);

        _connection2()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection2(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type,
                                                                    arg2_type))
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base2<arg1_type, arg2_type>* clone()
        {
            return new _connection2<dest_type, arg1_type, arg2_type>(*this);
        }
        
        virtual _connection_base2<arg1_type, arg2_type>* duplicate(has_slots* pnewdest)
        {
            return new _connection2<dest_type, arg1_type, arg2_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal(arg1_type a1, arg2_type a2)
        {
            (m_pobject->*m_pmemfun)(a1, a2);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t m_pmemfun;
    };

    template<class dest_type, class arg1_type, class arg2_type, class arg3_type>
    class _connection3 : public _connection_base3<arg1_type, arg2_type, arg3_type>
    {
    public:
        typedef void (dest_type::* function_ptr_t)(arg1_type, arg2_type, arg3_type);

        _connection3()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection3(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type,
                                                                    arg2_type, arg3_type))
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* clone()
        {
            return new _connection3<dest_type, arg1_type, arg2_type, arg3_type>(*this);
        }
        
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* duplicate(has_slots* pnewdest)
        {
            return new _connection3<dest_type, arg1_type, arg2_type, arg3_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }

        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t m_pmemfun;
    };
    
    template<class dest_type, class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _connection4 : public _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>
    {
    public:
        typedef void (dest_type::* function_ptr_t)(arg1_type, arg2_type, arg3_type, arg4_type);

        _connection4()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection4(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type))
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* clone()
        {
            return new _connection4<dest_type, arg1_type, arg2_type, arg3_type, arg4_type>(*this);
        }
        
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* duplicate(has_slots* pnewdest)
        {
            return new _connection4<dest_type, arg1_type, arg2_type, arg3_type, arg4_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3, a4);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }

        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t m_pmemfun;
    };
    
    template<class dest_type, class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _connection5 : public _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>
    {
    public:
        typedef void (dest_type::* function_ptr_t)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type);

        _connection5()
        {
            m_pobject = 0;
            m_pmemfun = 0;
        }
        
        _connection5(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type))
        {
            m_pobject = pobject;
            m_pmemfun = pmemfun;
        }
        
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* clone()
        {
            return new _connection5<dest_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(*this);
        }
        
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* duplicate(has_slots* pnewdest)
        {
            return new _connection5<dest_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>((dest_type *)pnewdest, m_pmemfun);
        }
        
        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3, a4, a5);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
        function_ptr_t getfunc() const 
        {
            return m_pmemfun;
        }

    private:
        dest_type* m_pobject;
        function_ptr_t m_pmemfun;
    };
    
// ================================================================================================

    
    class signal0 : public _signal_base0
    {
    public:
        typedef _signal_base0 base;
        typedef base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle0 : public _signal_handle_base {
        public:
            signal_handle0(signal0* sender)
                : _sender(sender)
            {};

            virtual ~signal_handle0() {};

            // override
            void processSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal();
                    it = itNext;
                }
            };

            signal0* _sender;
        };


        signal0() {}
        
        signal0(const signal0& s)
        : _signal_base0(s) {}

        virtual ~signal0() {}
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)())
        {
            typedef _connection0<desttype> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal()
        {
            signal_handle0* sh = new signal_handle0(this);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().triggerSignalImpl(sh);
        }
        
        void queueSignal()
        {
            signal_handle0* sh = new signal_handle0(this);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().queueSignalImpl(sh);
        }

        void emitSignal()
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal();
            else
                queueSignal();
        }

        void operator()()
        {
            emitSignal();
        }
    };

    template<class arg1_type>
    class signal1 : public _signal_base1<arg1_type>
    {
    public:
        typedef _signal_base1<arg1_type> base;
        typedef typename base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle1 : public _signal_handle_base {
        public:
            signal_handle1(signal1<arg1_type>* sender, arg1_type a1)
                : _sender(sender)
                , _a1(a1)
            {};

            virtual ~signal_handle1() {};

            // override
            void processSignal() const {
                typename connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                typename connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal(_a1);
                    it = itNext;
                }
            };

            signal1<arg1_type>* _sender;
            arg1_type _a1;
        };

        signal1() {}
        
        signal1(const signal1<arg1_type>& s)
        : _signal_base1<arg1_type>(s) {}

        virtual ~signal1() {}
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type))
        {
            typedef _connection1<desttype, arg1_type> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal(arg1_type a1)
        {
            signal_handle1* sh = new signal_handle1(this, a1);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().triggerSignalImpl(sh);
        }

        void queueSignal(arg1_type a1)
        {
            signal_handle1* sh = new signal_handle1(this, a1);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().queueSignalImpl(sh);
        }

        void emitSignal(arg1_type a1)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal(a1);
            else
                queueSignal(a1);
        }

        void operator()(arg1_type a1)
        {
            emitSignal(a1);
        }
    };
    
    template<class arg1_type, class arg2_type>
    class signal2 : public _signal_base2<arg1_type, arg2_type>
    {
    public:
        typedef _signal_base2<arg1_type, arg2_type> base;
        typedef typename base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle2 : public _signal_handle_base {
        public:
            signal_handle2(signal2<arg1_type, arg2_type>* sender, arg1_type a1, arg2_type a2)
                : _sender(sender)
                , _a1(a1)
                , _a2(a2)
            {};

            virtual ~signal_handle2() {};

            // override
            void processSignal() const {
                typename connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                typename connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal(_a1, _a2);
                    it = itNext;
                }
            };

            signal2<arg1_type, arg2_type>* _sender;
            arg1_type _a1;
            arg2_type _a2;
        };

        signal2() {}
        
        signal2(const signal2<arg1_type, arg2_type>& s)
        : _signal_base2<arg1_type, arg2_type>(s) {}

        virtual ~signal2() {}

        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type,
                                                                 arg2_type))
        {
            typedef _connection2<desttype, arg1_type, arg2_type> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal(arg1_type a1, arg2_type a2)
        {
            signal_handle2* sh = new signal_handle2(this, a1, a2);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().triggerSignalImpl(sh);
        }

        void queueSignal(arg1_type a1, arg2_type a2)
        {
            signal_handle2* sh = new signal_handle2(this, a1, a2);
            writeDebugInfoToSignalHandle(sh);
            signal_manager::getRef().queueSignalImpl(sh);
        }
        
        void emitSignal(arg1_type a1, arg2_type a2)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal(a1, a2);
            else
                queueSignal(a1, a2);
        }

        void operator()(arg1_type a1, arg2_type a2)
        {
            emitSignal(a1, a2);
        }
    };
    
    template<class arg1_type, class arg2_type, class arg3_type>
    class signal3 : public _signal_base3<arg1_type, arg2_type, arg3_type>
    {
    public:
        typedef _signal_base3<arg1_type, arg2_type, arg3_type> base;
        typedef typename base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle3 : public _signal_handle_base {
        public:
            signal_handle3(signal3<arg1_type, arg2_type, arg3_type>* sender, arg1_type a1, arg2_type a2, arg3_type a3)
                : _sender(sender)
                , _a1(a1)
                , _a2(a2)
                , _a3(a3)
            {};

            virtual ~signal_handle3() {};

            // override
            void processSignal() const {
                typename connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                typename connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal(_a1, _a2, _a3);
                    it = itNext;
                }
            };

            signal3<arg1_type, arg2_type, arg3_type>* _sender;
            arg1_type _a1;
            arg2_type _a2;
            arg3_type _a3;
        };

        signal3() {}
        
        signal3(const signal3<arg1_type, arg2_type, arg3_type>& s)
        : _signal_base3<arg1_type, arg2_type, arg3_type>(s) {}

        virtual ~signal3() {}

        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type,
                                                                 arg2_type, arg3_type))
        {
            typedef _connection3<desttype, arg1_type, arg2_type, arg3_type> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            signal_handle3* sh = new signal_handle3(this, a1, a2, a3);
            signal_manager::getRef().triggerSignalImpl(sh);
        }

        void queueSignal(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            signal_handle3* sh = new signal_handle3(this, a1, a2, a3);
            signal_manager::getRef().queueSignalImpl(sh);
        }
        
        void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal(a1, a2, a3);
            else
                queueSignal(a1, a2, a3);
        }

        void operator()(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            emitSignal(a1, a2, a3);
        }
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class signal4 : public _signal_base4<arg1_type, arg2_type, arg3_type, arg4_type>
    {
    public:
        typedef _signal_base4<arg1_type, arg2_type, arg3_type, arg4_type> base;
        typedef typename base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle4 : public _signal_handle_base {
        public:
            signal_handle4(signal4<arg1_type, arg2_type, arg3_type, arg4_type>* sender, arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
                : _sender(sender)
                , _a1(a1)
                , _a2(a2)
                , _a3(a3)
                , _a4(a4)
            {};

            virtual ~signal_handle4() {};

            // override
            void processSignal() const {
                typename connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                typename connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal(_a1, _a2, _a3, _a4);
                    it = itNext;
                }
            };

            signal4<arg1_type, arg2_type, arg3_type, arg4_type>* _sender;
            arg1_type _a1;
            arg2_type _a2;
            arg3_type _a3;
            arg4_type _a4;
        };

        signal4() {}
        
        signal4(const signal4<arg1_type, arg2_type, arg3_type, arg4_type>& s)
        : _signal_base4<arg1_type, arg2_type, arg3_type, arg4_type>(s) {}

        virtual ~signal4() {}

        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type))
        {
            typedef _connection4<desttype, arg1_type, arg2_type, arg3_type, arg4_type> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            signal_handle4* sh = new signal_handle4(this, a1, a2, a3, a4);
            signal_manager::getRef().triggerSignalImpl(sh);
        }

        void queueSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            signal_handle4* sh = new signal_handle4(this, a1, a2, a3, a4);
            signal_manager::getRef().queueSignalImpl(sh);
        }
        
        void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal(a1, a2, a3, a4);
            else
                queueSignal(a1, a2, a3, a4);
        }

        void operator()(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            emitSignal(a1, a2, a3, a4);
        }
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class signal5 : public _signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>
    {
    public:
        typedef _signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> base;
        typedef typename base::connections_list connections_list;
        using base::m_connected_slots;

        class signal_handle5 : public _signal_handle_base {
        public:
            signal_handle5(signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* sender, arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
                : _sender(sender)
                , _a1(a1)
                , _a2(a2)
                , _a3(a3)
                , _a4(a4)
                , _a5(a5)
            {};

            virtual ~signal_handle5() {};

            // override
            void processSignal() const {
                typename connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                typename connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->processSignal(_a1, _a2, _a3, _a4, _a5);
                    it = itNext;
                }
            };

            signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* _sender;
            arg1_type _a1;
            arg2_type _a2;
            arg3_type _a3;
            arg4_type _a4;
            arg5_type _a5;
        };

        signal5() {}
        
        signal5(const signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& s)
        : _signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(s) {}

        virtual ~signal5() {}

        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type))
        {
            typedef _connection5<desttype, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> connection_type;
            assertSingleConnectionToDestination(connection_type, pclass, pmemfun);
            connection_type* conn = new connection_type(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void triggerSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            signal_handle5* sh = new signal_handle5(this, a1, a2, a3, a4, a5);
            signal_manager::getRef().triggerSignalImpl(sh);
        }

        void queueSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            signal_handle5* sh = new signal_handle5(this, a1, a2, a3, a4, a5);
            signal_manager::getRef().queueSignalImpl(sh);
        }
        
        void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                triggerSignal(a1, a2, a3, a4, a5);
            else
                queueSignal(a1, a2, a3, a4, a5);
        }

        void operator()(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            emitSignal(a1, a2, a3, a4, a5);
        }
    };


#ifdef CAMPVIS_DEBUG
    // These are decorator classes (non strictly speaking) to add debug functionality to signals.
    // They overload the emitSignal() method with a version that stores calling function, file and line.
    // Together with the macro magic further down, this provides us with a clean, non-intrusive way
    // to automatically enhance all client calls to emitSignal with additional debug information.

    // In debug mode, this macro writes the caller information to the signal handle
    #define writeDebugInfoToSignal() \
        tbb::spin_mutex::scoped_lock lock(this->_debugMutex); \
        this->_callingFunction = caller; \
        this->_callingFile = file; \
        this->_callingLine = line;

    //
    // Implementaion inspired by http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
    class signal0_debug_decorator : public signal0 {
    public:
        signal0_debug_decorator() {}
        signal0_debug_decorator(const signal0_debug_decorator& s) : signal0(s) {}
        virtual ~signal0_debug_decorator() {}

        signal0& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal0& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal0& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    template<class arg1_type>
    class signal1_debug_decorator : public signal1<arg1_type> {
    public:
        signal1_debug_decorator() {}
        signal1_debug_decorator(const signal1_debug_decorator<arg1_type>& s) : signal1<arg1_type>(s) {}
        virtual ~signal1_debug_decorator() {}

        signal1<arg1_type>& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal1<arg1_type>& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal1<arg1_type>& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    template<class arg1_type, class arg2_type>
    class signal2_debug_decorator : public signal2<arg1_type, arg2_type> {
    public:
        signal2_debug_decorator() {}
        signal2_debug_decorator(const signal2_debug_decorator<arg1_type, arg2_type>& s) : signal2<arg1_type, arg2_type>(s) {}
        virtual ~signal2_debug_decorator() {}

        signal2<arg1_type, arg2_type>& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal2<arg1_type, arg2_type>& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal2<arg1_type, arg2_type>& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type>
    class signal3_debug_decorator : public signal3<arg1_type, arg2_type, arg3_type> {
    public:
        signal3_debug_decorator() {}
        signal3_debug_decorator(const signal3_debug_decorator<arg1_type, arg2_type, arg3_type>& s) : signal3<arg1_type, arg2_type, arg3_type>(s) {}
        virtual ~signal3_debug_decorator() {}

        signal3<arg1_type, arg2_type, arg3_type>& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal3<arg1_type, arg2_type, arg3_type>& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal3<arg1_type, arg2_type, arg3_type>& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class signal4_debug_decorator : public signal4<arg1_type, arg2_type, arg3_type, arg4_type> {
    public:
        signal4_debug_decorator() {}
        signal4_debug_decorator(const signal4_debug_decorator<arg1_type, arg2_type, arg3_type, arg4_type>& s) : signal4<arg1_type, arg2_type, arg3_type, arg4_type>(s) {}
        virtual ~signal4_debug_decorator() {}

        signal4<arg1_type, arg2_type, arg3_type, arg4_type>& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal4<arg1_type, arg2_type, arg3_type, arg4_type>& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal4<arg1_type, arg2_type, arg3_type, arg4_type>& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class signal5_debug_decorator : public signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> {
    public:
        signal5_debug_decorator() {}
        signal5_debug_decorator(const signal5_debug_decorator<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& s) : signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(s) {}
        virtual ~signal5_debug_decorator() {}

        signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& triggerSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& queueSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
        signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& emitSignal(std::string caller, std::string file, int line) {
            writeDebugInfoToSignal();
            return *this;
        }
    };

    // redefine all necessary symbols
    #undef signal0
    #define signal0 signal0_debug_decorator
    #undef signal1
    #define signal1 signal1_debug_decorator
    #undef signal2
    #define signal2 signal2_debug_decorator
    #undef signal3
    #define signal3 signal3_debug_decorator
    #undef signal4
    #define signal4 signal4_debug_decorator
    #undef signal5
    #define signal5 signal5_debug_decorator

    #undef triggerSignal
    #define triggerSignal triggerSignal(__FUNCTION__, __FILE__, __LINE__).triggerSignal
    #undef queueSignal
    #define queueSignal queueSignal(__FUNCTION__, __FILE__, __LINE__).queueSignal
    #undef emitSignal
    #define emitSignal emitSignal(__FUNCTION__, __FILE__, __LINE__)

#endif

} // namespace sigslot

#endif // SIGSLOT_H
