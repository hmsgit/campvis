// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef DATACONTAINER_H__
#define DATACONTAINER_H__

#include "sigslot/sigslot.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/spin_mutex.h"
#include "core/datastructures/datahandle.h"

#include <string>
#include <map>
#include <utility>
#include <vector>

namespace campvis {
    class AbstractData;
    
    /**
     * A DataContainer manages instances of AbstractData and offers access to them via string identifiers (names/keys).
     * Therefore, it stores them in DataHandles which take ownership of the AbstractData instance. Hence,
     * as soon as an AbstractData instance is added to a DataContainer via DataContainer::addData(), its 
     * lifetime is managed by the wrapping DataHandle instance and its reference counting mechanism.
     * Because the DataHandles are stored as const handles, the underlying data cannot be changed anymore. This
     * also ensures (hopefully) that nobody can do messy things, such as changing the data while some other 
     * thread is reading it. Theoretically this should be possible, but a correct implementation would require
     * some brain fuck.
     * 
     * \todo    Check thread-safety
     */
    class DataContainer {
    public:
        /**
         * Creates a new empty DataContainer
         */
        DataContainer();

        /**
         * Destructor of the DataContainer. Will disconnect all DataHandles from this container.
         */
        ~DataContainer();


        /**
         * Adds the given AbstractData instance \a data, accessible by the key \name, to this DataContainer.
         * In doing so, the DataContainer (respectively the created DataHandle) takes ownership of \a data
         * and will manage its lifetime. So don't even think about deleting \a data yourself!
         *
         * \param   name    Key for accessing the DataHandle within this DataContainer.
         * \param   data    The data to wrap in a DataHandle and add to this DataContainer, must not be 0.
         * \return  A DataHandle containing \a data.
         */
        DataHandle addData(const std::string& name, AbstractData* data);

        /**
         * Adds the given DataHandle \a data, accessible by the key \name, to this DataContainer.
         * Already existing DataHandles with the same key will be removed from this DataContainer.
         *
         * \param   name    Key for accessing the DataHandle within this DataContainer
         * \param   data    DataHandle to add.
         */
        void addDataHandle(const std::string& name, const DataHandle& dh);

        /**
         * Checks whether this DataContainer contains a DataHandle with the given name.
         *
         * \param   name    Key of the DataHandle to search for
         * \return  true, if this DataContainer contains a DataHandle with the given name.
         */
        bool hasData(const std::string& name) const;

        /**
         * Returns the DataHandle with the given name from this container.
         * If no such DataHandle exists, this method returns an empty DataHandle.
         *
         * \param   name    Key of the DataHandle to search for
         * \return  The stored DataHandle with the given name, an empty DataHandle if no such DataHandle exists.
         */
        DataHandle getData(const std::string& name) const;

        /**
         * Removes the DataHandle with the given name from this container.
         * If no such DataHandle exists, nothing happens.
         * \param   name    Key of the DataHandle to remove.
         */
        void removeData(const std::string& name);

        /**
         * Removes all DataHandles from this DataContainer.
         * \note    This method is \b NOT thread-safe!
         */
        void clear();

        /**
         * Returns a copy of the current list of DataHandles.
         * \note    Use with caution, this method is to be considered as slow, as it includes several 
         *          copies and locks the whole DataContainer during execution.
         * \return  A list of pairs (name, DataHandle). The caller has to take ownership of the passed pointers!
         */
        std::vector< std::pair< std::string, DataHandle> > getDataHandlesCopy() const;

        /**
         * Returns a copy of the current map of DataHandles.
         * \note    Use with caution, this method is to be considered as slow, as it includes several 
         *          copies and locks the whole DataContainer during execution.
         * \return  A copy of the current handles map. The caller has to take ownership of the passed pointers!
         */
        //std::map<std::string, DataHandle> getHandlesCopy() const;

        /**
         * Signal emitted when data has been added to this DataContainer (this includes also data being replaced).
         * First parameter is the name of the added data, second parameter contains a DataHandle to the new data.
         */
        sigslot::signal2<const std::string&, const DataHandle&> s_dataAdded;

        /// Signal emitted when list of DataHandles has changed.
        sigslot::signal0<> s_changed;

    private:
        /// Map of the DataHandles in this collection and their IDs. The DataHandles contain valid data.
        //std::map<std::string, DataHandle> _handles;
        mutable tbb::spin_mutex _localMutex;

        tbb::concurrent_unordered_map<std::string, DataHandle> _handles;

        static const std::string loggerCat_;
    };

}

#endif // DATACONTAINER_H__
