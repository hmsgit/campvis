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

#ifndef ABSTRACTPROPERTY_H__
#define ABSTRACTPROPERTY_H__

#include "sigslot/sigslot.h"
#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>
#include "tgt/logmanager.h"
#include "core/pipeline/abstractprocessor.h"

#include <set>
#include <string>

namespace campvis {
    /**
     * Abstract base class for CAMPVis Property.
     * 
     * \todo    Add PropertyWidgets, add clone()?
     *          Think about a reasonable locking mechanism and implement that
     */
    class AbstractProperty : public sigslot::has_slots<> {
    public:
        /**
         * Creates a new AbstractProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        AbstractProperty(const std::string& name, const std::string& title, int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractProperty();


        /**
         * Initializes the property.
         * Everything that requires a valid OpenGL context or is otherwise expensive gets in here.
         */
        virtual void init();
        
        /**
         * Deinitializes this property.
         * Everything that requires a valid OpenGL context gets in here instead into the dtor.
         */
        virtual void deinit();

        /**
         * Returns the property name.
         * \return  _name
         */
        const std::string& getName();

        /**
         * Returns the property title (e.g. used for GUI).
         * \return  _title
         */
        const std::string& getTitle();

        /**
         * Returns the invalidation level that this property triggers.
         * \return  _invalidationLevel
         */
        int getInvalidationLevel() const;

        /**
         * Sets the invalidation level that this property triggers.
         * \param il    New invalidation level that this property triggers.
         */
        void setInvalidationLevel(AbstractProcessor::InvalidationLevel il);

        /**
         * Returns whether this proberty shall be visible in the GUI.
         * \return  _isVisible
         */
        bool isVisible() const;

        /**
         * Sets whether this property shall be visible in the GUI.
         * \param isVisible new visibility flag
         */
        void setVisible(bool isVisible);

        /**
         * Adds the given property \a prop to the set of shared properties.
         * All shared properties will be changed when this property changes.
         * Overload in subclasses to make sure that shared properties are of the same type.
         * \note        Make sure not to build circular sharing or you will encounter endless loops!
         * \param prop  Property to add.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Removes the given property \a prop from the set of shared properties.
         * \param prop  Property to remove.
         */
        void removeSharedProperty(AbstractProperty* prop);

        /**
         * Returns the list of shared properties, which will be changed when this property changes.
         * \note    Property sharing only works in one direction, i.e. shared properties act as child properties.
         *          Make sure not to build circular sharing structures or you will encounter endless loops.
         * \return  _sharedProperties
         */
        const std::set<AbstractProperty*>& getSharedProperties() const;


        /**
         * Locks the property and marks it as "in use". Overwrite if necessary.
         * \sa  AbstractProperty::unlock
         */
        virtual void lock();

        /**
         * Unlocks the property and marks it as "not in use". Overwrite if necessary.
         * \sa  AbstractProperty::lock
         */
        virtual void unlock();


        /// Signal emitted, when the property changes.
        sigslot::signal1<const AbstractProperty*> s_changed;

        /// Signal emitted, when the visibility of this property changed
        sigslot::signal1<const AbstractProperty*> s_visibilityChanged;

    protected:

        void onChanged(const AbstractProperty* prop);
        
        // DO NOT REMOVE THE CONSTNESS OF _name. PropertyCollection relies on it!
        const std::string _name;                ///< Property name (unchangable on purpose!)
        std::string _title;                     ///< Property title (e.g. used for GUI)
        int _invalidationLevel;                 ///< Invalidation level that this property triggers
        tbb::atomic<bool> _isVisible;           ///< Flag whether this property shall be visible in the GUI

        tbb::atomic<int> _inUse;                ///< flag whether property is currently in use and values are written to back buffer
        tbb::spin_mutex _localMutex;            ///< Mutex used when altering local members

        /**
         * List of shared properties that will be changed when this property changes.
         * \note    Property sharing only works in one direction, i.e. shared properties act as child properties.
         *          Make sure not to build circular sharing structures or you will encounter endless loops.
         */
        std::set<AbstractProperty*> _sharedProperties;

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTPROPERTY_H__
