#ifndef METATABLELUATABLE_H__
#define METATABLELUATABLE_H__

#include "luatable.h"
#include "scripting/scriptingapi.h"

namespace campvis {

    /**
     * Class representing Lua metatables.
     *
     * Any Lua field may contain a metatable. They serve multiple purposes, including emulation 
     * of classes and objects.
     */
    class CAMPVIS_SCRIPTING_API MetatableLuaTable : public LuaTable {
    public:
        /**
         * Creates a new MetatableLuaTable.
         *
         * \param   parent  Lua table (regular or global) that stores this table
         * \param   name    Name of the field carrying this metatable
         */
        MetatableLuaTable(std::shared_ptr<LuaTable> parent, std::string fieldName);

        /**
         * Virtual destructor.
         */
        virtual ~MetatableLuaTable();

        virtual bool isValid() override;
        virtual void callInstanceMethod(const std::string& name) override;
        
        virtual void pushField(const std::string& name) override;
        virtual void popRecursive() override;

    protected:
        virtual void populateValueMap() override;
        virtual LuaTable* getParentTable() override;

    private:
        std::shared_ptr<LuaTable> _parent;  ///< Lua table in which this metatable is stored
        std::string _fieldName;             ///< Field name in _parent this metatable belongs to
    };
}

#endif // METATABLELUATABLE_H__
