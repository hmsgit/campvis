// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>
#include <tbb/spin_rw_mutex.h>

#include "sigslot/sigslot.h"
#include "tgt/logmanager.h"

#include "core/coreapi.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/propertycollection.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace campvis {
    class AbstractProperty;

    /**
     * Abstract base class for CAMPVis Processors.
     * A processor implements a specific task, which it performs on the DataCollection passed
     * during process(). Properties provide a transparent layer for adjusting the processor's 
     * behaviour.
     * Once a processor has finished it sets it should set its invalidation level to valid. As
     * soon as one of its properties changes, the processor will be notified and possibliy
     * change its invalidation level. Observing pipelines will be notified of this and can
     * and have to decide which part of the pipeline has to be re-evaluated wrt. the processor's
     * invalidation level.
     * 
     * \sa AbstractPipeline
     */
    class CAMPVIS_CORE_API AbstractProcessor : public HasPropertyCollection {
    public:
        /**
         * Scoped lock of an AbstractProcessor that automatically unlocks the processor on destruction.
         * Useful for exception safety.
         */
        struct CAMPVIS_CORE_API ScopedLock {
            /**
             * Constructs a new Scoped lock, locking \a p and unlocking \a p on destruction.
             * \param   p                   Processor to lock
             * \param   unlockInExtraThread Unlock \a p in extra thread (since this might be an expensive operation)
             */
            ScopedLock(AbstractProcessor* p, bool unlockInExtraThread);

            /// Destructor, unlocks the processor
            ~ScopedLock();

            AbstractProcessor* _p;      ///< The processor to lock
            bool _unlockInExtraThread;  ///< Unlock _p in extra thread (since this might be an expensive operation)
        };

        /**
         * Available invalidation levels
         */
        enum InvalidationLevel {
            VALID               = 0,        ///< Valid, no need to run the process() method
            INVALID_RESULT      = 1 << 0,   ///< Need to run the updateResult() method
            INVALID_SHADER      = 1 << 1,   ///< Need to run the updateShader() method (e.g. to recompile the shader)
            INVALID_PROPERTIES  = 1 << 2,   ///< Need to run the updateProperties() method (e.g. to adjust property ranges)
            FIRST_FREE_TO_USE_INVALIDATION_LEVEL = 1 << 3
        };

        /// Current state of a processor in terms of stability.
        enum ProcessorState {
            EXPERIMENTAL,
            TESTING,
            STABLE
        };

        /**
         * Creates a AbstractProcessor.
         */
        AbstractProcessor();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractProcessor();


        /**
         * Initializes the processor.
         * Everything that requires a valid OpenGL context or is otherwise expensive gets in here.
         * 
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();
        
        /**
         * Deinitializes this processor.
         * \note    When overwriting this method, make sure to call the base class version at the end.
         */
        virtual void deinit();

        /**
         * Gets the name of this very processor. To be defined by every subclass.
         * \return  The name of this processor.
         */
        virtual const std::string getName() const = 0;

        /**
         * Gets a description of this processor. To be defined by every subclass.
         * \return  A description what this processor does.
         */
        virtual const std::string getDescription() const = 0;

        /**
         * Gets the name of the author of this processor. Can be handy if you have questions on how to do XYZ with this processor.
         * \return  The name of the author of this processor.
         */
        virtual const std::string getAuthor() const = 0;

        /**
         * Gets the current processor state in terms of stability. To be defined by every subclass.
         * \return  The current processor state in terms of stability.
         */
        virtual ProcessorState getProcessorState() const = 0;
        
        /**
         * Registers \a prop as property with the default invalidation level of INVALID_RESULT.
         * \see HasPropertyCollection::addProperty()
         * \param prop  Property to register
         */
        virtual void addProperty(AbstractProperty& prop);

        /**
         * Registers \a prop as property with the provided invalidation level. Registered properties 
         * can be accessed from the outside, e.g. via getProperty(), and will automatically invalidate
         * this processor on change. An already existing property with the same name will be replaced.
         *
         * \param   prop                Property to add
         * \param   invalidationLevel   Invalidation level of this property
         */
        void addProperty(AbstractProperty& prop, int invalidationLevel);

        /**
         * Sets the property invalidation level to the specified value.
         *
         * \param   prop                Property whose invalidation level is to change.
         * \param   invalidationLevel   New invalidation level of this property
         */
        void setPropertyInvalidationLevel(AbstractProperty& prop, int invalidationLevel);

        /**
         * Execute this processor.
         * Locks the processor and calls updateShader(), updateProperties() and/or updateResult() 
         * with respect to the current invalidation level.
         * 
         * \param   data                DataContainer to work on.
         * \param   unlockInExtraThread Flag whether the processor shall be unlockedin an extra thread (since unlock might be expensive).
         **/
        void process(DataContainer& data, bool unlockInExtraThread = false);

        /**
         * Gets the flag whether this processor is currently enabled.
         * \return _enabled
         */
        bool getEnabled() const;

        /**
         * Sets the flag whether this processor is currently enabled.
         * \param   enabled     New flag whether this processor is currently enabled.
         */
        void setEnabled(bool enabled);

        /**
         * Returns whether to measure the execution time of this processor.
         * \return  _clockExecutionTime
         */
        bool getClockExecutionTime() const;

        /**
         * Sets whether to measure the execution time of this processor.
         * \param   value   The new flag vlaue whether to measure the execution time of this processor.
         */
        void setClockExecutionTime(bool value);

        /**
         * Returns the current lockProcessor status of this processor.
         * If a processor is locked, all of its properties are locked and its process method must not be called.
         * \return  _locked != 0
         */
        bool isLocked();

// = Invalidation Level related stuff =============================================================

        /**
         * Returns the current invalidation level.
         * \return _level
         */
        int getInvalidationLevel() const {
            return _level;
        }

        /**
         * Returns whether the invalidation level is valid (i.e. no invalid flag is set).
         * \return _level == VALID
         */
        bool isValid() const {
            return _level == static_cast<int>(VALID);
        }

        /**
         * Returns whether the the INVALID_RESULT flag is set.
         * \return _level & INVALID_RESULT
         */
        bool hasInvalidResult() const {
            return (_level & static_cast<int>(INVALID_RESULT)) != 0;
        }

        /**
         * Returns whether the the INVALID_SHADER flag is set.
         * \return _level & INVALID_SHADER
         */
        bool hasInvalidShader() const {
            return (_level & static_cast<int>(INVALID_SHADER)) != 0;
        }

        /**
         * Returns whether the the INVALID_PROPERTIES flag is set.
         * \return _level & INVALID_PROPERTIES
         */
        bool hasInvalidProperties() const {
            return (_level & static_cast<int>(INVALID_PROPERTIES)) != 0;
        }

        /**
         * Sets the invalidation level to valid (i.e. clears all invalidation flags).
         */
        void setValid() {
            _level = static_cast<int>(VALID);
        }

        /**
         * Sets all invalidation flags specified in \a level.
         * \param   level   Flags to set to invalid.
         */
        void invalidate(int level);

        /**
         * Sets all invalidation flags specified in \a il's level.
         * \param   il  Flags to set to invalid.
         */
        void invalidate(InvalidationLevel il) {
            invalidate(static_cast<int>(il));
        }

        /**
         * Clears all invalidation flags specified in \a level.
         * \param   level   Flags to set to valid.
         */
        void validate(int level);

        /**
         * Clears all invalidation flags specified in \a il's level.
         * \param   il  Flags to set to valid.
         */
        void validate(InvalidationLevel il) {
            validate(static_cast<int>(il));
        }

        /// Signal emitted when the processor has been invalidated.
        sigslot::signal1<AbstractProcessor*> s_invalidated;

        /// Signal emitted when the processor has been validated.
        sigslot::signal1<AbstractProcessor*> s_validated;

    protected:
        /**
         * Gets called from default process() method when having an invalidation level of INVALID_SHADER.
         * 
         * Override this method for your needs, for instance if you need to recompile your shaders.
         * The default implementation only validates the INVALID_SHADER level again.
         */
        virtual void updateShader();

        /**
         * Gets called from default process() method when having an invalidation level of INVALID_PROPERTIES.
         * 
         * Override this method for your needs, for instance if you need to adjust your properties
         * to incoming data or other properties' settings. The default implementation only 
         * validates the INVALID_PROPERTIES level again.
         * 
         * \param   dc  DataContainer   The DataContainer of the calling pipeline.
         */
        virtual void updateProperties(DataContainer& dataContainer);

        /**
         * Implement this method to your needs to compute the result/output of your processor.
         * This method is considered to contain the actual algorithm each processor realizes. It 
         * gets called from default process() method when having an invalidation level of 
         * INVALID_RESULT.
         * 
         * \note    The default implementation only validates the INVALID_SHADER level again.
         * \param   dataContainer   The DataContainer to work on.
         */
        virtual void updateResult(DataContainer& dataContainer) = 0;
                
        /**
         * 
         * Locks all properties in the processor's PropertyCollection and marks them as "in use".
         * \sa  AbstractProcessor::unlockProcessor
         */
        void lockProcessor();

        /**
         * Unlocks all properties in the processor's PropertyCollection and marks them as "not in use".
         * \sa  AbstractProcessor::lockProcessor
         */
        void unlockProcessor();

// = Slots ========================================================================================

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        tbb::atomic<bool> _enabled;                 ///< flag whether this processor is currently enabled
        tbb::atomic<bool> _clockExecutionTime;      ///< flag whether to measure the execution time of this processor

        /// Flag whether this processor is currently locked
        /// (This implies, that all properties are locked and it is not valid to call process())
        tbb::atomic<bool> _locked;

        tbb::spin_rw_mutex _mtxInvalidationMap;     ///< Mutex protecting _invalidationMap
        /// Hash map storing the invalidation levels for each registered property
        std::unordered_map<const AbstractProperty*, int> _invalidationMap;

    private:
        tbb::atomic<int> _level;            ///< current invalidation level
        tbb::concurrent_queue<int> _queuedInvalidations;

        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
