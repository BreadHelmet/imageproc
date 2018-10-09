#ifndef UTILS
#define UTILS

namespace Utils
{
    template<typename T> void safe_delete(T*& a);
    template<typename T> void max(T*& a, T*& b);
    template <typename T> int sgn(T val);
}

#endif // UTILS
