// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef DIGRAPHVISUALIZATIONPIPELINE_H__
#define DIGRAPHVISUALIZATIONPIPELINE_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include "tgt/event/eventlistener.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/properties/genericproperty.h"

#include <map>
#include <set>

namespace campvis {
    class VisualizationProcessor;

    /**
     * Specialization of the VisualizationPipeline that performs automatic evaluation based on
     * an acyclic directed dependency graph.
     * 
     * \todo    Implement thread-safety. The current graph implementation is \b not thread-safe.
     */
    class DigraphVisualizationPipeline : public VisualizationPipeline {
    public:
        /**
         * Creates a DigraphVisualizationPipeline.
         */
        DigraphVisualizationPipeline();

        /**
         * Virtual Destructor
         **/
        virtual ~DigraphVisualizationPipeline();


        /**
         * Execute this pipeline.
         * Pipeline must have a valid canvas set before calling this method.
         **/
        virtual void execute() = 0;


        /**
         * Adds a processor to this pipeline, so that it will be managed.
         * 
         * \note    Add each processor only once!
         * \param   processor   Processor to add
         */
        void addProcessor(AbstractProcessor* processor);

        /**
         * Adds a dependency link between two processors.
         * The processor \a childProc needs to be executed every time when \a fatherProc has been evaluated.
         * \note    Add each dependency between two processors only once!
         * \param   fatherProc  The processor \a childProc is dependent on.
         * \param   childProc   The dependent processor of \a fatherProc.
         */
        void addProcessorDependency(AbstractProcessor* fatherProc, AbstractProcessor* childProc);


    protected:
        /**
         * Struct for the nodes defining the processor evaluation dependency graph.
         * Each Node represents one processor and stores the processors which are dependent on this
         * processors, hence, the ones which need to be updated afterwards.
         * 
         * \todo    This struct is \b not thread-safe!
         */
        struct DependencyNode {
            /**
             * Creates a new DependencyNode for the given Processor
             * \param   processor   The processor this node represents. Must not be 0.
             */
            DependencyNode(AbstractProcessor* processor);

            /**
             * Adds the given DependencyNode as dependency (child) of this node.
             * \param   dependency  DependencyNode containing a processor which needs to be updated after this one.
             */
            void addDependency(DependencyNode* dependency);

            AbstractProcessor* _processor;              ///< Processor this node wraps around
            bool _isVisualizationProcessor;             ///< Flag whether \a _processor is a VisualizationProcessor (hence, needs an OpenGL context)
            std::set<DependencyNode*> _dependentNodes;  ///< Set of all dependent nodes.
        };

        /**
         * Slot getting called when one of the observed processors got invalidated.
         * The default behaviour is just to set the invalidation level to invalid.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);


        static const std::string loggerCat_;

        std::map<AbstractProcessor*, DependencyNode*> _processorNodeMap;
        tbb::atomic<DependencyNode*> _topNodeToEvaluate;
    };

}

#endif // DIGRAPHVISUALIZATIONPIPELINE_H__
