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

#ifndef ABSTRACTIMAGEREADER_H__
#define ABSTRACTIMAGEREADER_H__

#include <string>
#include <vector>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

namespace campvis {
    /**
     * Reads an image file into the pipeline. This is mainly a wrapper class. It uses
     * the other image reader implemented for its tasks.
     *
     * Provides the interface for the classes that reads images from different types
     * of files into ImageRepresentationDisk representation
     *
     */
    class AbstractImageReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new AbstractImageReader Processor
         **/
        AbstractImageReader();

        /**
         * Destructor
         **/
        virtual ~AbstractImageReader();

        /// \see AbstractProcessor::acceptsExtension()
        virtual bool acceptsExtension(const std::string& extenstion);

        /// functions to set the property of the readers
        virtual void setURL(StringProperty p_url);
        virtual void setTargetImageId(DataNameProperty& targetImageId);
        virtual void setTargetImageId(std::string imageId);
        virtual void setTargetImageIdSharedProperty(DataNameProperty* sharedProperty);
        
    public:
        StringProperty p_url;               ///< URL for file to read
        DataNameProperty p_targetImageID;   ///< image ID for read image

    protected:
        std::vector<std::string> _ext;

        static const std::string loggerCat_;
        
    private:

    };

}

#endif // ABSTRACTIMAGEREADER_H__
