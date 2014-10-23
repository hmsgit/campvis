// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "abstractprocessordecorator.h"

#include "cgt/assert.h"
#include "cgt/shadermanager.h"

namespace campvis {

    void AbstractProcessorDecorator::addProperties(AbstractProcessor* propCollection) {
    }

    void AbstractProcessorDecorator::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {

    }

    void AbstractProcessorDecorator::renderEpilog(tgt::Shader* shader) {

    }

    std::string AbstractProcessorDecorator::generateHeader() const {
        return "";
    }


// ================================================================================================

    HasProcessorDecorators::~HasProcessorDecorators() {
        for (std::vector<AbstractProcessorDecorator*>::iterator it = _decorators.begin(); it != _decorators.end(); ++it)
            delete *it;
    }

    void HasProcessorDecorators::addDecorator(AbstractProcessorDecorator* decorator) {
        tgtAssert(decorator != 0, "Decorator must not be 0.");
        _decorators.push_back(decorator);
    }

    void HasProcessorDecorators::decoratePropertyCollection(AbstractProcessor* propCollection) const {
        for (std::vector<AbstractProcessorDecorator*>::const_iterator it = _decorators.begin(); it != _decorators.end(); ++it)
            (*it)->addProperties(propCollection);
    }

    void HasProcessorDecorators::decorateRenderProlog(const DataContainer& dataContainer, tgt::Shader* shader) const {
        for (std::vector<AbstractProcessorDecorator*>::const_iterator it = _decorators.begin(); it != _decorators.end(); ++it)
            (*it)->renderProlog(dataContainer, shader);
    }

    void HasProcessorDecorators::decorateRenderEpilog(tgt::Shader* shader) const {
        for (std::vector<AbstractProcessorDecorator*>::const_iterator it = _decorators.begin(); it != _decorators.end(); ++it)
            (*it)->renderEpilog(shader);
    }

    std::string HasProcessorDecorators::getDecoratedHeader() const {
        std::string toReturn;
        for (std::vector<AbstractProcessorDecorator*>::const_iterator it = _decorators.begin(); it != _decorators.end(); ++it)
            toReturn.append((*it)->generateHeader());
        return toReturn;
    }


}
