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

#include "abstractimagerepresentation.h"
#include "core/datastructures/imagedata.h"

namespace campvis {
    const std::string AbstractImageRepresentation::loggerCat_ = "CAMPVis.core.datastructures.AbstractImageRepresentation";

    AbstractImageRepresentation::AbstractImageRepresentation(ImageData* parent)
        : _parent(parent)
    {
        tgtAssert(parent != 0, "Parent ImageData must not be 0!");
    }

    AbstractImageRepresentation::~AbstractImageRepresentation() {
    }

    const ImageData* AbstractImageRepresentation::getParent() const {
        return _parent;
    }

    size_t AbstractImageRepresentation::getDimensionality() const {
        return _parent->getDimensionality();
    }

    const tgt::svec3& AbstractImageRepresentation::getSize() const {
        return _parent->getSize();
    }

    size_t AbstractImageRepresentation::getNumElements() const {
        return _parent->getNumElements();
    }

    void AbstractImageRepresentation::addToParent() const {
        const_cast<ImageData*>(_parent)->addRepresentation(this);
    }


}