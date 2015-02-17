// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "genericimagereader.h"

#include <fstream>

#include "cgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

#include "modules/io/processors/amirameshreader.h"
#include "modules/io/processors/csvdimagereader.h"
#include "modules/io/processors/ltfimagereader.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/io/processors/niftiimagereader.h"
#include "modules/io/processors/rawimagereader.h"
#include "modules/io/processors/vtkimagereader.h"

namespace campvis {
    namespace {
        struct checkExtension {
            checkExtension(const std::string& str) : _str(str) {}

            bool operator()(const std::pair<AbstractImageReader*, MetaProperty*>& v) const { 
                return v.first->acceptsExtension(this->_str); 
            }

        private:
            std::string _str;
        };
    }

    const std::string GenericImageReader::loggerCat_ = "CAMPVis.modules.io.GenericImageReader";

    GenericImageReader::GenericImageReader() 
        : AbstractProcessor()
        , p_url("Url", "Image URL", "", StringProperty::OPEN_FILENAME)
        , p_targetImageID("TargetImageName", "Target Image ID", "AbstractImageReader.output", DataNameProperty::WRITE)
        , _currentlyVisible(nullptr)
    {
        addProperty(p_url);
        addProperty(p_targetImageID);
        p_url.s_changed.connect(this, &GenericImageReader::onUrlPropertyChanged);

        addReader(new AmiraMeshReader());        
        addReader(new CsvdImageReader());        
        addReader(new LtfImageReader());
        addReader(new MhdImageReader());
        addReader(new NiftiImageReader());
        addReader(new RawImageReader());
        addReader(new VtkImageReader());

#ifdef CAMPVIS_HAS_MODULE_DEVIL
        addReader(new DevilImageReader());
#endif
    }
    
    GenericImageReader::~GenericImageReader() {
        for (auto it = _readers.begin(); it != _readers.end(); ++it) {
            delete it->second;
            delete it->first;
        }
    }

    void GenericImageReader::init() {

    }

    void GenericImageReader::deinit() {
        for(std::map<AbstractImageReader*, MetaProperty*>::iterator it = _readers.begin(); it != _readers.end(); ++it) {
            // deinit MetaProperty first!
            if (nullptr != it->second) {
                it->second->deinit();
            }
            // then we can delete the reader!
            if (nullptr != it->first) {
                it->first->deinit();
            }
        }
    }

    void GenericImageReader::updateResult(DataContainer& data) {
        const std::string extension = cgt::FileSystem::fileExtension(p_url.getValue());
        auto it = findReader(extension);

        if (it != this->_readers.end()) {
            it->first->process(data);
        }
    }

    void GenericImageReader::updateVisibility(const std::string& extension) {
        if (_currentlyVisible != nullptr) {
            _currentlyVisible->setVisible(false);
            _currentlyVisible = nullptr;
        }

        auto it = findReader(extension);
        if (it != this->_readers.end()) {
            it->second->setVisible(true);
            this->_currentlyVisible = it->second;
        }
    }

    void GenericImageReader::addReader(AbstractImageReader* reader) {
        MetaProperty* meta = new MetaProperty(reader->getName() + "MetaProp", reader->getName());
        meta->addPropertyCollection(*reader);
        meta->setVisible(false);
        this->addProperty(*meta);

        p_url.addSharedProperty(&reader->p_url);
        reader->p_url.setVisible(false);
        p_targetImageID.addSharedProperty(&reader->p_targetImageID);
        reader->p_targetImageID.setVisible(false);

        _readers.insert(std::pair<AbstractImageReader*, MetaProperty*>(reader, meta));
    }

    void GenericImageReader::onUrlPropertyChanged(const AbstractProperty* prop) {
        // now update extension
        std::string extension = cgt::FileSystem::fileExtension(p_url.getValue());
        updateVisibility(extension);
    }

    std::map<AbstractImageReader*, MetaProperty*>::const_iterator GenericImageReader::findReader(const std::string& extension) const {
        auto it = std::find_if(_readers.begin(), _readers.end(), checkExtension(extension));
        if (it == _readers.end())
            it = std::find_if(_readers.begin(), _readers.end(), checkExtension("raw"));

        return it;
    }

}