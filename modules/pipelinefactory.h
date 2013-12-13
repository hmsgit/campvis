// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef PIPELINEFACTORY_H__
#define PIPELINEFACTORY_H__

#include "tgt/logmanager.h"
#include "tgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include <map>
#include <string>
#include <vector>

namespace campvis {
    class AbstractPipeline;
    class DataContainer;

    /**
     * Factory for creating pipelines by their name.
     * Using some template-magic, PipelineFactory is able to register pipelines during static 
     * initialization in cooperation with the PipelineRegistrar.
     * 
     * \note    PipelineFactory is a thread-safe lazy-instantiated singleton.
     */
    class PipelineFactory {
    public:
        /**
         * Returns a reference to the PipelineFactory singleton.
         * Creates the singleton if necessary
         * \return  *_singleton
         */
        static PipelineFactory& getRef();
	
	static void deinit();

        std::vector<std::string> getRegisteredPipelines() const;

        AbstractPipeline* createPipeline(const std::string& id, DataContainer* dc) const;

        /**
         * Statically registers the pipeline of type T using \a callee as factory method.
         * \note    The template instantiation of PipelineRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerPipeline(AbstractPipeline* (* callee)(DataContainer*)) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            std::map<std::string, AbstractPipeline* (*)(DataContainer*)>::iterator it = _pipelineMap.lower_bound(T::getId());
            if (it == _pipelineMap.end() || it->first != T::getId()) {
                _pipelineMap.insert(it, std::make_pair(T::getId(), callee));
            }
            else {
                tgtAssert(false, "Registered two pipelines with the same ID.");
            }
            
            return _pipelineMap.size();
        }

    private:
        mutable tbb::spin_mutex _mutex;
        static tbb::atomic<PipelineFactory*> _singleton;    ///< the singleton object

        std::map<std::string, AbstractPipeline* (*)(DataContainer*)> _pipelineMap;
    };


// ================================================================================================

    template<typename T>
    class PipelineRegistrar {
    public:
        /**
         * Static factory method for creating the pipeline of type T.
         * \param   dc  DataContainer for the created pipeline to work on.
         * \return  A newly created pipeline of type T. Caller has to take ownership of the pointer.
         */
        static AbstractPipeline* create(DataContainer* dc) {
            return new T(dc);
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T>
    const size_t PipelineRegistrar<T>::_factoryId = PipelineFactory::getRef().registerPipeline<T>(&PipelineRegistrar<T>::create);

}

#endif // PIPELINEFACTORY_H__
