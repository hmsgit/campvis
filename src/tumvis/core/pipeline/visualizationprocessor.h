// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef VISUALIZATIONPROCESSOR_H__
#define VISUALIZATIONPROCESSOR_H__

#include "tgt/vector.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {

    /**
     * Specialization of AbstractProcessor for visualization purposes.
     * VisualizationProcessors are required to be called by a VisualizationPipeline which ensure
     * to provide a valid OpenGL context when calling the processor's process() method. Hence, a
     * VisualizationProcessor is allowed/capable of performing OpenGl operations.
     * For determining the canvas/viewport size, a VisualizationProcessor gets a reference to the
     * parent pipeline's render target size property during instantiation.
     * 
     * \sa VisualizationPipeline
     */
    class VisualizationProcessor : public AbstractProcessor {
    public:

        /**
         * Creates a VisualizationProcessor.
         * \note    The render target size property of this VisualizationProcessor will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \param   renderTargetSize    Reference to the parent pipeline's render target size property.
         */
        VisualizationProcessor(GenericProperty<tgt::ivec2>& renderTargetSize);

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationProcessor();


    protected:
        GenericProperty<tgt::ivec2> _renderTargetSize;        ///< Viewport size of target canvas
    };

}

#endif // VISUALIZATIONPROCESSOR_H__
