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

#include "trackedusfilereader.h"
#include "tgt/filesystem.h"
#include "modules/scr_msk/datastructures/usinterfacedata.h"


namespace campvis {
    const std::string TrackedUsFileReader::loggerCat_ = "CAMPVis.modules.io.TrackedUsFileReader";

    TrackedUsFileReader::TrackedUsFileReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "TrackedUsFileReader.output", DataNameProperty::WRITE)
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
    }

    TrackedUsFileReader::~TrackedUsFileReader() {

    }

    void TrackedUsFileReader::process(DataContainer& data) {
        try {
            std::string path = tgt::FileSystem::parentDir(p_url.getValue());
            if (! tgt::FileSystem::fileExists(path + "/content.xml")) {
                LERROR("No Tracked US data in this directory");
                validate(INVALID_RESULT);
                return;
            }

            TrackedUSFileIO* fio = new TrackedUSFileIO();
            fio->parse(path.c_str());
            if (! fio->open(0)) {
                validate(INVALID_RESULT);
                return;
            }

            TrackedUsFileIoData* file = new TrackedUsFileIoData(fio);
            data.addData(p_targetImageID.getValue(), file);
        }
        catch (std::exception& e) {
            LERROR("Error : " << e.what());
            validate(INVALID_RESULT);
            return;
        }

        validate(INVALID_RESULT);
    }
}