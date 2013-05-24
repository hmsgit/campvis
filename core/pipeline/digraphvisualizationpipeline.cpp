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

#include "digraphvisualizationpipeline.h"
#include "tgt/tgt_gl.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontext.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"

#include <stack>

namespace campvis {
    const std::string DigraphVisualizationPipeline::loggerCat_ = "CAMPVis.core.datastructures.DigraphVisualizationPipeline";



    DigraphVisualizationPipeline::DependencyNode::DependencyNode(AbstractProcessor* processor)
        : _processor(processor)
    {
        tgtAssert(_processor != 0, "Processor must not be 0.");
        if (_processor != 0) {
            _isVisualizationProcessor = (dynamic_cast<VisualizationProcessor*>(_processor) != 0);
        }
    }

    void DigraphVisualizationPipeline::DependencyNode::addDependency(DependencyNode* dependency) {
        tgtAssert(dependency != 0, "DependencyNode must not be 0.");

        // check for acyclicness
        std::stack<DependencyNode*> nodesToVisit;
        for (std::set<DependencyNode*>::const_iterator it = dependency->_dependentNodes.begin(); it != dependency->_dependentNodes.end(); ++it) {
            nodesToVisit.push(*it);
        }
        while (! nodesToVisit.empty()) {
            DependencyNode* node = nodesToVisit.top();
            nodesToVisit.pop();
            if (node == this) {
                LERROR("Circle in dependency graph detected. DependencyNode insertion aborted.");
                return;
            }
            for (std::set<DependencyNode*>::const_iterator it = dependency->_dependentNodes.begin(); it != dependency->_dependentNodes.end(); ++it) {
                nodesToVisit.push(*it);
            }
        }

        // add dependency
        _dependentNodes.insert(dependency);
    }

// ================================================================================================

    DigraphVisualizationPipeline::DigraphVisualizationPipeline()
        : VisualizationPipeline()
    {

    }

    DigraphVisualizationPipeline::~DigraphVisualizationPipeline() {
        // delete all DependencyNodes
        for (std::map<AbstractProcessor*, DependencyNode*>::iterator it = _processorNodeMap.begin(); it != _processorNodeMap.end(); ++it) {
            delete it->second;
        }
    }

    void DigraphVisualizationPipeline::addProcessor(AbstractProcessor* processor) {
        tgtAssert(processor != 0, "Processor must not be 0!");

        // add processor to processor list and connect signals
        VisualizationPipeline::addProcessor(processor);
        processor->s_invalidated.connect<DigraphVisualizationPipeline>(this, &DigraphVisualizationPipeline::onProcessorInvalidated);

        // create DependencyNode
        std::map<AbstractProcessor*, DependencyNode*>::iterator lb = _processorNodeMap.lower_bound(processor);
        if (lb == _processorNodeMap.end() || lb->first != processor) {
            DependencyNode* node = new DependencyNode(processor);
            _processorNodeMap.insert(lb, std::make_pair(processor, node));
        }
        else {
            LDEBUG("Processor already added!");
        }
    }

    void DigraphVisualizationPipeline::addProcessorDependency(AbstractProcessor* fatherProc, AbstractProcessor* childProc) {
        tgtAssert(fatherProc != 0, "Father Processor must not be 0!");
        tgtAssert(childProc != 0, "Child Processor must not be 0!");

        // find corresponding DependencyNodes
        std::map<AbstractProcessor*, DependencyNode*>::iterator fatherNode = _processorNodeMap.find(fatherProc);
        std::map<AbstractProcessor*, DependencyNode*>::iterator childNode = _processorNodeMap.find(childProc);
        if (fatherNode == _processorNodeMap.end() || childNode == _processorNodeMap.end()) {
            tgtAssert(false, "Add processors first before defining dependencies between them!");
            LERROR("Could not find DependencyNodes for at least one of the given processors. No dependency added!");
            return;
        }

        // add dependency
        fatherNode->second->addDependency(childNode->second);
    }

    void DigraphVisualizationPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        // dirty hack - implement proper initialization...
        if (_canvas == 0)
            return;

        // TODO: think about a more elaborate implementation, this one doesn't care about the processor graph
        std::map<AbstractProcessor*, DependencyNode*>::iterator node = _processorNodeMap.find(processor);
        if (node != _processorNodeMap.end()) {
            if (node->second->_isVisualizationProcessor) {
                GLJobProc.enqueueJob(
                    _canvas, 
                    makeJobOnHeap<DigraphVisualizationPipeline, AbstractProcessor*, bool>(this, &DigraphVisualizationPipeline::executeProcessor, processor, true), 
                    OpenGLJobProcessor::SerialJob);
            }
            else {
                SimpleJobProc.enqueueJob(makeJob<DigraphVisualizationPipeline, AbstractProcessor*, bool>(this, &DigraphVisualizationPipeline::executeProcessor, processor, false));
            }
        }
        else {
            LWARNING("Caught invalidation of a processor that is not in the processor graph!");
        }
    }

}
