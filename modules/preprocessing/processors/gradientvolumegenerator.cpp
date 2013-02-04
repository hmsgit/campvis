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

#include "gradientvolumegenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "tbb/include/tbb/tbb.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    class ApplyCentralDifferences {
    public:
        ApplyCentralDifferences(const ImageRepresentationLocal* input, GenericImageRepresentationLocal<float, 4>* output)
            : _input(input)
            , _output(output)
        {
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                tgt::svec3 pos = _input->getParent()->indexToPosition(i);
                const tgt::svec3& size = _input->getSize();

                float dx, dy, dz, mdx, mdy, mdz;
                dx = dy = dz = mdx = mdy = mdz = 0.f;

                if (pos.x != size.x - 1)
                    dx = _input->getElementNormalized(pos + tgt::svec3(1, 0, 0), 0);
                if (pos.y != size.y - 1)
                    dy = _input->getElementNormalized(pos + tgt::svec3(0, 1, 0), 0);
                if (pos.z != size.z - 1)
                    dz = _input->getElementNormalized(pos + tgt::svec3(0, 0, 1), 0);

                if (pos.x != 0)
                    mdx = _input->getElementNormalized(pos + tgt::svec3(-1, 0, 0), 0);
                if (pos.y != 0)
                    mdy = _input->getElementNormalized(pos + tgt::svec3(0, -1, 0), 0);
                if (pos.z != 0)
                    mdz = _input->getElementNormalized(pos + tgt::svec3(0, 0, -1), 0);


                tgt::vec3 gradient(mdx - dx, mdy - dy, mdz - dz);
                gradient /= _input->getParent()->getMappingInformation().getVoxelSize() * tgt::vec3(2.f);
                
                _output->setElement(i, tgt::vec4(gradient, tgt::length(gradient)));
            }
        }

    protected:
        const ImageRepresentationLocal* _input;
        GenericImageRepresentationLocal<float, 4>* _output;
    };

// ================================================================================================

    const std::string GradientVolumeGenerator::loggerCat_ = "CAMPVis.modules.classification.GradientVolumeGenerator";

    GradientVolumeGenerator::GradientVolumeGenerator()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
    }

    GradientVolumeGenerator::~GradientVolumeGenerator() {

    }

    void GradientVolumeGenerator::process(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());

        if (input != 0) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 4);
            GenericImageRepresentationLocal<float, 4>* output = GenericImageRepresentationLocal<float, 4>::create(id, 0);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, input->getNumElements()), ApplyCentralDifferences(input, output));

            data.addData(p_targetImageID.getValue(), id);
            p_targetImageID.issueWrite();
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        _invalidationLevel.setValid();
    }

}
