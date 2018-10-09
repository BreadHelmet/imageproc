
#include "utils.h"

template<typename T> void Utils::safe_delete(T*& a) {
    delete a;
    a = NULL;
}

template<typename T> void Utils::max(T*& a, T*& b) {
    if(a > b) return a;
    return b;
}

/**
 * https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c#answer-4609795
 */
template <typename T> int Utils::sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}
