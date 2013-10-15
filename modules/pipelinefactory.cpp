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

#include "pipelinefactory.h"
#include "gen_pipelineregistration.h"

#include <sstream>

namespace campvis {

    // declare one single symbol for the PipelineFactory singleton
    tbb::atomic<PipelineFactory*> PipelineFactory::_singleton;

    PipelineFactory& PipelineFactory::getRef() {
        if (_singleton == 0) {
            std::cout << "creating PipelineFactory...\n";
            PipelineFactory* tmp = new PipelineFactory();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }
    
    void PipelineFactory::deinit() {
	delete _singleton;
    }


    std::vector<std::string> PipelineFactory::getRegisteredPipelines() const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        std::vector<std::string> toReturn;
        toReturn.reserve(_pipelineMap.size());
        for (std::map<std::string, AbstractPipeline* (*)(DataContainer*)>::const_iterator it = _pipelineMap.begin(); it != _pipelineMap.end(); ++it)
            toReturn.push_back(it->first);
        return toReturn;
    }

    AbstractPipeline* PipelineFactory::createPipeline(const std::string& id, DataContainer* dc) const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        std::map<std::string, AbstractPipeline* (*)(DataContainer*)>::const_iterator it = _pipelineMap.find(id);
        if (it == _pipelineMap.end())
            return 0;
        else
            return (it->second)(dc);
    }

}