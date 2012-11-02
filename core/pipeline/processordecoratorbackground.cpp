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

#include "processordecoratorbackground.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {

    ProcessorDecoratorBackground::ProcessorDecoratorBackground()
        : AbstractProcessorDecorator()
        , _backgroundColor1("backgroundColor1", "Background Color 1", tgt::vec4(.9f, .9f, .9f, 1), tgt::vec4(0.f), tgt::vec4(1.f))
        , _backgroundColor2("backgroundColor2", "Background Color 2", tgt::vec4(.7f, .7f, .7f, 1), tgt::vec4(0.f), tgt::vec4(1.f))
    {
    }

    ProcessorDecoratorBackground::~ProcessorDecoratorBackground() {

    }

    void ProcessorDecoratorBackground::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(&_backgroundColor1);
        propCollection->addProperty(&_backgroundColor2);
    }

    void ProcessorDecoratorBackground::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        shader->setUniform("_backgroundColor1", _backgroundColor1.getValue());
        shader->setUniform("_backgroundColor2", _backgroundColor2.getValue());
    }

}
