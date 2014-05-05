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

#ifndef GENERICIMAGEREADER_H__
#define GENERICIMAGEREADER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/metaproperty.h"
#include "core/properties/stringproperty.h"

#include "modules/io/processors/csvdimagereader.h"
#include "modules/io/processors/ltfimagereader.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/io/processors/rawimagereader.h"
#include "modules/io/processors/vtkimagereader.h"

namespace campvis {
    /**
     * Reads an image file into the pipeline. This is mainly a wrapper class. It uses
     * the other image reader implemented for its tasks.
     *
     */
    class GenericImageReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new GenericImageReader Processor
         **/
        GenericImageReader();

        /**
         * Destructor
         **/
        virtual ~GenericImageReader();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GenericImageReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads an image into the pipeline."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Hossain Mahmud <mahmud@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /// functions to set the property of the readers
        void setURL(std::string p_url);
        void setURL(StringProperty p_url);
        void setURL(const char* p_url);
        void setTargetImageId(DataNameProperty& targetImageId);
        void setTargetImageId(std::string imageId);
        void setTargetImageId(const char* imageId);
        void setTargetImageIdSharedProperty(DataNameProperty* sharedProperty);


        void setVisibibility(const std::string& extention, bool visibility);

        StringProperty p_url;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
        
    private:
        void onUrlPropertyChanged(const AbstractProperty*);

        void adjustToNewExtension();

        std::map<AbstractImageReader*, MetaProperty*> _readers;
        std::string _ext;
        MetaProperty* _currentlyVisible;

        /**
        * Adds a particular reader to the generic reader
        * Creates MetaProperty wrapper for the added reader, that
        * is freed from the destructor of the class
        * 
        * /param reader    pointer to the reader to be added
        */
        int addReader(AbstractImageReader* reader);

    };


    struct checkExt {
        checkExt( std::string str ) : _str(str) {}
        bool operator()( const std::pair<AbstractImageReader*, MetaProperty*>& v ) const { 
            return v.first->acceptsExtension(this->_str); 
        }
    private:
        std::string _str;
    };
}

#endif // GENERICIMAGEREADER_H__
