// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "modules/modulesapi.h"
#include "modules/io/processors/abstractimagereader.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include "modules/devil/processors/devilimagereader.h"
#endif

namespace campvis {
    /**
     * Reads an image file into the pipeline. This is mainly a wrapper class. It uses
     * the other image reader implemented for its tasks.
     *
     */
    class CAMPVIS_MODULES_API GenericImageReader : public AbstractProcessor {
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

        void updateVisibility(const std::string& extension);

        StringProperty p_url;               ///< URL for file to read
        DataNameProperty p_targetImageID;   ///< image ID for read image

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
        
    private:
        void onUrlPropertyChanged(const AbstractProperty* prop);

        std::map<AbstractImageReader*, MetaProperty*> _readers;
        MetaProperty* _currentlyVisible;

        /**
        * Adds a particular reader to the generic reader
        * Creates MetaProperty wrapper for the added reader, that
        * is freed from the destructor of the class
        * 
        * /param reader    pointer to the reader to be added
        */
        void addReader(AbstractImageReader* reader);

        /**
         * Returns an iterator to the corresponding reader-property pair that can read a file with
         * the given extension. If there is no matching reader found, the RawImageReader is returned.
         * \param   extension   Extension to look for.
         * \return  An iterator to the corresponding reader-property pair.
         */
        std::map<AbstractImageReader*, MetaProperty*>::const_iterator findReader(const std::string& extension) const;

    };

}

#endif // GENERICIMAGEREADER_H__
