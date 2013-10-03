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

#ifndef METAPROPERTY_H__
#define METAPROPERTY_H__


#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

namespace campvis {
    /**
     * Property wrapping around a bunch of other properties.
     * Useful either for grouping properties or for wrapping around entire property collections.
     */
    class MetaProperty : public AbstractProperty, public HasPropertyCollection {
    public:
        /**
         * Creates a new MetaProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        MetaProperty(const std::string& name, const std::string& title, int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~MetaProperty();

                
        /// \see AbstractProperty::init()
        virtual void init();
        /// \see AbstractProperty::deinit()
        virtual void deinit();
        /// \see AbstractProperty::lock()
        virtual void lock();
        /// \see AbstractProperty::unlock()
        virtual void unlock();

        /// \see HasPropertyCollection::onPropertyChanged
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Adds all properties in \a pc to this meta property.
         * \param   pc  PropertyCollection to add
         */
        void addPropertyCollection(HasPropertyCollection& pc);

    protected:

        static const std::string loggerCat_;
    };

}

#endif // METAPROPERTY_H__
