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
        , p_url("url", "Image URL", "")
	{
		//CsvdImageReader *_csvdImageReader = new CsvdImageReader();
		//this->_readers.push_back(_csvdImageReader);
		//LtfImageReader *_ltfImageReader = new LtfImageReader();
		//this->_readers.push_back(_ltfImageReader);
		//MhdImageReader *_mhdImageReader = new MhdImageReader();
		//this->_readers.push_back(_mhdImageReader);
		//RawImageReader *_rawImageReader = new RawImageReader();
		//this->_readers.push_back(_rawImageReader);
		//VtkImageReader *_vtkImageReader = new VtkImageReader();
		//this->_readers.push_back(_vtkImageReader);

		this->_readers.push_back(new CsvdImageReader());
		this->_readers.push_back(new LtfImageReader());
		this->_readers.push_back(new MhdImageReader());
		this->_readers.push_back(new RawImageReader());
		this->_readers.push_back(new VtkImageReader());
	}

    GenericImageReader::~GenericImageReader() {
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			delete *it;
		}

    }

    void GenericImageReader::process(DataContainer& data) {
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			if((*it)->getExtension() == this->_ext) {
				(*it)->process(data);
				break;
			}
		}
		return;
    }

	void GenericImageReader::setURL(StringProperty p_url) {
		this->p_url.setValue(p_url.getValue());
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			if((*it)->getExtension() == this->_ext) {
				(*it)->p_url.setValue(this->p_url.getValue());
				break;
			}
		}

		std::string url = this->p_url.getValue();
		unsigned extPos = (unsigned) url.rfind('.');
		if (extPos != std::string::npos) {
			this->_ext = url.substr(extPos);
		}
		return;
	}

	void GenericImageReader::setTargetImageId(DataNameProperty& targetImageId) {
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			if((*it)->getExtension() == this->_ext) {
				(*it)->p_targetImageID.setValue(targetImageId.getValue());
				std::set<AbstractProperty*> sharedProperties = targetImageId.getSharedProperties();
				for(std::set<AbstractProperty*>::iterator jt = sharedProperties.begin(); jt != sharedProperties.end(); jt++) {
					(*it)->p_targetImageID.addSharedProperty(*jt);
				}
				break;
			}
		}
		return;
	}

	void GenericImageReader::setTargetImageId(std::string imageId) {
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			if((*it)->getExtension() == this->_ext) {
				(*it)->p_targetImageID.setValue(imageId);
				break;
			}
		}
		return;
	}

	void GenericImageReader::setTargetImageIdSharedProperty(DataNameProperty* sharedProperty) {
		for(std::vector<AbstractImageReader*>::iterator it = this->_readers.begin(); it != this->_readers.end(); it++) {
			if((*it)->getExtension() == this->_ext) {
				(*it)->p_targetImageID.addSharedProperty(sharedProperty);
				break;
			}
		}
		return;
	}

	void GenericImageReader::setMetaProperties(PropertyCollection &metaProperties) {
		return;
	}

}