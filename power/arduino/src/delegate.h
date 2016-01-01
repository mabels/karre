class Delegate {
public:
    typedef void (*Ptr)(void *);

    template <class T, void (T::*TMethod)(void)>
    static Ptr instance(T* object_ptr) {
        return static_cast<Ptr>(&method_stub<T, TMethod>);
    }
private:
    template <class T, void (T::*TMethod)(void)>
    static void method_stub(void* object_ptr) {
        T* p = static_cast<T*>(object_ptr);
        return (p->*TMethod)();
    }
};
