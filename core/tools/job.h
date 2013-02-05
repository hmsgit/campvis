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

#ifndef JOB_H__
#define JOB_H__

#include "sigslot/sigslot.h"
#include "tgt/assert.h"

namespace campvis {

    /**
     * Abstract base class for a job that can be executed.
     */
    class AbstractJob {
    public:
        /**
         * Pure virtual destructor.
         */
        virtual ~AbstractJob() = 0 {};

        /**
         * Executes the job, to be overwritten by subclasses.
         */
        virtual void execute() = 0;
    };

// = Specific Jobs ================================================================================

    /**
     * Specific job, that is calling a member function pasing no argument.
     */
    template<class T>
    class CallMemberFuncJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing no argument.
         * \param   target  Target object
         * \param   callee  Member function to call
         */
        CallMemberFuncJob(T* target, void (T::*callee)())
            : _target(target)
            , _callee(callee)
        {
            tgtAssert(_target != 0, "Target object must not be 0.");
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallMemberFuncJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (_target->*_callee)();
        }

    protected:
        T* _target;                 ///< Target object
        void (T::*_callee)();       /// <Member function to call
    };

    /**
     * Specific job, that is calling a member function pasing a single argument.
     */
    template<class T, class A1>
    class CallMemberFunc1ArgJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing \a arg1 as single argument.
         * \param   target  Target object
         * \param   callee  Member function to call
         * \param   arg1    Argument to pass to \a callee
         */
        CallMemberFunc1ArgJob(T* target, void (T::*callee)(A1), A1 arg1)
            : _target(target)
            , _callee(callee)
            , _arg1(arg1)
        {
            tgtAssert(_target != 0, "Target object must not be 0.");
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallMemberFunc1ArgJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (_target->*_callee)(_arg1);
        }

    protected:
        T* _target;                 ///< Target object
        void (T::*_callee)(A1);     /// <Member function to call
        A1 _arg1;                   ///< Argument to pass to \a callee
    };

// = Helper functions for easier creation of jobs =================================================

    /**
     * Creates a new CallMemberFuncJob for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \return  Pointer to the newly created CallMemberFuncJob. Caller has ownership!
     */
    template<class T>
    CallMemberFuncJob<T>* make_job(T* target, void (T::*callee)()) {
        return new CallMemberFuncJob<T>(target, callee);
    }

    /**
     * Creates a new CallMemberFunc1ArgJob for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  Pointer to the newly created CallMemberFunc1ArgJob. Caller has ownership!
     */
    template<class T, class A1>
    CallMemberFunc1ArgJob<T, A1>* make_job(T* target, void (T::*callee)(A1), A1 arg1) {
        return new CallMemberFunc1ArgJob<T, A1>(target, callee, arg1);
    }

}

#endif // JOB_H__