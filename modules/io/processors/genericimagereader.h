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

#ifndef GENERICIMAGEREADER_H__
#define GENERICIMAGEREADER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/metaproperty.h"

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

        /**
         * Reads the MHD file into an ImageRepresentationDisk representation
         * \param data  DataContainer to work on
         */
        virtual void process(DataContainer& data);

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


        void setVisibibility(const char* extention, bool visibility);

    protected:
        static const std::string loggerCat_;
        
    private:
        std::map<AbstractImageReader*, MetaProperty*> _readers;
        StringProperty p_url;
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
