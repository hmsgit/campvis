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

#ifndef ABSTRACTPROCESSORDECORATOR_H__
#define ABSTRACTPROCESSORDECORATOR_H__

#include <vector>
#include <string>

#include "core/coreapi.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class DataContainer;
    class HasPropertyCollection;

    /**
     * Abstract base class for processor decorators.
     * ProcessorDecorators are created and assigned to a processor during its construction. The
     * virtual methods are then calles at an appropriate time.
     */
    class CAMPVIS_CORE_API AbstractProcessorDecorator {
    public:
        /**
         * Pure virtual Destructor
         */
        virtual ~AbstractProcessorDecorator() {};

        /**
         * Gets called by the processor during initialization to let the decorator
         * add its properties to the processor.
         * \param   propCollection  PropertyCollection defining where to add the properties (usually the owning processor).
         */
        virtual void addProperties(HasPropertyCollection* propCollection);

        /**
         * Gets called by the processor with an active OpenGL context having \a shader activated before the
         * actual rendering takes place.
         * Here is the place to setup additional shader arguments (i.e. uniforms) or bind additional textures.
         * \param   dataContainer   The current processor's DataContainer
         * \param   shader          Shader object used during rendering.
         */
        virtual void renderProlog(const DataContainer& dataContainer, tgt::Shader* shader);

        /**
         * Gets called by the processor after the actual rendering took place.
         * Here is the place to clean up and deinit created resources.
         * \param   shader          Shader object used during rendering.
         */
        virtual void renderEpilog(tgt::Shader* shader);

        /**
         * Returns an additional header that will be linked into the fragment shader.
         * Gets calles when building the shader. Overwrite this method to add a decorator-specific header to
         * your fragment shader.
         * \return  The default implementation returns an empty string.
         */
        virtual std::string generateHeader() const;

    };

// ================================================================================================

    /**
     * Pure virtual base class for processors using decorators.
     */
    class CAMPVIS_CORE_API HasProcessorDecorators {
    public:
        /**
         * Virtual Destructor, deletes all Decorators
         */
        virtual ~HasProcessorDecorators();

    protected:
        /**
         * Adds \a decorator to the list of processor decorators.
         * \param   decorator   Processor decorator to add
         */
        void addDecorator(AbstractProcessorDecorator* decorator);

        /**
         * Decorates \a propCollection with the properties of all held decorators.
         * \param   propCollection  PropertyCollection to decorate (usually your processor)
         */
        void decoratePropertyCollection(HasPropertyCollection* propCollection) const;

        /**
         * Calls renderProlog() on each held decorator.
         * To be called before rendering to decorate \a shader.
         * \param   dataContainer   DataContainer to work on
         * \param   shader          Shader to decorate
         */
        void decorateRenderProlog(const DataContainer& dataContainer, tgt::Shader* shader) const;

        /**
         * Calls renderProlog() on each held decorator.
         * To be called after rendering to decorate \a shader.
         * \param   shader          Shader to decorate
         */
        void decorateRenderEpilog(tgt::Shader* shader) const;

        /**
         * Concatenates and returns the GLSL headers of all held decorators.
         */
        std::string getDecoratedHeader() const;


        std::vector<AbstractProcessorDecorator*> _decorators;   ///< List of all processor decorators
    };

}

#endif // ABSTRACTPROCESSORDECORATOR_H__
