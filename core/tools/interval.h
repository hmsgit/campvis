#ifndef INTERVAL_H__
#define INTERVAL_H__

#include "tgt/tgt_math.h"
#include "tgt/vector.h"
#include <limits>

namespace campvis {

    /**
     * Templated class for handling intervals of numeric types.
     * Offers support for common interval arithmetic.
     * \tparam  T   Numeric type of the interval.
     * \todo    Thread-safety - use atomics!
     */
    template<typename T>
    class Interval {
    public:
        /**
         * Default constructor, creates the empty interval (0,0).
         */
        Interval();

        /**
         * Copy constructor.
         * \param   src     Source interval
         */
        Interval(const Interval<T>& src);

        /**
         * Constructor, creates a new Interval<T>.
         * \param   left        Left interval border
         * \param   right       Right interval border
         * \param   leftOpen    Flag whether left interval border is open, defaults to false
         * \param   rightOpen   Flag whether right interval border is open, defaults to false
         */
        Interval(T left, T right, bool leftOpen=false, bool rightOpen=false);

        /**
         * Creates a new closed Interval<T> with the given vec2 as values
         * \param   vec     Vector with interval borders
         */
        explicit Interval(const tgt::Vector2<T>& vec);

        /**
         * Destructor
         */
        virtual ~Interval();


        /**
         * Returns left interval border.
         * \return  _left
         */
        inline const T& getLeft() const;

        /**
         * Returns right interval border.
         * \return  _right
         */
        inline const T& getRight() const;

        /**
         * Returns whether left interval border is open.
         * \return  _leftOpen
         */
        inline bool getLeftOpen() const;

        /**
         * Returns whether right interval border is open.
         * \return  _rightOpen
         */
        inline bool getRightOpen() const;


        /**
         * Returns whether the interval is empty.
         * \return  True if the interval is empty.
         */
        bool empty() const;

        /**
         * Returns the size of the interval 
         * \return  (_right - _left)
         */
        T size() const;


        /**
         * Checks whether the interval contains the value \a value.
         * \param   value   Value to check.
         * \return  True if \a value is inside this interval.
         */
        bool contains(const T& value) const;

        /**
         * Checks whether the interval contains the entire interval \a interval.
         * \param   value   Interval to check.
         * \return  True if \a interval is entirely inside this interval.
         */
        bool contains(const Interval<T>& interval) const;


        /**
         * Intersects this interval with \a rhs.
         * \param   rhs The other interval
         */
        void intersectWith(const Interval<T>& rhs);

        /**
         * Builds the union of this interval and \a rhs.
         * \param   rhs The other interval
         */
        void unionWith(const Interval<T>& rhs);

        /**
         * Sets all infinite, NaN or std::numeric_limits<T>::max() values to according values in rhs
         * \param   rhs The other interval
         */
        void clampInfinitesTo(const Interval<T>& rhs);


        /**
         * Enlarges the current interval by factor \a factor.
         * \param   factor  Enlargement factor
         */
        void enlarge(const T& factor);

        /**
         * Enlarges current interval as much as necessary until it contains \a value.
         * \param   value   Value the interval shall contain
         */
        void nibble(const T& value);


        /**
         * Clamps \a value so that it fits into this interval.
         * \param   value   Value to clamp.
         * \return  \a value clamped to this interval.
         */
        T clampValue(const T& value) const;


        /**
         * Returns a string representation of this interval.
         * \return  (_left, _right) or [_left, _right]
         */
        std::string toString() const;

        
        /**
         * Returns an interval whose borders are incremented by \a rhs.
         * \note    The interval size does not change!
         * \param   rhs The amount to add to the borders
         */
        Interval<T> operator+(const T& rhs) const;

        /**
         * Returns an interval whose borders are decremented by \a rhs.
         * \note    The interval size does not change!
         * \param   rhs The amount to substract from the borders
         */
        Interval<T> operator-(const T& rhs) const;

        /**
         * Returns an interval whose borders are multiplied by \a rhs.
         * \param   rhs The factor to multiply to the borders
         */
        Interval<T> operator*(const T& rhs) const;

        /**
         * Returns an interval whose borders are divided by \a rhs.
         * \param   rhs The divider to apply to the borders
         */
        Interval<T> operator/(const T& rhs) const;


        /**
         * Returns whether this interval is equal to \a rhs.
         * \param   rhs Interval to compare
         * \return  True if borders and open flags match.
         */
        bool operator==(const Interval<T>& rhs) const;

        /**
         * Returns whether this interval is not equal to \a rhs.
         * \param   rhs Interval to compare
         * \return  True if at least one of the borders or open flags does not match.
         */
        bool operator!=(const Interval<T>& rhs) const;

    private:
        T _left;                ///< left interval border
        T _right;               ///< right interval border

        bool _leftOpen;         ///< flag whether left interval border is open
        bool _rightOpen;        ///< flag whether right interval border is open

    };


    // ---------------------------------------- implementation ----------------------------------------

    template<typename T>
    Interval<T>::Interval()
        : _left(0)
        , _right(0)
        , _leftOpen(true)
        , _rightOpen(true)
    {}

    template<typename T>
    Interval<T>::Interval(const Interval<T>& src)
        : _left(src._left)
        , _right(src._right)
        , _leftOpen(src._leftOpen)
        , _rightOpen(src._rightOpen)
    {}


    template<typename T>
    Interval<T>::Interval(T left, T right, bool leftOpen, bool rightOpen)
        : _left(left)
        , _right(right)
        , _leftOpen(leftOpen)
        , _rightOpen(rightOpen)
    {}

    template<typename T>
    Interval<T>::Interval(const tgt::Vector2<T>& vec)
        : _left(vec.x)
        , _right(vec.y)
        , _leftOpen(false)
        , _rightOpen(false)
    {
        if (_left > _right)
            std::swap(_left, _right);
    }

    template<typename T>
    Interval<T>::~Interval() {
    }

    template<typename T>
    const T& Interval<T>::getLeft() const {
        return _left;
    }

    template<typename T>
    const T& Interval<T>::getRight() const {
        return _right;
    }

    template<typename T>
    bool Interval<T>::getLeftOpen() const {
        return _leftOpen;
    }

    template<typename T>
    bool Interval<T>::getRightOpen() const {
        return _rightOpen;
    }

    template<typename T>
    T Interval<T>::size() const {
        return _right - _left;
    }

    template<typename T>
    bool Interval<T>::empty() const {
        if (_left > _right)
            return true;
        if (_left == _right)
            return (_leftOpen || _rightOpen);
        return false;
    }

    template<typename T>
    bool Interval<T>::contains(const T& value) const {
        if (tgt::isNaN(value))
            return false;
        if ((value < _left) || (_leftOpen && value == _left))
            return false;
        if ((value > _right) || (_rightOpen && value == _right))
            return false;
        return true;
    }

    template<typename T>
    bool Interval<T>::contains(const Interval<T>& rhs) const {
        if ((_left > rhs._left) || ((_leftOpen || rhs._leftOpen) && _left == rhs._left))
            return false;
        if ((_right < rhs._right) || ((_rightOpen || rhs._rightOpen) && _right == rhs._right))
            return false;
        return true;
    }

    template<typename T>
    void Interval<T>::intersectWith(const Interval<T>& rhs) {
        if (rhs.empty())
            return;
        if (empty()) {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
            return;
        }

        if (_left < rhs._left) {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
        }
        else if (_left == rhs._left) {
            _leftOpen = _leftOpen || rhs._leftOpen;
        }
        else {
            // do nothing, keep values
        }

        if (_right > rhs._right) {
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
        }
        else if (_right == rhs._right) {
            _rightOpen = _rightOpen || rhs._rightOpen;
        }
        else {
            // do nothing, keep values
        }
    }

    template<typename T>
    void Interval<T>::unionWith(const Interval<T>& rhs) {
        // check if one of the intervals is empty
        if (empty()) {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
            return;
        }
        if (rhs.empty())
            return;

        if (_left < rhs._left) {
            // do nothing, keep values
        }
        else if (_left == rhs._left) {
            _leftOpen = _leftOpen && rhs._leftOpen;
        }
        else {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
        }

        if (_right > rhs._right) {
            // do nothing, keep values
        }
        else if (_right == rhs._right) {
            _rightOpen = _rightOpen && rhs._rightOpen;
        }
        else {
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
        }
    }

    template<typename T>
    void Interval<T>::clampInfinitesTo(const Interval<T>& rhs) {
        if (rhs.empty())
            return;
        if (empty()) {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
            return;
        }

        if (tgt::isNaN(_left) || _left == std::numeric_limits<T>::infinity() || _left == -std::numeric_limits<T>::max()) {
            _left = rhs._left;
            _leftOpen = rhs._leftOpen;
        }
        if (tgt::isNaN(_right) || _right == std::numeric_limits<T>::infinity() || _right == std::numeric_limits<T>::max()) {
            _right = rhs._right;
            _rightOpen = rhs._rightOpen;
        }
    }

    template<typename T>
    Interval<T> Interval<T>::operator+(const T& rhs) const {
        return Interval<T>(_left+rhs, _right+rhs, _leftOpen, _rightOpen);
    }

    template<typename T>
    Interval<T> Interval<T>::operator-(const T& rhs) const {
        return Interval<T>(_left-rhs, _right-rhs, _leftOpen, _rightOpen);
    }

    template<typename T>
    Interval<T> Interval<T>::operator*(const T& rhs) const {
        return Interval<T>(_left*rhs, _right*rhs, _leftOpen, _rightOpen);
    }

    template<typename T>
    Interval<T> Interval<T>::operator/(const T& rhs) const {
        return Interval<T>(_left/rhs, _right/rhs, _leftOpen, _rightOpen);
    }

    template<typename T>
    void Interval<T>::enlarge(const T& factor) {
        T length = _right-_left;
        T offset = length*(factor-1.0)/2.0;
        _left -= offset;
        _right += offset;
    }

    template<typename T>
    void Interval<T>::nibble(const T& value) {
        if (empty()) {
            _left = value;
            _right = value;
            _leftOpen = false;
            _rightOpen = false;
        }
        else if (_leftOpen && _left == value)
            _leftOpen = false;
        else if (_rightOpen && _right == value)
            _rightOpen = false;
        else if (_left > value) {
            _left = value;
            _leftOpen = false;
        }
        else if (_right < value) {
            _right = value;
            _rightOpen = false;
        }
    }

    template<typename T>
    T Interval<T>::clampValue(const T& value) const {
        if (value < _left)
            return _left;
        else if (value > _right)
            return _right;
        else // contains(value)
            return value;
    }

    template<typename T>
    std::string Interval<T>::toString() const {
        std::stringstream result;
        result << (_leftOpen ? "(" : "[") << _left << ", " << _right << (_rightOpen ? ")" : "]");
        return result.str();
    }

    template<typename T>
    bool Interval<T>::operator==(const Interval<T>& rhs) const {
        return (_left == rhs._left
             && _right == rhs._right
             && _leftOpen == rhs._leftOpen
             && _rightOpen == rhs._rightOpen);
    }

    template<typename T>
    bool Interval<T>::operator!=(const Interval<T>& rhs) const {
        return (_left != rhs._left
             || _right != rhs._right
             || _leftOpen != rhs._leftOpen
             || _rightOpen != rhs._rightOpen);
    }

}

#endif // INTERVAL_H__
