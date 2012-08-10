// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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

#ifndef INVALIDATIONLEVEL_H__
#define INVALIDATIONLEVEL_H__

namespace TUMVis {

    /**
     * Wrapper class for enum-int-based invalidation levels
     * Invalidation Level will internally be stored in an integer, the named levels define the bit mask.
     * All Accessor methods correctly apply the bitmasks.
     */
    struct InvalidationLevel {
    public:
        /**
         * Available invalidation levels
         */
        enum NamedLevels {
            VALID               = 0,
            INVALID_RESULT      = 1 << 0,
            INVALID_SHADER      = 1 << 1,
        };

        /**
         * Creates a new invalidation level.
         * \param defaultLevel  Initial invalidation level
         */
        InvalidationLevel(int defaultLevel = VALID) {
            _level = static_cast<int>(defaultLevel);
        }

        /**
         * Returns the invalidation level
         * \return _level
         */
        int getLevel() const {
            return _level;
        }

        /**
         * Returns wheter the invalidation level is valid.
         * \return _level == VALID
         */
        bool isValid() const {
            return _level == static_cast<int>(VALID);
        }

        bool isInvalidResult() const {
            return (_level & static_cast<int>(INVALID_RESULT)) != 0;
        }

        bool isInvalidShader() const {
            return (_level & static_cast<int>(INVALID_SHADER)) != 0;
        }


        void setValid() {
            _level = static_cast<int>(VALID);
        }

        /**
         * Update the invalidation level by \a level.
         * If \a level is VALID, the invalidation level will be set to VALID.
         * If \a level is one of the INVALID_X state, the corresponding flag will be set.
         * \param level     Invalidation level to apply.
         */
        void setLevel(int level) {
            if (level == static_cast<int>(VALID)) {
                _level = level;
            }
            else {
                _level |= static_cast<int>(level);
            }        
        }

        void setLevel(NamedLevels nl) {
            setLevel(static_cast<int>(nl));
        }

        void setLevel(InvalidationLevel il) {
            setLevel(il._level);
        }

    private:
        int _level;
    };

}

#endif // INVALIDATIONLEVEL_H__
