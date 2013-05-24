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

#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include "sigslot/sigslot.h"
#include "tbb/atomic.h"
#include "tbb/concurrent_queue.h"
#include "tgt/logmanager.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/propertycollection.h"

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
    class AbstractProcessor : public HasPropertyCollection {
    public:
        /**
         * Available invalidation levels
         */
        enum InvalidationLevel {
            VALID               = 0,        ///< Valid
            INVALID_RESULT      = 1 << 0,   ///< Need to rerun the process() method
            INVALID_SHADER      = 1 << 1,   ///< Need to recompile the shader
            INVALID_FILE        = 1 << 2,   ///< Need to reread the file
            INVALID_PROPERTIES  = 1 << 3    ///< Need to update the properties
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
         * Execute this processor.
         * \param data      DataContainer to work on.
         **/
        virtual void process(DataContainer& data) = 0;

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
         * Returns wheter the invalidation level is valid (i.e. no invalid flag is set).
         * \return _level == VALID
         */
        bool isValid() const {
            return _level == static_cast<int>(VALID);
        }

        /**
         * Returns wheter the the INVALID_RESULT flag is set.
         * \return _level & INVALID_RESULT
         */
        bool hasInvalidResult() const {
            return (_level & static_cast<int>(INVALID_RESULT)) != 0;
        }

        /**
         * Returns wheter the the INVALID_SHADER flag is set.
         * \return _level & INVALID_SHADER
         */
        bool hasInvalidShader() const {
            return (_level & static_cast<int>(INVALID_SHADER)) != 0;
        }

        /**
         * Returns wheter the the INVALID_FILE flag is set.
         * \return _level & INVALID_FILE
         */
        bool hasInvalidFile() const {
            return (_level & static_cast<int>(INVALID_FILE)) != 0;
        }

        /**
         * Returns wheter the the INVALID_PROPERTIES flag is set.
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

// = Slots ========================================================================================

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

    protected:
        tbb::atomic<bool> _enabled;                 ///< flag whether this processor is currently enabled
        tbb::atomic<bool> _clockExecutionTime;      ///< flag whether to measure the execution time of this processor

        /// Flag whether this processor is currently locked
        /// (This implies, that all properties are locked and it is not valid to call process())
        tbb::atomic<bool> _locked;

    private:
        tbb::atomic<int> _level;            ///< current invalidation level
        tbb::concurrent_queue<int> _queuedInvalidations;

        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
