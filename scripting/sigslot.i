%module sigslot

%include lua_fnptr.i

%{
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

    typedef tbb::recursive_mutex LuaStateMutexType;

    /**
     * Custom signal-slot connection type that accepts Lua functions as slots.
     *
     * SWIG's lua_fnptr extension and one of the structures it defines, SWIGLUA_REF, are used to
     * handle Lua functions on the C++ side.
     */
    template<class arg1_type, class mt_policy>
    class _lua_connection1 : public _connection_base1<arg1_type, mt_policy>
    {
    public:
        _lua_connection1(SWIGLUA_REF slot_fn)
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

        _lua_connection1(SWIGLUA_REF slot_fn, LuaStateMutexType* lua_state_mutex)
            : _slot_fn(slot_fn)
            , _dummy_dest(nullptr)
            , _lua_state_mutex(lua_state_mutex)
        {}

        virtual ~_lua_connection1() {
            swiglua_ref_clear(&_slot_fn);

            if (_dummy_dest != nullptr)
                delete _dummy_dest;
        }

        virtual _connection_base1<arg1_type, mt_policy>* clone() {
            SWIGLUA_REF slot_fn;

            {
                LuaStateMutexType::scoped_lock lock(*_lua_state_mutex);

                swiglua_ref_get(&_slot_fn);
                swiglua_ref_set(&slot_fn, _slot_fn.L, -1);
                lua_pop(_slot_fn.L, 1);
            }

            return new _lua_connection1(slot_fn, _lua_state_mutex);
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
            const char* const typeName = LuaConnectionArgTraits<arg1_type>::typeName;
            swig_type_info* argTypeInfo = SWIG_TypeQuery(typeName);

            if (argTypeInfo == nullptr) {
                std::cerr << "SWIG wrapper for " << typeName << " not found" << std::endl;
                return;
            }

            {
                LuaStateMutexType::scoped_lock lock(*_lua_state_mutex);

                // Put this connection's slot and all arguments on Lua's stack
                swiglua_ref_get(&_slot_fn);
                SWIG_NewPointerObj(_slot_fn.L, a1, argTypeInfo, 0);

                if (lua_pcall(_slot_fn.L, 1, 0, 0) != LUA_OK) {
                    const char* errorMsg = lua_tostring(_slot_fn.L, -1);

                    if (errorMsg == nullptr)
                        std::cerr << "(error object is not a string)" << std::endl;
                    else
                        std::cerr << "An error occured while calling a Lua slot function: " << errorMsg << std::endl;

                    lua_pop(_slot_fn.L, 1);
                }
            }
        }

        virtual has_slots<mt_policy>* getdest() const {
            /*
             * Because Lua connections do not have any destination objects, a dummy one has to be
             * created here and returned to comply with sigslot's API.
             */
            if (_dummy_dest == nullptr)
                _dummy_dest = new has_slots<mt_policy>();

            return _dummy_dest;
        }

    private:
        SWIGLUA_REF _slot_fn;                          ///< Reference to a Lua function acting as a slot
        mutable has_slots<mt_policy>* _dummy_dest;     ///< Dummy destination object needed to support getdest()
        LuaStateMutexType* _lua_state_mutex;           ///< Mutex guarding access to the above function's Lua state
    };
}
}

namespace sigslot {

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
        }
    };
}
