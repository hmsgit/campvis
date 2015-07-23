/*
 * Module providing SWIG wrappers for sigslot that make it possible to use Lua functions as slots.
 *
 * SWIG's lua_fnptr extension and one of the structures it defines, SWIGLUA_REF, are used to handle
 * Lua functions on the C++ side.
 */

%module sigslot

%include lua_fnptr.i

%{
#include <cstdio>
#include <iostream>
#include <type_traits>
#include "tbb/recursive_mutex.h"
#include "ext/cgt/logmanager.h"
#include "ext/sigslot/sigslot.h"
%}


%inline {
namespace sigslot {

    /**
     * Signal arguments need to be wrapped before they can be passed to slots defined in Lua. That
     * requires the textual representation of their respective types to be known. Unfortunately,
     * there is no portable way to get a string describing a type in C++, and SWIG doesn't expose
     * such functionality. Consequently, we use a trait for that purpose; it needs to be specialised
     * for all types used as arguments of signals that are exposed to Lua, e.g.:
     *
     * template<>
     * struct LuaConnectionArgTraits<campvis::AbstractProcessor*> {
     *     static const char* const typeName;
     * };
     *
     * const char* const LuaConnectionArgTraits<campvis::AbstractProcessor*>::typeName = "campvis::AbstractProcessor *";
     */
    template<typename T>
    struct LuaConnectionArgTraits {};

    /*
     * Type bundling all information necessary to inject a signal argument into a Lua state:
     * a pointer to the argument and its corresponding SWIG type information.
     */
    struct ArgWithTypeInfoType {
        void* ptr;
        swig_type_info* type_info;
        void (*deleter)(void*);
    };
    
    template<typename T, bool makeCopy>
    struct CreateAwtitHelper {};
    
    template<typename T>
    struct CreateAwtitHelper<T, true> {
        static void deletePtr(void* ptr) {
            delete static_cast<T*>(ptr);
        }

        static ArgWithTypeInfoType createAwtit(T arg, swig_type_info* typeInfo) {
            ArgWithTypeInfoType toReturn = { new T(arg), typeInfo, &deletePtr };
            return toReturn;
        }
    };

    template<typename T>
    struct CreateAwtitHelper<T, false> {
        static ArgWithTypeInfoType createAwtit(T arg, swig_type_info* typeInfo) {
            ArgWithTypeInfoType toReturn = { arg, typeInfo, nullptr };
            return toReturn;
        }
    };


    /*
     * Return an object bundling the provided argument with its SWIG type information.
     */
    template<typename T>
    inline ArgWithTypeInfoType getArgWithTypeInfo(T arg) {
        const char* const typeName = LuaConnectionArgTraits<T>::typeName;
        swig_type_info* typeInfo = SWIG_TypeQuery(typeName);

        if (typeInfo == nullptr) {
            LogMgr.log("Lua", cgt::LuaError, "SWIG wrapper for " + std::string(typeName) + " not found");
        }

        ArgWithTypeInfoType toReturn = CreateAwtitHelper<T, !std::is_pointer<T>::value && !std::is_reference<T>::value>::createAwtit(arg, typeInfo);
        return toReturn;
    }

    /*
     * Prepend a new element to a list of objects that bundle arguments with their SWIG type
     * information and return the resulting list.
     */
    inline std::list<ArgWithTypeInfoType>* argWithTypeInfoListCons(ArgWithTypeInfoType head,
                                                                   std::list<ArgWithTypeInfoType>* tail)
    {
        if (head.type_info == nullptr) {
            delete tail;
        } else if (tail != nullptr) {
            tail->push_front(head);
            return tail;
        }

        return nullptr;
    }

    /*
     * Series of functions that take several arguments and return a list of objects that bundle
     * those arguments with their SWIG type information.
     */

    template<typename T>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T arg) {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T>(arg), new std::list<ArgWithTypeInfoType>());
    }

    template<typename T1, typename T2>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2) {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1), getArgWithTypeInfoList<T2>(arg2));
    }

    template<typename T1, typename T2, typename T3>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3) {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1), getArgWithTypeInfoList<T2, T3>(arg2, arg3));
    }

    template<typename T1, typename T2, typename T3, typename T4>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3, T4 arg4) {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1),
                                       getArgWithTypeInfoList<T2, T3, T4>(arg2, arg3, arg4));
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1),
                                       getArgWithTypeInfoList<T2, T3, T4, T5>(arg2, arg3, arg4, arg5));
    }

    // Type of mutex used to guard access to Lua state. See LuaVmState for more details.
    typedef tbb::recursive_mutex LuaStateMutexType;

    /*
     * Base class for Lua connections for specific signal arities.
     *
     * This class implements the bridging between sigslot's signals and slots defined in Lua, as
     * well as some other logic (e.g. copySlotFunction and getDummyDest) common to all Lua
     * connections. Subclasses need simply to call the helpers provided by this class from their
     * interface implementation methods.
     *
     * The template parameter is just a dummy parameter to enforce that _connection_base0 is always
     * locally instantiated as template and not linked into the sigslot library. This makes writing
     * scripting bindings much easier
     */
    template<int DUMMY_PARAMETER = 0>
    class _lua_connection_base {
    public:
        _lua_connection_base(SWIGLUA_REF slot_fn)
            : _slot_fn(slot_fn)
            , _dummy_dest(nullptr)
        {
            // Retrieve from the registry the mutex associated with the function's Lua state
            lua_pushlightuserdata(slot_fn.L, static_cast<void*>(slot_fn.L));
            lua_gettable(slot_fn.L, LUA_REGISTRYINDEX);

            // The mutex should be stored as light userdata
            assert(lua_islightuserdata(slot_fn.L, -1));
            _lua_state_mutex = static_cast<LuaStateMutexType*>(lua_touserdata(slot_fn.L, -1));
            lua_pop(slot_fn.L, 1);
        }

        _lua_connection_base(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _slot_fn(slot_fn)
            , _dummy_dest(nullptr)
            , _lua_state_mutex(lua_state_mutex)
        {}

        virtual ~_lua_connection_base() {
            swiglua_ref_clear(&_slot_fn);

            if (_dummy_dest != nullptr)
                delete _dummy_dest;
        }

        /**
         * Check if this connection's slot function is the same as the given Lua function.
         *
         * @param slot_fn reference to a Lua function acting as a slot
         * @return true if this connection wraps the given Lua function, false otherwise
         */
        bool wrapsSlotFunction(SWIGLUA_REF slot_fn) {
            if (slot_fn.L != _slot_fn.L)
                return false;

            swiglua_ref_get(&_slot_fn);
            swiglua_ref_get(&slot_fn);

            bool result = lua_rawequal(slot_fn.L, -1, -2) == 1;
            lua_pop(_slot_fn.L, 2);

            return result;
        }

    protected:
        /**
         * Return a copy of this connection's slot function.
         *
         * @return copy of this connection's slot function
         */
        SWIGLUA_REF copySlotFunction() {
            SWIGLUA_REF slot_fn;

            {
                LuaStateMutexType::scoped_lock lock(*_lua_state_mutex);

                swiglua_ref_get(&_slot_fn);
                swiglua_ref_set(&slot_fn, _slot_fn.L, -1);
                lua_pop(_slot_fn.L, 1);
            }

            return slot_fn;
        }

        /**
         * Return a dummy destination object.
         *
         * Because Lua connections do not have any destination objects (i.e. slots defined in Lua
         * are not attached to any class), a dummy one has to be created to comply with sigslot's
         * API.
         *
         * @return dummy destination object
         */
        has_slots* getDummyDest() const {
            if (_dummy_dest == nullptr)
                _dummy_dest = new has_slots();

            return _dummy_dest;
        }

        /**
         * Call a slot defined in Lua with the provided arguments.
         *
         * @param argWithTypeInfoList list of objects that bundle signal arguments with their SWIG
         *                            type information
         */
        void callLuaSlot(std::list<ArgWithTypeInfoType>* argWithTypeInfoList) {
            /*
             * argWithTypeInfoList is null if type information for some arguments could not be
             * found. In that case an error has been logged already and there's no processing left
             * to be done.
             */
            if (argWithTypeInfoList != nullptr) {
                LuaStateMutexType::scoped_lock lock(*_lua_state_mutex);

                // Put this connection's slot and all arguments on Lua's stack
                swiglua_ref_get(&_slot_fn);

                for (auto it = argWithTypeInfoList->begin(); it != argWithTypeInfoList->end(); ++it)
                    SWIG_NewPointerObj(_slot_fn.L, it->ptr, it->type_info, 0);

                if (lua_pcall(_slot_fn.L, argWithTypeInfoList->size(), 0, 0) != LUA_OK) {
                    const char* errorMsg = lua_tostring(_slot_fn.L, -1);

                    if (errorMsg == nullptr)
                        LogMgr.log("Lua", cgt::LuaError, "(error object is not a string)");
                    else
                        LogMgr.log("Lua", cgt::LuaError, "An error occured while calling a Lua slot function: " + std::string(errorMsg));

                    lua_pop(_slot_fn.L, 1);
                }

                for (auto it = argWithTypeInfoList->begin(); it != argWithTypeInfoList->end(); ++it) {
                    if (it->deleter != nullptr)
                        (it->deleter)(it->ptr);
                }
                delete argWithTypeInfoList;
            }
        }

        SWIGLUA_REF _slot_fn;                          ///< Reference to a Lua function acting as a slot
        mutable has_slots* _dummy_dest;                ///< Dummy destination object needed to support getdest()
        LuaStateMutexType* _lua_state_mutex;           ///< Mutex guarding access to the above function's Lua state
    };

    /**
     * Custom signal-slot connection type for nullary signals that accepts Lua functions as slots.
     *
     * The template parameter is just a dummy parameter to enforce that _connection_base0 is always
     * locally instantiated as template and not linked into the sigslot library. This makes writing
     * scripting bindings much easier
     */
    template<int DUMMY_PARAMETER = 0>
    class _lua_connection0 : public _lua_connection_base<>, public _connection_base0
    {
    public:
        _lua_connection0(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection0(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base0* clone() {
            return new _lua_connection0<DUMMY_PARAMETER>(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base0* duplicate(sigslot::has_slots* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal() {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = new std::list<ArgWithTypeInfoType>(0);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for unary signals that accepts Lua functions as slots.
     */
    template<class arg1_type>
    class _lua_connection1 : public _lua_connection_base<>, public _connection_base1<arg1_type>
    {
    public:
        _lua_connection1(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection1(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base1<arg1_type>* clone() {
            return new _lua_connection1(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base1<arg1_type>* duplicate(sigslot::has_slots* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal(arg1_type a1) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = getArgWithTypeInfoList<arg1_type>(a1);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for binary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type>
    class _lua_connection2 : public _lua_connection_base<>
                           , public _connection_base2<arg1_type, arg2_type>
    {
    public:
        _lua_connection2(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection2(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base2<arg1_type, arg2_type>* clone() {
            return new _lua_connection2(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base2<arg1_type, arg2_type>* duplicate(sigslot::has_slots* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal(arg1_type a1, arg2_type a2) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = getArgWithTypeInfoList<arg1_type, arg2_type>(a1, a2);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for ternary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type>
    class _lua_connection3 : public _lua_connection_base<>
                           , public _connection_base3<arg1_type, arg2_type, arg3_type>
    {
    public:
        _lua_connection3(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection3(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* clone() {
            return new _lua_connection3(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base3<arg1_type, arg2_type, arg3_type>* duplicate(sigslot::has_slots* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type>(a1, a2, a3);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 4-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class _lua_connection4 : public _lua_connection_base<>
                           , public _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>
    {
    public:
        _lua_connection4(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection4(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* clone() {
            return new _lua_connection4(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type>* duplicate(sigslot::has_slots* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type>(a1, a2, a3, a4);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 5-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class _lua_connection5 : public _lua_connection_base<>
                           , public _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>
    {
    public:
        _lua_connection5(SWIGLUA_REF slot_fn)
            : _lua_connection_base<>(slot_fn)
        {}

        _lua_connection5(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* clone() {
            return new _lua_connection5(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* duplicate(sigslot::has_slots* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void processSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(a1, a2, a3, a4, a5);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots* getdest() const {
            return this->getDummyDest();
        }
    };

}
}

namespace sigslot {

    class signal0 {
    public:
        signal0();
        signal0(const sigslot::signal0& s);
        virtual ~signal0();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection0<>* conn = new sigslot::_lua_connection0<>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base0::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection0<>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection0<>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }
        }
    };

    template<class arg1_type>
    class signal1 {
    public:
        signal1();
        signal1(const sigslot::signal1<arg1_type>& s);
        virtual ~signal1();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection1<arg1_type>* conn =
                    new sigslot::_lua_connection1<arg1_type>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base1<arg1_type>::connections_list connections_list;

                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection1<arg1_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection1<arg1_type>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }
        }
    };

    template<class arg1_type, class arg2_type>
    class signal2 {
    public:
        signal2();
        signal2(const sigslot::signal2<arg1_type, arg2_type>& s);
        virtual ~signal2();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection2<arg1_type, arg2_type>* conn =
                    new sigslot::_lua_connection2<arg1_type, arg2_type>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base2<arg1_type, arg2_type>::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection2<arg1_type, arg2_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection2<arg1_type, arg2_type>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }

            void disconnectAllLuaSlots() {
                typedef sigslot::_signal_base2<arg1_type, arg2_type>::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection2<arg1_type, arg2_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection2<arg1_type, arg2_type>*>(*it);

                    if (lua_connection != nullptr) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                    }

                    ++it;
                }
            }
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type>
    class signal3 {
    public:
        signal3();
        signal3(const sigslot::signal3<arg1_type, arg2_type, arg3_type>& s);
        virtual ~signal3();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type>* conn =
                    new sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base3<arg1_type, arg2_type, arg3_type>::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type>
    class signal4 {
    public:
        signal4();
        signal4(const sigslot::signal4<arg1_type, arg2_type, arg3_type, arg4_type>& s);
        virtual ~signal4();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type>* conn =
                    new sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base4<arg1_type, arg2_type, arg3_type, arg4_type>::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }
        }
    };

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type>
    class signal5 {
    public:
        signal5();
        signal5(const sigslot::signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>& s);
        virtual ~signal5();

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* conn =
                    new sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>::connections_list connections_list;
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   arg5_type>*>(*it);

                    if (lua_connection != nullptr && lua_connection->wrapsSlotFunction(slot_fn)) {
                        delete lua_connection;
                        $self->m_connected_slots.erase(it);
                        return;
                    }

                    ++it;
                }
            }
        }
    };
}
