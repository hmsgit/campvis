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

#ifndef VIRTUALMIRRORGEOMETRYGENERATOR_H__
#define VIRTUALMIRRORGEOMETRYGENERATOR_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/cameraproperty.h"

namespace campvis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class VirtualMirrorGeometryGenerator : public AbstractProcessor {
    public:
        /**
         * Constructs a new VirtualMirrorGeometryGenerator Processor
         **/
        VirtualMirrorGeometryGenerator();

        /**
         * Destructor
         **/
        virtual ~VirtualMirrorGeometryGenerator();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VirtualMirrorGeometryGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Genereates entry-/exit point textures for the given image and camera."; };

        virtual void process(DataContainer& data);

        DataNameProperty _mirrorID;         ///< ID for output geometry

        Vec3Property _mirrorCenter;         ///< position of mirror center
        Vec3Property _mirrorNormal;         ///< normal of mirror
        FloatProperty _size;                ///< Mirror size
        IntProperty _numVertices;           ///< Number of vertices of mirror

        Vec3Property _poi;                  ///< Point of intereset
        CameraProperty _camera;

    protected:
        void updateClipProperties();

        clock_t _sourceTimestamp;

        static const std::string loggerCat_;
    };

}

#endif // VIRTUALMIRRORGEOMETRYGENERATOR_H__