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

#ifndef GEOMETRYRENDERER_H__
#define GEOMETRYRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class GeometryRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new GeometryRenderer Processor
         **/
        GeometryRenderer(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~GeometryRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GeometryRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders Geometry."; };

        virtual void process(DataContainer& data);

        DataNameProperty _geometryID;       ///< ID for input geometry
        DataNameProperty _renderTargetID;   ///< image ID for output image
        CameraProperty _camera;

        Vec4Property _color;                ///< rendering color

    protected:

        tgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };

}

#endif // GEOMETRYRENDERER_H__
