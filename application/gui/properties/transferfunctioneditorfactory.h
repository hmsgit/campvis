// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef TRANSFERFUNCTIONEDITORFACTORY_H__
#define TRANSFERFUNCTIONEDITORFACTORY_H__

namespace campvis {
    class AbstractTransferFunction;
    class AbstractTransferFunctionEditor;

    /**
     * Factory class offering the static method createEditor to create transfer function editors
     * for a given transfer function (depending on its type).
     */
    class TransferFunctionEditorFactory {
    public:
        /**
         * Creates the corresponding TransferFunctionEditor for the given transfer function \a tf.
         * \note    The callee has to take the ownership of the returned pointer.
         * \param   tf  Transfer function to generate the editor for.
         * \return  A new transfer function editor for the given transfer function (depending on its type).
         */
        static AbstractTransferFunctionEditor* createEditor(AbstractTransferFunction* tf);

    private:
        /// Shall not instantiate
        TransferFunctionEditorFactory();
    };
}

#endif // TRANSFERFUNCTIONEDITORFACTORY_H__
