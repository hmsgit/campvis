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

#ifndef VTKIMAGEREADER_H__
#define VTKIMAGEREADER_H__

#include <string>
#include "abstractimagereader.h"

#include "cgt/exception.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Reads a VTK image file into the pipeline.
     *
     * \note    Full format specification at http://www.vtk.org/VTK/img/file-formats.pdf
     */
    class CAMPVIS_MODULES_API VtkImageReader : public AbstractImageReader {
    public:
        /**
         * Constructs a new VtkImageReader Processor
         **/
        VtkImageReader();

        /**
         * Destructor
         **/
        virtual ~VtkImageReader();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VtkImageReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads a VTK image into the pipeline."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        
        Vec3Property p_imageOffset;         ///< Image Offset in mm
        Vec3Property p_voxelSize;           ///< Voxel Size in mm

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        void parseStructuredPoints(DataContainer& data, std::ifstream& file) throw (cgt::Exception, std::exception);

        void parsePolydata(DataContainer& data, std::ifstream& file) throw (cgt::Exception, std::exception);

        static const std::string loggerCat_;
    };

}

#endif // VTKIMAGEREADER_H__
