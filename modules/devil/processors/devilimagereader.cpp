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

#include "devilimagereader.h"


#include <IL/il.h>
#include <cstring>

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/texturereaderdevil.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/genericimagedatalocal.h"


namespace campvis {
    const std::string DevilImageReader::loggerCat_ = "CAMPVis.modules.io.DevilImageReader";

    DevilImageReader::DevilImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "DevilImageReader.output", DataNameProperty::WRITE)
        , _devilTextureReader(0)
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);

        _devilTextureReader = new tgt::TextureReaderDevil();
    }

    DevilImageReader::~DevilImageReader() {
        delete _devilTextureReader;
    }

    void DevilImageReader::process(DataContainer& data) {
        try {
            tgt::Texture* tex = _devilTextureReader->loadTexture(p_url.getValue(), tgt::Texture::LINEAR, false, true, true, false);
            if (tex == 0) {
                LERROR("Unknown error while loading image.");
                return;
            }

            // all parsing done - lets create the image:
            ImageDataGL* image = ImageDataGL::createFromTexture(tex);
            data.addData(p_targetImageID.getValue(), image);
            p_targetImageID.issueWrite();
        }
        catch (tgt::Exception& e) {
            LERROR("Error while loading image: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while loading image: " << e.what());
            return;
        }

        _invalidationLevel.setValid();
    }
}