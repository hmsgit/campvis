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

#ifndef DATANAMEPROPERTY_H__
#define DATANAMEPROPERTY_H__

#include "core/properties/genericproperty.h"

#include <ctime>
#include <set>
#include <string>

namespace campvis {

    class DataNameProperty : public GenericProperty<std::string> {
    public:
        /**
         * DataAccessInfo flag defining whether processor will read or write from/to the corresponding DataHandle.
         */
        enum DataAccessInfo {
            READ,
            WRITE
        };

        /**
         * Creates a new DataNameProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value
         * \param access    DataAccessInfo flag defining whether processor will read or write from/to the DataHandle with the given ID.
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        DataNameProperty(const std::string& name, const std::string& title, const std::string& value, DataAccessInfo access, int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~DataNameProperty();


        /**
         * Connects this property with the given DataNameProperty \a reader.
         * \param   reader  DataNameProperty to connect to, must be have read access.
         */
        void connect(DataNameProperty* reader);

        /**
         * Disconnects this property from the given DataNameProperty \a reader.
         * \param   reader  DataNameProperty to disconnect from, must be have read access.
         */
        void disconnect(DataNameProperty* reader);

        /**
         * Returns the access info of this property defining whether processor will read or write.
         * \return  _accessInfo
         */
        DataAccessInfo getAccessInfo() const;

        void issueWrite();

    protected:

        void notifyReaders();

        DataAccessInfo _accessInfo;
        std::set<DataNameProperty*> _connectedReaders;

        static const std::string loggerCat_;
    };

}

#endif // DATANAMEPROPERTY_H__
