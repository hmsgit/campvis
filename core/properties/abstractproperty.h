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
    class AbstractProperty {
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
        const std::string& getName() const;

        /**
         * Returns the property title (e.g. used for GUI).
         * \return  _title
         */
        const std::string& getTitle() const;

        /**
         * Returns the invalidation level that this property triggers.
         * \return  _invalidationLevel
         */
        int getInvalidationLevel() const;

        /**
         * Sets the invalidation level that this property triggers.
         * \param il    New invalidation level that this property triggers.
         */
        void setInvalidationLevel(int il);

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
