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
		PropertyCollection& getMetaProperties();

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
