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

#include "geometry2dtransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/classification/tfgeometry2d.h"

namespace campvis {

    const std::string Geometry2DTransferFunction::loggerCat_ = "CAMPVis.core.classification.Geometry2DTransferFunction";

    Geometry2DTransferFunction::Geometry2DTransferFunction(const tgt::svec2& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/) 
        : GenericGeometryTransferFunction<TFGeometry2D>(tgt::svec3(size, 1), intensityDomain)
    {
    }

    Geometry2DTransferFunction::~Geometry2DTransferFunction() {
    }

    Geometry2DTransferFunction* Geometry2DTransferFunction::clone() const {
        Geometry2DTransferFunction* toReturn = new Geometry2DTransferFunction(_size.xy(), _intensityDomain);
        for (std::vector<TFGeometry2D*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            toReturn->addGeometry((*it)->clone());
        }
        return toReturn;
    }

    size_t Geometry2DTransferFunction::getDimensionality() const {
        return 2;
    }

}