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

#ifndef LTFIMAGEREADER_H__
#define LTFIMAGEREADER_H__

#include "AbstractImageReader.h"

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/tools/endianhelper.h"
#include "core/tools/weaklytypedpointer.h"

namespace campvis {
    /**
     * Reads raw images into the pipeline.
     */
    class LtfImageReader : public AbstractImageReader {
    public:
        /**
         * Constructs a new LtfImageReader Processor
         **/
        LtfImageReader();

        /**
         * Destructor
         **/
        virtual ~LtfImageReader();


        /**
         * Reads the raw file into an ImageRepresentationDisk representation
         * \param data  DataContainer to work on
         */
        virtual void process(DataContainer& data);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "LtfImageReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads a series of raw images into the pipeline."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
		
        /// \see AbstractProcessor::getExtension()
		virtual const std::string getExtension() { return _ext;};
        /// \see AbstractProcessor::getMetaProperties()
		//PropertyCollection& getMetaProperties();

        IVec3Property p_size;               ///< Image size
        IntProperty p_numChannels;          ///< Number of channels per element
        GenericOptionProperty<WeaklyTypedPointer::BaseType> p_baseType; ///< Base type

        Vec3Property p_imageOffset;         ///< Image Offset in mm
        Vec3Property p_voxelSize;           ///< Voxel Size in mm

    protected:

        static const std::string loggerCat_;
    };

}

#endif // LTFIMAGEREADER_H__
