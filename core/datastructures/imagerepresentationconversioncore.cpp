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

#include "imagerepresentationconversioncore.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/glcontextmanager.h"

#include "core/tools/opengljobprocessor.h"
#include "core/tools/job.h"

namespace campvis {

    ImageRepresentationGL* GlConversion::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == nullptr)
            return nullptr;

        // test source image type via dynamic cast
        if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
            WeaklyTypedPointer wtp = tester->getImageData();

            if (wtp._pointer == nullptr) {
                LERRORC("CAMPVis.core.datastructures.GlConversion", "Could not load image from disk during conversion.");
                return nullptr;
            }

            ImageRepresentationGL* toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), wtp);

            switch (wtp._baseType) {
            case WeaklyTypedPointer::UINT8:
                delete [] static_cast<uint8_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::INT8:
                delete [] static_cast<int8_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::UINT16:
                delete [] static_cast<uint16_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::INT16:
                delete [] static_cast<int16_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::UINT32:
                delete [] static_cast<uint32_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::INT32:
                delete [] static_cast<int32_t*>(wtp._pointer);
                break;
            case WeaklyTypedPointer::FLOAT:
                delete [] static_cast<float*>(wtp._pointer);
                break;
            default:
                tgtAssert(false, "Should not reach this - wrong base data type!");
                break;
            }

            return toReturn;
        }
        else if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(source)) {
            ImageRepresentationGL* toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), tester->getWeaklyTypedPointer());
            return toReturn;
        }

        return nullptr;
    }


    ImageRepresentationLocal* LocalConversion::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
            return ImageRepresentationLocal::create(tester->getParent(), tester->getImageData());
        }
        else if (const ImageRepresentationGL* tester = dynamic_cast<const ImageRepresentationGL*>(source)) {
            OpenGLJobProcessor::ScopedSynchronousGlJobExecution jobGuard;
            WeaklyTypedPointer wtp = tester->getWeaklyTypedPointerCopy();
            if (wtp._pointer != nullptr)
                return ImageRepresentationLocal::create(source->getParent(), wtp);

            return nullptr;
        }

        return nullptr;
    }

}