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
#include "tbb/recursive_mutex.h"
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
    typedef std::pair<void*, swig_type_info*> ArgWithTypeInfoType;

    /*
     * Return an object bundling the provided argument with its SWIG type information.
     */
    template<typename T>
    inline ArgWithTypeInfoType getArgWithTypeInfo(T arg) {
        const char* const typeName = LuaConnectionArgTraits<T>::typeName;
        swig_type_info* typeInfo = SWIG_TypeQuery(typeName);

        if (typeInfo == nullptr) {
            std::cerr << "SWIG wrapper for " << typeName << " not found" << std::endl;
        }

        return std::make_pair(arg, typeInfo);
    }

    /*
     * Prepend a new element to a list of objects that bundle arguments with their SWIG type
     * information and return the resulting list.
     */
    inline std::list<ArgWithTypeInfoType>* argWithTypeInfoListCons(ArgWithTypeInfoType head,
                                                                   std::list<ArgWithTypeInfoType>* tail)
    {
        if (head.second == nullptr) {
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

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5,
                                                                  T6 arg6)
    {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1),
                                       getArgWithTypeInfoList<T2, T3, T4, T5, T6>(arg2, arg3, arg4, arg5, arg6));
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
                                                                  T7 arg7)
    {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1),
                                       getArgWithTypeInfoList<T2, T3, T4, T5, T6, T7>(arg2, arg3, arg4, arg5, arg6,
                                                                                      arg7));
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    inline std::list<ArgWithTypeInfoType>* getArgWithTypeInfoList(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
                                                                  T7 arg7, T8 arg8)
    {
        return argWithTypeInfoListCons(getArgWithTypeInfo<T1>(arg1),
                                       getArgWithTypeInfoList<T2, T3, T4, T5, T6, T7, T8>(arg2, arg3, arg4, arg5, arg6,
                                                                                          arg7, arg8));
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
     */
    template<class mt_policy>
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
        has_slots<mt_policy>* getDummyDest() const {
            if (_dummy_dest == nullptr)
                _dummy_dest = new has_slots<mt_policy>();

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
                    SWIG_NewPointerObj(_slot_fn.L, it->first, it->second, 0);

                if (lua_pcall(_slot_fn.L, argWithTypeInfoList->size(), 0, 0) != LUA_OK) {
                    const char* errorMsg = lua_tostring(_slot_fn.L, -1);

                    if (errorMsg == nullptr)
                        std::cerr << "(error object is not a string)" << std::endl;
                    else
                        std::cerr << "An error occured while calling a Lua slot function: " << errorMsg << std::endl;

                    lua_pop(_slot_fn.L, 1);
                }

                delete argWithTypeInfoList;
            }
        }

        SWIGLUA_REF _slot_fn;                          ///< Reference to a Lua function acting as a slot
        mutable has_slots<mt_policy>* _dummy_dest;     ///< Dummy destination object needed to support getdest()
        LuaStateMutexType* _lua_state_mutex;           ///< Mutex guarding access to the above function's Lua state
    };

    /**
     * Custom signal-slot connection type for nullary signals that accepts Lua functions as slots.
     */
    template<class mt_policy>
    class _lua_connection0 : public _lua_connection_base<mt_policy>, public _connection_base0<mt_policy>
    {
    public:
        _lua_connection0(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection0(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base0<mt_policy>* clone() {
            return new _lua_connection0(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base0<mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal() {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = new std::list<ArgWithTypeInfoType>(0);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for unary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class mt_policy>
    class _lua_connection1 : public _lua_connection_base<mt_policy>, public _connection_base1<arg1_type, mt_policy>
    {
    public:
        _lua_connection1(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection1(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base1<arg1_type, mt_policy>* clone() {
            return new _lua_connection1(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base1<arg1_type, mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = getArgWithTypeInfoList<arg1_type>(a1);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for binary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class mt_policy>
    class _lua_connection2 : public _lua_connection_base<mt_policy>
                           , public _connection_base2<arg1_type, arg2_type, mt_policy>
    {
    public:
        _lua_connection2(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection2(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base2<arg1_type, arg2_type, mt_policy>* clone() {
            return new _lua_connection2(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base2<arg1_type, arg2_type, mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest) {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList = getArgWithTypeInfoList<arg1_type, arg2_type>(a1, a2);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for ternary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class mt_policy>
    class _lua_connection3 : public _lua_connection_base<mt_policy>
                           , public _connection_base3<arg1_type, arg2_type, arg3_type, mt_policy>
    {
    public:
        _lua_connection3(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection3(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base3<arg1_type, arg2_type, arg3_type, mt_policy>* clone() {
            return new _lua_connection3(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base3<arg1_type, arg2_type, arg3_type,
                                  mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type>(a1, a2, a3);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 4-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class mt_policy>
    class _lua_connection4 : public _lua_connection_base<mt_policy>
                           , public _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>
    {
    public:
        _lua_connection4(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection4(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>* clone() {
            return new _lua_connection4(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base4<arg1_type, arg2_type, arg3_type, arg4_type,
                                  mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type>(a1, a2, a3, a4);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 5-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class mt_policy>
    class _lua_connection5 : public _lua_connection_base<mt_policy>
                           , public _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, mt_policy>
    {
    public:
        _lua_connection5(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection5(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, mt_policy>* clone() {
            return new _lua_connection5(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type,
                                  mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>(a1, a2, a3, a4, a5);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 6-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class mt_policy>
    class _lua_connection6 : public _lua_connection_base<mt_policy>
                           , public _connection_base6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                      mt_policy>
    {
    public:
        _lua_connection6(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection6(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                  mt_policy>* clone()
        {
            return new _lua_connection6(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                  mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6) {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type,
                                           arg6_type>(a1, a2, a3, a4, a5, a6);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 7-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class arg7_type, class mt_policy>
    class _lua_connection7 : public _lua_connection_base<mt_policy>
                           , public _connection_base7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                      arg7_type, mt_policy>
    {
    public:
        _lua_connection7(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection7(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                  mt_policy>* clone()
        {
            return new _lua_connection7(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                  mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6,
                                arg7_type a7)
        {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                           arg7_type>(a1, a2, a3, a4, a5, a6, a7);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };

    /**
     * Custom signal-slot connection type for 8-ary signals that accepts Lua functions as slots.
     */
    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class arg7_type, class arg8_type, class mt_policy>
    class _lua_connection8 : public _lua_connection_base<mt_policy>
                           , public _connection_base8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                      arg7_type, arg8_type, mt_policy>
    {
    public:
        _lua_connection8(SWIGLUA_REF slot_fn)
            : _lua_connection_base<mt_policy>(slot_fn)
        {}

        _lua_connection8(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _lua_connection_base<mt_policy>(slot_fn, lua_state_mutex)
        {}

        virtual _connection_base8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                  arg8_type, mt_policy>* clone()
        {
            return new _lua_connection8(this->copySlotFunction(), this->_lua_state_mutex);
        }

        virtual _connection_base8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                  arg8_type, mt_policy>* duplicate(sigslot::has_slots<mt_policy>* pnewdest)
        {
            /*
             * Because Lua connections do not have any external destination objects that could be
             * copied (which in turn would require duplicating the connections for the copy), this
             * method should never be invoked.
             */
            return nullptr;
        }

        virtual void emitSignal(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6,
                                arg7_type a7, arg8_type a8)
        {
            std::list<ArgWithTypeInfoType>* argWithTypeInfoList =
                    getArgWithTypeInfoList<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                           arg7_type, arg8_type>(a1, a2, a3, a4, a5, a6, a7, a8);
            this->callLuaSlot(argWithTypeInfoList);
        }

        virtual has_slots<mt_policy>* getdest() const {
            return this->getDummyDest();
        }
    };
}
}

namespace sigslot {

    template<class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal0 {
    public:
        signal0();
        signal0(const sigslot::signal0<mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection0<mt_policy>* conn = new sigslot::_lua_connection0<mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base0<mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection0<mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection0<mt_policy>*>(*it);

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

    template<class arg1_type, class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal1 {
    public:
        signal1();
        signal1(const sigslot::signal1<arg1_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection1<arg1_type, mt_policy>* conn =
                    new sigslot::_lua_connection1<arg1_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base1<arg1_type, mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection1<arg1_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection1<arg1_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal2 {
    public:
        signal2();
        signal2(const sigslot::signal2<arg1_type, arg2_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection2<arg1_type, arg2_type, mt_policy>* conn =
                    new sigslot::_lua_connection2<arg1_type, arg2_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base2<arg1_type, arg2_type, mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection2<arg1_type, arg2_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection2<arg1_type, arg2_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal3 {
    public:
        signal3();
        signal3(const sigslot::signal3<arg1_type, arg2_type, arg3_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type, mt_policy>* conn =
                    new sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base3<arg1_type, arg2_type, arg3_type,
                                               mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection3<arg1_type, arg2_type, arg3_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type,
             class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal4 {
    public:
        signal4();
        signal4(const sigslot::signal4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>* conn =
                    new sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base4<arg1_type, arg2_type, arg3_type, arg4_type,
                                               mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection4<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type,
             class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal5 {
    public:
        signal5();
        signal5(const sigslot::signal5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, mt_policy>* conn =
                    new sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type,
                                                  mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type,
                                               mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type,
                                              mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection5<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   arg5_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal6 {
    public:
        signal6();
        signal6(const sigslot::signal6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                          mt_policy>* conn =
                    new sigslot::_lua_connection6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                  mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                               mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection6<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                              mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection6<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   arg5_type, arg6_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class arg7_type, class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal7 {
    public:
        signal7();
        signal7(const sigslot::signal7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                       mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                          mt_policy>* conn =
                    new sigslot::_lua_connection7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                  arg7_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                               arg7_type, mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection7<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                              arg7_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection7<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   arg5_type, arg6_type, arg7_type, mt_policy>*>(*it);

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

    template<class arg1_type, class arg2_type, class arg3_type, class arg4_type, class arg5_type, class arg6_type,
             class arg7_type, class arg8_type, class mt_policy = sigslot::SIGSLOT_DEFAULT_MT_POLICY>
    class signal8 {
    public:
        signal8();
        signal8(const sigslot::signal8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                       arg8_type, mt_policy>& s);

        %extend {
            /**
             * Connect this signal to a Lua function.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void connect(SWIGLUA_REF slot_fn) {
                sigslot::lock_block_write<mt_policy> lock($self);
                sigslot::_lua_connection8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type,
                                          arg8_type, mt_policy>* conn =
                    new sigslot::_lua_connection8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                                  arg7_type, arg8_type, mt_policy>(slot_fn);
                $self->m_connected_slots.push_back(conn);
            }

            /**
             * Disconnect a Lua function from this signal.
             *
             * @param slot_fn reference to a Lua function acting as a slot
             */
            void disconnect(SWIGLUA_REF slot_fn) {
                typedef sigslot::_signal_base8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                               arg7_type, arg8_type, mt_policy>::connections_list connections_list;

                sigslot::lock_block_write<mt_policy> lock($self);
                connections_list::iterator it = $self->m_connected_slots.begin();
                connections_list::iterator itEnd = $self->m_connected_slots.end();

                while (it != itEnd) {
                    sigslot::_lua_connection8<arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type,
                                              arg7_type, arg8_type, mt_policy>* lua_connection =
                            dynamic_cast<sigslot::_lua_connection8<arg1_type, arg2_type, arg3_type, arg4_type,
                                                                   arg5_type, arg6_type, arg7_type, arg8_type,
                                                                   mt_policy>*>(*it);

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
