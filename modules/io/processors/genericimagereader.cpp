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


namespace campvis {
    const std::string GenericImageReader::loggerCat_ = "CAMPVis.modules.io.GenericImageReader";

    GenericImageReader::GenericImageReader() 
        : AbstractProcessor()
        , p_url("Url", "Image URL", "", StringProperty::OPEN_FILENAME)
        , p_targetImageID("TargetImageName", "Target Image ID", "AbstractImageReader.output", DataNameProperty::WRITE)
    {
        addProperty(p_url);
        addProperty(p_targetImageID);
        p_url.s_changed.connect(this, &GenericImageReader::onUrlPropertyChanged);

        this->addReader(new CsvdImageReader());        
        this->addReader(new LtfImageReader());
        this->addReader(new MhdImageReader());
        this->addReader(new RawImageReader());
        this->addReader(new VtkImageReader());

#ifdef CAMPVIS_HAS_MODULE_DEVIL
        this->addReader(new DevilImageReader());
#endif

        this->_ext = "";
        this->_currentlyVisible = nullptr;
    }
    
    GenericImageReader::~GenericImageReader() {
        for(std::map<AbstractImageReader*, MetaProperty*>::iterator it = this->_readers.begin(); it != this->_readers.end(); ++it) {
            delete it->second;
            delete it->first;
        }
    }

    void GenericImageReader::init() {

    }

    void GenericImageReader::deinit() {
        for(std::map<AbstractImageReader*, MetaProperty*>::iterator it = this->_readers.begin(); it != this->_readers.end(); ++it) {
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
        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(this->_ext));
        if(it != this->_readers.end()) {
            if (this->_currentlyVisible != it->second) {
                if(nullptr != this->_currentlyVisible) {
                    this->_currentlyVisible->setVisible(false);
                }
                (it->second)->setVisible(true);
                this->_currentlyVisible = it->second;
            }
            it->first->process(data);
        }
    }

    void GenericImageReader::setVisibibility(const std::string& extention, bool visibility) {
        std::string _ext = extention;
        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(_ext));
        if(it != this->_readers.end()) {
            if(nullptr != this->_currentlyVisible) {
                this->_currentlyVisible->setVisible(!visibility);
            }
            (it->second)->setVisible(visibility);
            this->_currentlyVisible = it->second;
            //(it->first)->process(data);
        }
    }

    int GenericImageReader::addReader(AbstractImageReader* reader) {
        MetaProperty* meta = new MetaProperty(reader->getName() + "MetaProp", reader->getName());
        meta->addPropertyCollection(*reader);
        meta->setVisible(false);
        this->addProperty(*meta);

        p_url.addSharedProperty(&reader->p_url);
        reader->p_url.setVisible(false);
        p_targetImageID.addSharedProperty(&reader->p_targetImageID);
        reader->p_targetImageID.setVisible(false);

        this->_readers.insert(std::pair<AbstractImageReader*, MetaProperty*>(reader, meta));
        return 0;
    }

    void GenericImageReader::onUrlPropertyChanged(const AbstractProperty* prop) {
        // first set visibility of old extension to false
        setVisibibility(_ext, false);

        // now update extension
        const std::string& url = this->p_url.getValue();
        size_t extPos = url.rfind('.');
        if (extPos != std::string::npos) {
            this->_ext = url.substr(extPos);
        }

        // set visibility of new extension's properties to true
        setVisibibility(_ext, true);

    }

    void GenericImageReader::adjustToNewExtension() {

    }

}