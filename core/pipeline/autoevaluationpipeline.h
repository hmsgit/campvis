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

#ifndef AUTOEVALUATIONPIPELINE_H__
#define AUTOEVALUATIONPIPELINE_H__

#include "sigslot/sigslot.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/spin_rw_mutex.h>

#include "core/pipeline/abstractpipeline.h"


namespace campvis {
    /**
     * Specializtaion of AbstractPipeline performing automatic execution of invalidated processors.
     * AutoEvaluationPipeline connects to the s_(in)validated signals of all of its processors and
     * executes processors with invalid results using the correct threads.
     * 
     * \param   dc  Pointer to the DataContainer containing local working set of data for this 
     *              pipeline, must not be 0, must be valid the whole lifetime of this pipeline.
     */
    class CAMPVIS_CORE_API AutoEvaluationPipeline : public AbstractPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        AutoEvaluationPipeline(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~AutoEvaluationPipeline();


        /// \see AbstractPipeline::init()
        virtual void init();
        /// \see AbstractPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::addProcessor()
        virtual void addProcessor(AbstractProcessor* processor);

    protected:
        /**
         * Slot getting called when one of the observed processors got invalidated.
         * The default behaviour is to dispatch a job to execute the invalidated processor and emit the s_invalidated signal.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);

        static const std::string loggerCat_;

    private:
        /**
         * Callback slot called if one of the DataNameProperties in the port map has changed.
         * \param   prop    DataNameProperty that has changed.
         */
        void onDataNamePropertyChanged(const AbstractProperty* prop);

        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        virtual void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /**
         * Recursively looks for all DataNameProperties in \a pc and adds them to the port map.
         * If \a pc contains a MetaProperty, it will be seached recursively.
         * \param   pc  PropertyCollection to search for DataNameProperties.
         */
        void findDataNamePropertiesAndAddToPortMap(const HasPropertyCollection* pc);

        /// Hashmap storing for each processor whether it's a VisualizationProcessor or not.
        tbb::concurrent_hash_map<AbstractProcessor*, bool> _isVisProcessorMap;

        /// PortMap typedef mapping a string to a set of DataNameProperties using a concurrent unordered multimap
        typedef tbb::concurrent_unordered_multimap<std::string, DataNameProperty*> PortMapType;
        /// IteratorMap typedef mapping a DataNameProperty to an iterator in a PortMap using a concurrent unordered map
        typedef tbb::concurrent_unordered_map<DataNameProperty*, PortMapType::iterator> IteratorMapType;

        /// Multimap to simulate ports between processors
        PortMapType _portMap;
        IteratorMapType _iteratorMap;
        tbb::spin_rw_mutex _pmMutex;
    };

}

#endif // AUTOEVALUATIONPIPELINE_H__
