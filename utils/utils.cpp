
template<typename T> void Utils::safe_delete(T*& a) {
    delete a;
    a = NULL;
}
