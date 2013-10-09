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
    class AutoEvaluationPipeline : public AbstractPipeline {
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
        void onDataNamePropertyChanged(const AbstractProperty* prop);

        virtual void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /// Hashmap storing for each processor whether it's a VisualizationProcessor or not.
        tbb::concurrent_hash_map<AbstractProcessor*, bool> _isVisProcessorMap;


        typedef tbb::concurrent_unordered_multimap<std::string, DataNameProperty*> PortMapType;
        typedef tbb::concurrent_unordered_map<DataNameProperty*, PortMapType::iterator> IteratorMapType;

        /// Multimap to simulate ports between processors
        PortMapType _portMap;
        IteratorMapType _iteratorMap;
        tbb::spin_rw_mutex _pmMutex;
    };

}

#endif // AUTOEVALUATIONPIPELINE_H__
