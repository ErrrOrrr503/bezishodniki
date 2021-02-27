#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iterator>
#include <string.h>
#include <cmath>
#include <string>
#include <limits>

/*
    1 byte stores 1 decimal number. Due to portability uint_XXt only will be used for variables crucial to size
    Sign is stored in sign of mantissa[0]
*/

 //fixed:
typedef int8_t digit_t;
typedef uint8_t flags_t;
//adjustable:
typedef uint8_t numdigits_t;
typedef int8_t order_t;


///??? g_var for a class only - inheritance? // static var of class with static methods
///??? std::copy vs memcpy, что эффективней? 
///??? iterators inside class?

///fixme: constructor from std::string "<number>[e][E]<number>"

class bignum {
    //fixed
    static const numdigits_t _maxdigits_unlimited = 0;
    static const flags_t _flag_dynamic =   0b00000001;
    static const flags_t _flag_unlimited = 0b00000011;
    static const flags_t _flag_none =      0b00000000;
    static const flags_t _flag_nan      =  0b00001000;
    static const numdigits_t _precision = 10;
    //adjustable
    static numdigits_t _def_maxdigits;
    static flags_t _def_flags;

    numdigits_t numdigits; //current amount of meaningful digits
    numdigits_t maxdigits; //max amount of meaningful digits. 0 means unlimited
    digit_t *mantissa;     //meaningful digits itself: m[0],m[1]m[2]m[3]....*10^order
    order_t order;
    flags_t flags;

    void init_vars ()
    {
        numdigits = 0;
        order = 0;
        flags = _def_flags;
        if (flags & _flag_dynamic) {
            if (flags & _flag_unlimited)
                maxdigits = _maxdigits_unlimited;
            else
                maxdigits = _def_maxdigits;
            mantissa = nullptr; // mem will be allocated on write
        }
        else {
            maxdigits = _def_maxdigits;
            mantissa = new digit_t[maxdigits];
        }
    }

public:

    #ifdef DEBUG
    std::string debug_str;
    #endif

    bignum ()
    {
        // default constructor creates a bignum according to default values
        init_vars ();
        
        #ifdef DEBUG
        char tmp[25];
        sprintf (tmp, "[%p]", this);
        debug_str += tmp;
        debug_str += "::[default]";
        #endif
    }

    bignum (const bignum &other)
    {
        numdigits = other.numdigits;
        maxdigits = other.maxdigits;
        order = other.order;
        flags = other.flags;
        if ((flags & _flag_dynamic) && numdigits > 0)
            mantissa = new digit_t [numdigits];
        else
            mantissa = new digit_t [maxdigits];
        memcpy (mantissa, other.mantissa, numdigits);

        #ifdef DEBUG
        char tmp[25];
        sprintf (tmp, "[%p]", this);
        debug_str += tmp;
        debug_str += "::[copy]";
        debug_str += " <- " + other.debug_str;
        #endif
    }

    bignum (const double d)
    {
        init_vars ();

        #ifdef DEBUG
        char tmp[30];
        sprintf (tmp, "[%p]", this);
        debug_str += tmp;
        sprintf (tmp, "%4lg", d);
        debug_str += "::[double:" + std::string (tmp) + "]";
        #endif

        if (!(d == d)) { //nan
            flags |= _flag_nan;
            return;
        }
        double c = d;
        bool flag_negative = 0;
        if (c < 0) {
            flag_negative = 1;
            c = -c;
        }
        while (floor (c) > 9 && order < std::numeric_limits<order_t>::max ()) {
            c /= 10;
            order++;
        }
        while (floor (c) < 1 && order > std::numeric_limits<order_t>::min ()) {
            c *= 10;
            order--;
        }
        if (mantissa == nullptr) {
            // dyn mode
            mantissa = new digit_t [_precision];
        }
        bool flag_zero = 1;
        for (numdigits_t i = 0; i < _precision; i++) {
            mantissa[i] = floor (c);
            if (mantissa[i])
                flag_zero = 0;
            c = (c - floor (c)) * 10;
            numdigits++;
        }
        if (flag_zero) {
            order = 0;
            numdigits = 0;
        }
        if (flag_negative)
            mantissa[0] = -mantissa[0];
    }

    ~bignum ()
    {
        // default destructor
        if (mantissa != nullptr)
            delete[] mantissa;
    }

    digit_t& operator [](numdigits_t i)
    {
        return mantissa[i];
    }

    const digit_t& operator [](numdigits_t i) const
    {
        return mantissa[i];
    }

    bignum operator =(const bignum& A)
    {
        numdigits = A.numdigits;
        maxdigits = A.maxdigits;
        order = A.order;
        flags = A.flags;
        if ((flags & _flag_dynamic) && numdigits > 0)
            mantissa = new digit_t [numdigits];
        else
            mantissa = new digit_t [maxdigits];
        memcpy (mantissa, A.mantissa, numdigits);

        #ifdef DEBUG
        char tmp[25];
        sprintf (tmp, "[%p]", this);
        debug_str += tmp;
        debug_str += "::[operator=]";
        debug_str += " <- " + A.debug_str;
        #endif

        return *this;
    }

    numdigits_t get_numdigits () const
    {
        return numdigits;
    }

    numdigits_t get_maxdigits () const
    {
        return maxdigits;
    }

    order_t get_order () const
    {
        return order;
    }

    void set_order (order_t in_order)
    {
        order = in_order;
    }

    void set_numdigits (numdigits_t in_numdigits)
    {
        numdigits = in_numdigits;
    }

    bool isnan () const
    {
        return (flags & _flag_nan);
    }

    void set_flags (flags_t in_flags)
    {
        //fixme::check
        flags = in_flags;
    }
};

numdigits_t bignum::_def_maxdigits = 80;
flags_t bignum::_def_flags = _flag_none;

std::ostream& operator<< (std::ostream &out, const bignum &num)
{
    if (num.isnan ()) {
        std::cout << "nan";
        goto RETURN;
    }
    if (!num.get_numdigits ()) {
        std::cout << "0";
        goto RETURN;
    }
    out << (int) num[0] << ".";
	for (numdigits_t i = 1; i < num.get_numdigits (); i++) {
        out << (int) num[i];
    }
    out << "_E" << (int) num.get_order();

    RETURN:
    #ifdef DEBUG
    out << std::endl << "[numdigits = " << (int) num.get_numdigits () << "]" << std::endl << num.debug_str << std::endl;
    #endif
	return out;
}


int32_t min (int32_t A, int32_t B)
{
    if (A < B)
        return A;
    return B;
}

int32_t max (int32_t A, int32_t B)
{
    if (A > B)
        return A;
    return B;
}

bignum operator +(const bignum& in_A, const bignum& in_B)
{
    bignum C; //zero default
    //fixme: allocation for dynamic mode
    bignum A;
    bignum B;
    // to same order
    if (in_A.get_order () >= in_B.get_order ()) {
        A = in_A;
        B = in_B;
        }
    else {
        A = in_B;
        B = in_A;
    }
    if (A.get_order () != B.get_order ()) {
        order_t delta = A.get_order () - B.get_order ();
        numdigits_t i0 = min ((B.get_numdigits () + delta - 1), (B.get_maxdigits () - 1));
        for (numdigits_t i = i0; i >= delta; i--)
            B[i] = B[i - delta];
        for (numdigits_t i = 0; i < min (delta, B.get_maxdigits ()); i++)
            B[i] = 0;
        B.set_order (A.get_order ());
        B.set_numdigits (min (B.get_maxdigits (), B.get_numdigits () + delta));
        // IF THIS FUCKING SPAGET WORKS I WILL INSTALL MACOS
    }
    //mantissa sum
    C.set_order (A.get_order ());
    C.set_numdigits (max (A.get_numdigits (), B.get_numdigits ()));
    numdigits_t M = C.get_numdigits ();
    bool flag_nonzeros_started = 0;
    if (M == 0) //both are zero
        return C; //==zero
    if (A[0] * B[0] >= 0) { //equal signs
        std::cout << "  [DEBUG]::operator+equal_signs" << std::endl;
        for (numdigits_t i = M - 1; i > 0; i--) {
            C[i] += (A[i] + B[i]) % 10;
            C[i - 1] += (A[i] + B[i]) / 10;
            if (!flag_nonzeros_started && C[i] == 0)
                C.set_numdigits (C.get_numdigits () - 1); //if somw kind of 1.5 + 0.5 then decreas numdigits to 1
            if (C[i] > 0)
                flag_nonzeros_started = 1;
        }
        C[0] += (A[0] + B[0]);
        if (abs (C[0]) >= 10) {
            numdigits_t lim = C.get_numdigits () + 1;
            if (lim > C.get_maxdigits ())
                lim--; //fixme:realloc for unlimited well it is literally trash without reallocs
            for (numdigits_t i = lim - 1; i > 0; i--) {
                C[i] = C[i - 1] % 10;
            }
            C.set_numdigits (C.get_numdigits () + 1);
            C[0] = 1;
            C.set_order (C.get_order () + 1);
        } //sign is seems to be correct
    }
    else { // different signs
        std::cout << "  [DEBUG]::operator+different_signs" << std::endl;
        numdigits_t first_nonzero = M;
        for (numdigits_t i = M - 1; i > 0; i--) {
            if (A[i] - B[i] + C[i] >= 0)
                C[i] += A[i] - B[i];
            else {
                C[i] += 10 + A[i] - B[i];
                C[i-1]--;
            }
            if (C[i] != 0)
                first_nonzero = i;
            if (!flag_nonzeros_started && C[i] == 0)
                C.set_numdigits (C.get_numdigits () - 1); //if some kind of 1.5 - 0.5 then decreas numdigits to 1
            if (C[i] > 0)
                flag_nonzeros_started = 1;
        }
        C[0] += A[0] - B[0]; // over 10 is impossible due to different signs. However 0 is possible...
        if (first_nonzero == M)
            C.set_numdigits (0);
        else {
            if (C[0] == 0) {
                for (numdigits_t i = 0; i < C.get_numdigits () - first_nonzero; i++)
                    C[i] = C[i + first_nonzero];
                C.set_numdigits (C.get_numdigits () - first_nonzero);
            }
        }
    }

     #ifdef DEBUG
    char tmp[25];
    sprintf (tmp, "[%p]", &C);
    C.debug_str += tmp;
    C.debug_str += "::[operator+]";
    #endif

    return C;
}