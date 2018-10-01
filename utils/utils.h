#ifndef UTILS
#define UTILS

namespace Utils
{
    template<typename T> void safe_delete(T*& a) {
        delete a;
        a = NULL;
    }
}

#endif // UTILS
