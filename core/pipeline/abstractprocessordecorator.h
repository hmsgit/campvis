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

#ifndef ABSTRACTPROCESSORDECORATOR_H__
#define ABSTRACTPROCESSORDECORATOR_H__

#include <vector>
#include <string>

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
    class AbstractProcessorDecorator {
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
    class HasProcessorDecorators {
    public:
        /**
         * Virtual Destructor, deletes all Decorators
         */
        virtual ~HasProcessorDecorators();

    protected:
        void addDecorator(AbstractProcessorDecorator* decorator);

        void decoratePropertyCollection(HasPropertyCollection* propCollection) const;

        void decorateRenderProlog(const DataContainer& dataContainer, tgt::Shader* shader) const;

        void decorateRenderEpilog(tgt::Shader* shader) const;

        std::string getDecoratedHeader() const;

        std::vector<AbstractProcessorDecorator*> _decorators;   ///< List of all processor decorators
    };

}

#endif // ABSTRACTPROCESSORDECORATOR_H__
