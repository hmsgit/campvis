/*
 
 sigslot.h: Signal/Slot classes
 
 Written by Sarah Thompson (sarah@telergy.com) 2002.
 
 License: Public domain. You are free to use this code however you like,
          with the proviso that the author takes on no responsibility or
          liability for any use.
 
 QUICK DOCUMENTATION
 
 (see also the full documentation at http://sigslot.sourceforge.net/)
 
 1. #define switches

    SIGSLOT_PURE_ISO:
    Define this to force ISO C++ compliance. This also disables
    all of the thread safety support on platforms where it is 
    available.
 
    SIGSLOT_USE_POSIX_THREADS:
    Force use of Posix threads when using a C++ compiler other than
    gcc on a platform that supports Posix threads. (When using gcc,
    this is the default - use SIGSLOT_PURE_ISO to disable this if 
    necessary)
 
    SIGSLOT_DEFAULT_MT_POLICY:
    Where thread support is enabled, this defaults to multi_threaded_global.
    Otherwise, the default is single_threaded. #define this yourself to
    override the default. In pure ISO mode, anything other than
    single_threaded will cause a compiler error.

 2. PLATFORM NOTES
 
    Win32:
    On Win32, the WIN32 symbol must be #defined. Most mainstream
    compilers do this by default, but you may need to define it
    yourself if your build environment is less standard. This causes
    the Win32 thread support to be compiled in and used automatically.
 
    Unix/Linux/BSD, etc:
    If you're using gcc, it is assumed that you have Posix threads
    available, so they are used automatically. You can override this
    (as under Windows) with the SIGSLOT_PURE_ISO switch. If you're using
    something other than gcc but still want to use Posix threads, you
    need to #define SIGSLOT_USE_POSIX_THREADS.
 
    ISO C++:
    If none of the supported platforms are detected, or if
    SIGSLOT_PURE_ISO is defined, all multithreading support is turned off,
    along with any code that might cause a pure ISO C++ environment to
    complain. Before you ask, gcc -ansi -pedantic won't compile this 
    library, but gcc -ansi is fine. Pedantic mode seems to throw a lot of
    errors that aren't really there. If you feel like investigating this,
    please contact the author.
      
 THREADING MODES
 
    single_threaded:
    Your program is assumed to be single threaded from the point of view
    of signal/slot usage (i.e. all objects using signals and slots are
    created and destroyed from a single thread). Behaviour if objects are
    destroyed concurrently is undefined (i.e. you'll get the occasional
    segmentation fault/memory exception).
 
    multi_threaded_global:
    Your program is assumed to be multi threaded. Objects using signals and
    slots can be safely created and destroyed from any thread, even when
    connections exist. In multi_threaded_global mode, this is achieved by a
    single global mutex (actually a critical section on Windows because they
    are faster). This option uses less OS resources, but results in more
    opportunities for contention, possibly resulting in more context switches
    than are strictly necessary.
 
    multi_threaded_local:
    Behaviour in this mode is essentially the same as multi_threaded_global,
    except that each signal, and each object that inherits has_slots, all 
    have their own mutex/critical section. In practice, this means that
    mutex collisions (and hence context switches) only happen if they are
    absolutely essential. However, on some platforms, creating a lot of 
    mutexes can slow down the whole OS, so use this option with care.
 
 USING THE LIBRARY
 
 See the full documentation at http://sigslot.sourceforge.net/
 
*/

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

#include "tgt/assert.h"
#include <tbb/compat/condition_variable>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>

#include "ext/tgt/runnable.h"
#include "ext/tgt/singleton.h"

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
                tgtAssert(_position != _end, "Trying to advance an iterator past the end!");
                do {
                    ++_position;
                } while (_position != _end && *_position == nullptr);
                return *this;
            }

            concurrent_pointer_list_iterator operator++(int) {
                concurrent_pointer_list_iterator toReturn = *this;
                operator++();
                return toReturn;
            }

            concurrent_pointer_list_iterator& operator--() {
                tgtAssert(_position != _begin, "Trying to advance an iterator befor the begin!");
                do {
                    --_position;
                } while (_position != _begin && *_position == nullptr);
                return *this;
            }

            concurrent_pointer_list_iterator operator--(int) {
                concurrent_pointer_list_iterator toReturn = *this;
                operator--();
                return toReturn;
            }

            reference operator*() const {
                return *_position;
            }

            pointer operator->() const {
                return &*_position;
            }

            bool operator==(const concurrent_pointer_list_iterator& rhs) const {
                return (_begin == rhs._begin && _position == rhs._position && _end == rhs._end);
            }

            bool operator!=(const concurrent_pointer_list_iterator& rhs) const {
                return (_begin != rhs._begin || _position != rhs._position || _end != rhs._end);
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

        void push_back(T* element) { _storage.push_back(element); };
        void insert(T* element) { _storage.push_back(element); };

        size_t erase(T* element);
        void erase(const const_iterator& it) { *it = nullptr; };
        void erase(const const_iterator& begin, const const_iterator& end);

        bool empty() const;

    protected:
        StorageType _storage;
    };

// ================================================================================================
    
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
    class _signal_handle_base {
    public:
        /// Virtual destructor
        virtual ~_signal_handle_base() {};
        /// Emits the signal of this signal handle.
        virtual void emitSignal() const = 0;
    };

// ================================================================================================

    /**
     * Singleton class that takes care of queuing and asynchronously dispatching signals.
     * 
     * The signal_manager implements the Runnable interface, i.e. runs in it's own thread once it
     * was launched. The signal_manager takes care of dispatching signals to their connections.
     * This is either done synchrnously by using triggerSignal() or asynchronously by using 
     * queueSignal(). Furthermore it allows to check, whether the current thread is the 
     * signal_manager thread. This allows the default signal emitting method operator() to 
     * automatically decide on the dispatch type based on the emitting thread.
     * 
     * signal_manager can be considered as thread-safe.
     */
    class SIGSLOT_API signal_manager : public tgt::Singleton<signal_manager>, public tgt::Runnable {
        friend class tgt::Singleton<signal_manager>;

    public:
        /**
         * Directly dispatches the signal \a signal to all currently registered listeners.
         * \note    For using threaded signal dispatching use queueSignal()
         * \param   signal   signal to dispatch
         */
        void triggerSignal(_signal_handle_base* signal) const;

        /**
         * Enqueue \a signal into the list of signals to be dispatched.
         * \note    Dispatch will be perfomed in signal_mananger thread. For direct dispatch in
         *          caller thread use triggerSignal().
         * \param   signal   signal to dispatch
         * \return  True, if \a signal was successfully enqueued to signal queue.
         */
        bool queueSignal(_signal_handle_base* signal);

        /**
         * Checks whether calling thread is signal_manager thread.
         * \return  std::this_thread::get_id() == _this_thread_id
         */
        bool isCurrentThreadSignalManagerThread() const;

        /// \see Runnable:run
        virtual void run();
        /// \see Runnable:stop
        virtual void stop();

    private:
        /// Private constructor only for singleton
        signal_manager();
        /// Private destructor only for singleton
        ~signal_manager();

        /// Typedef for the signal queue
        typedef tbb::concurrent_queue<_signal_handle_base*> SignalQueue;

        SignalQueue _signalQueue;   ///< Queue for signals to be dispatched

        std::condition_variable _evaluationCondition; ///< conditional wait to be used when there are currently no jobs to process
        tbb::mutex _ecMutex; ///< Mutex for protecting _evaluationCondition

        std::thread::id _this_thread_id;

        static const std::string loggerCat_;
    };

// ================================================================================================

    
    class has_slots;
    
    
    class _connection_base0
    {
    public:
        virtual ~_connection_base0() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal() = 0;
        virtual _connection_base0* clone() = 0;
        virtual _connection_base0* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type>
    class _connection_base1
    {
    public:
        virtual ~_connection_base1() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal(arg1_type) = 0;
        virtual _connection_base1<arg1_type>* clone() = 0;
        virtual _connection_base1<arg1_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type>
    class _connection_base2
    {
    public:
        virtual ~_connection_base2() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal(arg1_type, arg2_type) = 0;
        virtual _connection_base2<arg1_type, arg2_type>* clone() = 0;
        virtual _connection_base2<arg1_type, arg2_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type>
    class _connection_base3
    {
    public:
        virtual ~_connection_base3() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal(arg1_type, arg2_type, arg3_type) = 0;
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* clone() = 0;
        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _connection_base4
    {
    public:
        virtual ~_connection_base4() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal(arg1_type, arg2_type, arg3_type, arg4_type) = 0;
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* clone() = 0;
        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _connection_base5
    {
    public:
        virtual ~_connection_base5() {}
        virtual has_slots* getdest() const = 0;
        virtual void emitSignal(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) = 0;
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* clone() = 0;
        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* duplicate(has_slots* pnewdest) = 0;
    };
    
// ================================================================================================
    
    
    class SIGSLOT_API _signal_base
    {
    public:
        virtual void slot_disconnect(has_slots* pslot) = 0;
        virtual void slot_duplicate(has_slots const* poldslot, has_slots* pnewslot) = 0;
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
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
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
        
        ~_signal_base1()
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
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
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
        
        ~_signal_base2()
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
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                ++it;
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
        
        ~_signal_base3()
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
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                
                ++it;
            }
        }
        
        void slot_duplicate(const has_slots* oldtarget, has_slots* newtarget)
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
        
        ~_signal_base4()
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
            connections_list::const_iterator it = s.m_connected_slots.begin();
            connections_list::const_iterator itEnd = s.m_connected_slots.end();
            
            while (it != itEnd) {
                (*it)->getdest()->signal_connect(this);
                m_connected_slots.push_back((*it)->clone());
                
                ++it;
            }
        }
        
        void slot_duplicate(const has_slots* oldtarget, has_slots* newtarget)
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
        
        ~_signal_base5()
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
        
        virtual void emitSignal()
        {
            (m_pobject->*m_pmemfun)();
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)();
    };
    
    template<class dest_type, class arg1_type>
    class _connection1 : public _connection_base1<arg1_type>
    {
    public:
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
        
        virtual void emitSignal(arg1_type a1)
        {
            (m_pobject->*m_pmemfun)(a1);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)(arg1_type);
    };
    
    template<class dest_type, class arg1_type, class arg2_type>
    class _connection2 : public _connection_base2<arg1_type, arg2_type>
    {
    public:
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
        
        virtual void emitSignal(arg1_type a1, arg2_type a2)
        {
            (m_pobject->*m_pmemfun)(a1, a2);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)(arg1_type, arg2_type);
    };
    
    template<class dest_type, class arg1_type, class arg2_type, class arg3_type>
    class _connection3 : public _connection_base3<arg1_type, arg2_type, arg3_type>
    {
    public:
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
        
        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)(arg1_type, arg2_type, arg3_type);
    };
    
    template<class dest_type, class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _connection4 : public _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>
    {
    public:
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
        
        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3, a4);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type);
    };
    
    template<class dest_type, class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _connection5 : public _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>
    {
    public:
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
        
        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            (m_pobject->*m_pmemfun)(a1, a2, a3, a4, a5);
        }
        
        virtual has_slots* getdest() const
        {
            return m_pobject;
        }
        
    private:
        dest_type* m_pobject;
        void (dest_type::* m_pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type);
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

            ~signal_handle0() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal();
                    it = itNext;
                }
            };

            signal0* _sender;
        };


        signal0() {}
        
        signal0(const signal0& s)
        : _signal_base0(s) {}
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)())
        {
            _connection0<desttype>* conn = new _connection0<desttype>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger()
        {
            signal_handle0* sh = new signal_handle0(this);
            signal_manager::getRef().triggerSignal(sh);
        }
        
        void queue()
        {
            signal_handle0* sh = new signal_handle0(this);
            signal_manager::getRef().queueSignal(sh);
        }

        void operator()()
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger();
            else
                queue();
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

            ~signal_handle1() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal(_a1);
                    it = itNext;
                }
            };

            signal1<arg1_type>* _sender;
            arg1_type _a1;
        };

        signal1() {}
        
        signal1(const signal1<arg1_type>& s)
        : _signal_base1<arg1_type>(s) {}
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type))
        {
            _connection1<desttype, arg1_type>* conn = new _connection1<desttype, arg1_type>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger(arg1_type a1)
        {
            signal_handle1* sh = new signal_handle1(this, a1);
            signal_manager::getRef().triggerSignal(sh);
        }

        void queue(arg1_type a1)
        {
            signal_handle1* sh = new signal_handle1(this, a1);
            signal_manager::getRef().queueSignal(sh);
        }

        void operator()(arg1_type a1)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger(a1);
            else
                queue(a1);
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

            ~signal_handle2() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal(_a1, _a2);
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
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type,
                                                                 arg2_type))
        {
            _connection2<desttype, arg1_type, arg2_type>* conn = new _connection2<desttype, arg1_type, arg2_type>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger(arg1_type a1, arg2_type a2)
        {
            signal_handle2* sh = new signal_handle2(this, a1, a2);
            signal_manager::getRef().triggerSignal(sh);
        }

        void queue(arg1_type a1, arg2_type a2)
        {
            signal_handle2* sh = new signal_handle2(this, a1, a2);
            signal_manager::getRef().queueSignal(sh);
        }
        
        void operator()(arg1_type a1, arg2_type a2)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger(a1, a2);
            else
                queue(a1, a2);
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

            ~signal_handle3() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal(_a1, _a2, _a3);
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
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type,
                                                                 arg2_type, arg3_type))
        {
            _connection3<desttype, arg1_type, arg2_type, arg3_type>* conn = new _connection3<desttype, arg1_type, arg2_type, arg3_type>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            signal_handle3* sh = new signal_handle3(this, a1, a2, a3);
            signal_manager::getRef().triggerSignal(sh);
        }

        void queue(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            signal_handle3* sh = new signal_handle3(this, a1, a2, a3);
            signal_manager::getRef().queueSignal(sh);
        }
        
        void operator()(arg1_type a1, arg2_type a2, arg3_type a3)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger(a1, a2, a3);
            else
                queue(a1, a2, a3);
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

            ~signal_handle4() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal(_a1, _a2, _a3, _a4);
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
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type))
        {
            _connection4<desttype, arg1_type, arg2_type, arg3_type, arg4_type>* conn = new _connection4<desttype, arg1_type, arg2_type, arg3_type, arg4_type>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            signal_handle4* sh = new signal_handle4(this, a1, a2, a3, a4);
            signal_manager::getRef().triggerSignal(sh);
        }

        void queue(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            signal_handle4* sh = new signal_handle4(this, a1, a2, a3, a4);
            signal_manager::getRef().queueSignal(sh);
        }
        
        void operator()(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger(a1, a2, a3, a4);
            else
                queue(a1, a2, a3, a4);
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

            ~signal_handle5() {};

            // override
            void emitSignal() const {
                connections_list::const_iterator itNext, it = _sender->m_connected_slots.begin();
                connections_list::const_iterator itEnd = _sender->m_connected_slots.end();

                while (it != itEnd) {
                    itNext = it;
                    ++itNext;
                    (*it)->emitSignal(_a1, _a2, _a3, _a4, _a5);
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
        
        template<class desttype>
        void connect(desttype* pclass, void (desttype::*pmemfun)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type))
        {
            _connection5<desttype, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* conn = new _connection5<desttype, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(pclass, pmemfun);
            m_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }
        
        void trigger(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            signal_handle5* sh = new signal_handle5(this, a1, a2, a3, a4, a5);
            signal_manager::getRef().triggerSignal(sh);
        }

        void queue(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            signal_handle5* sh = new signal_handle5(this, a1, a2, a3, a4, a5);
            signal_manager::getRef().queueSignal(sh);
        }
        
        void operator()(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5)
        {
            if (signal_manager::getRef().isCurrentThreadSignalManagerThread())
                trigger(a1, a2, a3, a4, a5);
            else
                queue(a1, a2, a3, a4, a5);
        }
    };
    
} // namespace sigslot

#endif // SIGSLOT_H
