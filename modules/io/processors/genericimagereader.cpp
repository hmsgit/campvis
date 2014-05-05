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

#include "genericimagereader.h"

#include <fstream>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"


namespace campvis {
    const std::string GenericImageReader::loggerCat_ = "CAMPVis.modules.io.MhdImageReader";

    GenericImageReader::GenericImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "", StringProperty::OPEN_FILENAME)
    {
        addProperty(p_url);
        p_url.s_changed.connect(this, &GenericImageReader::onUrlPropertyChanged);

        this->addReader(new CsvdImageReader());        
        this->addReader(new LtfImageReader());
        this->addReader(new MhdImageReader());
        this->addReader(new RawImageReader());
        this->addReader(new VtkImageReader());


        this->_ext = "";
        this->_currentlyVisible = nullptr;
    }
    
    GenericImageReader::~GenericImageReader() {
    }

    void GenericImageReader::init() {

    }

    void GenericImageReader::deinit() {
        for(std::map<AbstractImageReader*, MetaProperty*>::iterator it = this->_readers.begin(); it != this->_readers.end(); ++it) {
            if (nullptr != it->first) delete it->first;
            if (nullptr != it->second) delete it->second;
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
            (it->first)->process(data);
        }
        
        validate(INVALID_RESULT);
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

    void GenericImageReader::setURL(std::string p_url) {
        this->p_url.setValue(p_url);

        std::string url = this->p_url.getValue();
        size_t extPos = url.rfind('.');
        if (extPos != std::string::npos) {
            this->_ext = url.substr(extPos);
        }

        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(this->_ext));
        if(it != this->_readers.end()) {
            (it->first)->p_url.setValue(this->p_url.getValue());
        }
        return;
    }

    void GenericImageReader::setURL(StringProperty p_url) {
        return this->setURL(p_url.getValue());
    }
    
    void GenericImageReader::setURL(const char* p_url) {
        return this->setURL(std::string(p_url));
    }

    void GenericImageReader::setTargetImageId(DataNameProperty& targetImageId) {
        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(this->_ext));
        if(it != this->_readers.end()) {
            (it->first)->p_targetImageID.setValue(targetImageId.getValue());
            std::set<AbstractProperty*> sharedProperties = targetImageId.getSharedProperties();
            for(std::set<AbstractProperty*>::iterator jt = sharedProperties.begin(); jt != sharedProperties.end(); ++jt) {
                (it->first)->p_targetImageID.addSharedProperty(*jt);
            }
        }
        return;
    }
    
    void GenericImageReader::setTargetImageId(const char* imageId) {
        return this->setTargetImageId(std::string(imageId));
    }
    void GenericImageReader::setTargetImageId(std::string imageId) {
        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(this->_ext));
        if(it != this->_readers.end()) {
            (it->first)->p_targetImageID.setValue(imageId);
        }
        return;
    }

    void GenericImageReader::setTargetImageIdSharedProperty(DataNameProperty* sharedProperty) {
        std::map<AbstractImageReader*, MetaProperty*>::iterator it = std::find_if(this->_readers.begin(), this->_readers.end(), checkExt(this->_ext));
        if(it != this->_readers.end()) {
            (it->first)->p_targetImageID.addSharedProperty(sharedProperty);
        }
        return;
    }

    int GenericImageReader::addReader(AbstractImageReader* reader) {
        MetaProperty* meta = new MetaProperty(reader->getName()+"MetaProp", reader->getName());
        meta->addPropertyCollection(*reader);
        meta->setVisible(false);


        StringProperty* sp = dynamic_cast<StringProperty*>(meta->getProperty("url"));
        tgtAssert(sp != 0, "This should not happen.");
        if (sp != 0) {
            p_url.addSharedProperty(sp);
            sp->setVisible(false);
        }

        this->addProperty(*meta);
        this->_readers.insert(std::pair<AbstractImageReader*, MetaProperty*>(reader, meta));
        return 0;
    }

    void GenericImageReader::onUrlPropertyChanged(const AbstractProperty*) {
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