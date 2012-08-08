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
