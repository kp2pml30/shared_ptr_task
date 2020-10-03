#pragma once

#include <utility>

template<typename T>
class weak_ptr;
template<typename T>
class shared_ptr;
template<typename T, typename ...A>
shared_ptr<T> make_shared(A &&...);

class control_block
{
private:
    std::size_t ref = 1;
    std::size_t wref = 0;

    template<typename Y>
    friend class weak_ptr;
    template<typename Y>
    friend class shared_ptr;
protected:
    void inc(void) noexcept { ref++; }
    void dec(void)
    {
        ref--;
        if (ref != 0)
            return;
        destroy();
        if (wref == 0)
            delete this;
    }
    std::size_t use_count(void) const noexcept { return ref; }

    void winc(void) noexcept { wref++; }
    void wdec(void) { if (--wref == 0 && ref == 0) delete this; }

    virtual ~control_block(void) {}
public:
    virtual void destroy(void) = 0;
};

template<typename T>
class default_control_block : public control_block
{
protected:
    T *obj;

    void destroy(void) override { delete obj; }
public:
    default_control_block(T *obj = nullptr) noexcept : obj(obj) {}
};


template<typename T, typename D>
class regular_control_block : public default_control_block<T>, private D
{
public:
    // move constructor of D should not throw
    regular_control_block(T *ptr, D deleter) noexcept : default_control_block<T>(ptr), D(std::move(deleter)) {}
    void destroy(void) override { if (default_control_block<T>::obj != nullptr) D::operator()(default_control_block<T>::obj); }
};

template<typename T>
class inplace_control_block : public control_block
{
private:
    template<typename Y, typename ...A>
    friend shared_ptr<Y> make_shared(A &&...);

    std::aligned_storage_t<sizeof(T), alignof(T)> obj;

    T * get_obj(void) noexcept { return reinterpret_cast<T *>(&obj); }
protected:
    void destroy(void) override { get_obj()->~T(); }
public:
    template<typename ...A>
    explicit inplace_control_block(A ...args) { new(&obj) T(std::forward<A>(args)...); }
};

template<typename T, typename ...A>
shared_ptr<T> make_shared(A &&...args);

template<typename T>
class shared_ptr
{
private:
    control_block *block;
    T *ptr;

    template<typename Y>
    friend class shared_ptr;
    template<typename Y, typename ...A>
    friend shared_ptr<Y> make_shared(A &&...);

    void dec(void) { if (block != nullptr) block->dec(); }

    shared_ptr(control_block *b, T *p) noexcept : block(b), ptr(p) {}
    template<typename Y>
    friend class weak_ptr;
public:
    shared_ptr(void) noexcept : block(nullptr), ptr(nullptr) {}
    shared_ptr(std::nullptr_t) noexcept : shared_ptr() {}
    template<typename Y>
    explicit shared_ptr(Y *ptr) noexcept : block(new default_control_block(ptr)), ptr(ptr) {}
    template<typename Y>
    shared_ptr(const shared_ptr<Y> &s, T *al) noexcept : block(s.block), ptr(al) { if (block != nullptr) block->inc(); }
    template<typename Y, typename D>
    shared_ptr(Y *ptr, D deleter) try : block(new regular_control_block<T, D>(ptr, std::move(deleter))), ptr(ptr)
    {}
    catch (...)
    {
        deleter(ptr);
        throw;
    }
    template<typename Y>
    shared_ptr(const shared_ptr<Y> &r) noexcept : block(r.block), ptr(r.ptr) { if (block != nullptr) block->inc(); }
    // idk how to remove this one
    shared_ptr(const shared_ptr &r) noexcept : block(r.block), ptr(r.ptr) { if (block != nullptr) block->inc(); }
    template<typename Y>
    shared_ptr(shared_ptr<Y> &&r) noexcept : block(r.block), ptr(r.ptr) { r.block = nullptr; r.ptr = nullptr; }
    shared_ptr(shared_ptr &&r) noexcept : block(r.block), ptr(r.ptr) { r.block = nullptr; r.ptr = nullptr; }

    template<typename Y>
    shared_ptr & assign(shared_ptr<Y> &&r)
    {
        if (block == r.block)
        {
            ptr = r.ptr;
            return *this;
        }
        dec();
        block = r.block;
        ptr = r.ptr;
        r.block = nullptr;
        r.ptr = nullptr;
        return *this;
    }

    template<typename Y>
    shared_ptr & assign(const shared_ptr<Y> &r)
    {
        auto old = block;
        block = r.block;
        if (block != nullptr)
            block->inc();
        ptr = r.ptr;
        if (old != nullptr)
            old->dec();
        return *this;
    }

    template<typename Y>
    shared_ptr & operator=(shared_ptr<Y> &&r) { return assign(std::forward<shared_ptr<Y> &&>(r)); }
    shared_ptr & operator=(shared_ptr &&r) { return assign(std::forward<shared_ptr>(r)); }
    template<typename Y>
    shared_ptr & operator=(const shared_ptr<Y> &r) { return assign(r); }
    shared_ptr & operator=(const shared_ptr &r) { return assign(r); }

    ~shared_ptr(void)
    {
        dec();
    }

    T & operator*(void) const noexcept { return *ptr; }
    T * get(void) const noexcept { return ptr; }
    T * operator->(void) const noexcept { return get(); }
    operator bool(void) const noexcept { return !!ptr; }
    std::size_t use_count(void) const noexcept
    {
        if (block == nullptr)
            return 0;
        return block->use_count();
    }
    void reset(void)
    {
        dec();
        block = nullptr;
        ptr = nullptr;
    }
    template<typename Y>
    void reset(Y *r)
    {
        dec();
        block = new default_control_block<Y>(r);
        ptr = r;
    }
    template<typename Y, typename D>
    void reset(Y *r, D deleter)
    {
        dec();
        try
        {
            block = nullptr;
            ptr = nullptr;
            block = new regular_control_block<Y, D>(r, std::move(deleter));
            ptr = r;
        }
        catch (...)
        {
            deleter(r);
            throw;
        }
    }
};


template<typename T, typename Y>
bool operator==(const shared_ptr<T> &t, const Y *y) noexcept { return t.get() == y; }
template<typename T, typename Y>
bool operator==(const Y *y, const shared_ptr<T> &t) noexcept { return t.get() == y; }
template<typename T, typename Y>
bool operator==(const shared_ptr<T> &t, const shared_ptr<Y> &y) noexcept { return t.get() == y.get(); }
template<typename T>
bool operator==(const shared_ptr<T> &t, std::nullptr_t) noexcept { return t.get() == nullptr; }
template<typename T>
bool operator==(std::nullptr_t, const shared_ptr<T> &t) noexcept { return t.get() == nullptr; }

template<typename T, typename Y>
bool operator!=(const shared_ptr<T> &t, const Y *y) noexcept { return !(t == y); }
template<typename T, typename Y>
bool operator!=(const Y *y, const shared_ptr<T> &t) noexcept { return !(t == y); }
template<typename T, typename Y>
bool operator!=(const shared_ptr<T> &t, const shared_ptr<Y> &y) noexcept { return !(t == y); }
template<typename T>
bool operator!=(const shared_ptr<T> &t, std::nullptr_t) noexcept { return !(t == nullptr); }
template<typename T>
bool operator!=(std::nullptr_t, const shared_ptr<T> &t) noexcept { return !(t == nullptr); }

template<typename T, typename ...A>
shared_ptr<T> make_shared(A &&...args)
{
    auto *cb = new inplace_control_block<T>(std::forward<A>(args)...);
    return shared_ptr<T>(static_cast<control_block *>(cb), cb->get_obj());
}

template<typename T>
class weak_ptr
{
private:
    control_block *block;
    T *ptr;

    void wdec(void) { if (block != nullptr) block->wdec(); }
public:
    weak_ptr(void) noexcept : block(nullptr), ptr(nullptr) {}
    weak_ptr(std::nullptr_t) noexcept : weak_ptr() {}
    template<typename Y>
    weak_ptr(const shared_ptr<Y> &t)
    {
        block = t.block;
        ptr = t.ptr;
        if (block != nullptr)
            block->winc();
    }
    template<typename Y>
    weak_ptr(const weak_ptr<Y> &r) noexcept : block(r.block), ptr(r.ptr) { if (block != nullptr) block->winc(); }
    // idk how to remove this one
    weak_ptr(const weak_ptr &r) noexcept : block(r.block), ptr(r.ptr) { if (block != nullptr) block->winc(); }
    template<typename Y>
    weak_ptr(weak_ptr<Y> &&r) noexcept : block(r.block), ptr(r.ptr) { r.block = nullptr; r.ptr = nullptr; }
    weak_ptr(weak_ptr &&r) noexcept : block(r.block), ptr(r.ptr) { r.block = nullptr; r.ptr = nullptr; }

    template<typename Y>
    weak_ptr & assign(weak_ptr<Y> &&r)
    {
        if (block == r.block)
        {
            ptr = r.ptr;
            return *this;
        }
        wdec();
        block = r.block;
        ptr = r.ptr;
        r.block = nullptr;
        r.ptr = nullptr;
        return *this;
    }

    template<typename Y>
    weak_ptr & assign(const weak_ptr<Y> &r)
    {
        auto old = block;
        block = r.block;
        if (block != nullptr)
            block->winc();
        ptr = r.ptr;
        if (old != nullptr)
            old->wdec();
        return *this;
    }

    template<typename Y>
    weak_ptr & operator=(weak_ptr<Y> &&r) { return assign(std::forward<weak_ptr<Y> &&>(r)); }
    weak_ptr & operator=(weak_ptr &&r) { return assign(std::forward<weak_ptr>(r)); }
    template<typename Y>
    weak_ptr & operator=(const weak_ptr<Y> &r) { return assign(r); }
    weak_ptr & operator=(const weak_ptr &r) { return assign(r); }

    ~weak_ptr(void)
    {
        if (block != nullptr)
            block->wdec();
    }

    shared_ptr<T> lock(void) const noexcept
    {
        if (block == nullptr || block->use_count() == 0)
            return shared_ptr<T>(nullptr);
        block->inc();
        return shared_ptr<T>(block, ptr);
    }
};