#ifndef UNDERSCORE_H
#define UNDERSCORE_H

#include <algorithm>

namespace _ {

namespace util {

// simply check if has mapped_type
template<typename T>
class IsMappedContainer {
private:
    template<typename U>
    static typename U::mapped_type check(int);

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(0))>::value;
};

template<typename T>
class HasPushBack {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->push_back(typename U::value_type()), int());

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T>
class HasInsert {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->insert(typename U::value_type()), int());

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T>
class HasInsertAfter {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->insert_after(p->begin(), typename U::value_type()), int()); // begin is just for checking

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T>
typename std::enable_if<HasPushBack<T>::value, void>::type 
add(T& c, const typename T::value_type& v) {
    c.push_back(v);
}

template<typename T>
typename std::enable_if<HasInsert<T>::value, void>::type 
add(T& c, const typename T::value_type& v) {
    c.insert(v);
}

template<typename T>
typename std::enable_if<HasInsertAfter<T>::value, void>::type 
add(T& c, const typename T::value_type& v) {
    // get to the end of the list, which is O(N) and not fast at all
    auto before_end = c.before_begin();
    for (auto& _ : c) {
        ++before_end;
    }
    c.insert_after(before_end, v);
}

} // namespace util


template<typename Collection, typename Function>
typename std::enable_if<!util::IsMappedContainer<Collection>::value, void>::type
each(Collection& obj, Function iterator) {
    std::for_each(std::begin(obj), std::end(obj), iterator);
}

template<typename Collection, typename Function>
typename std::enable_if<util::IsMappedContainer<Collection>::value, void>::type
each(Collection& obj, Function iterator) {
    std::for_each(std::begin(obj), std::end(obj), [&](typename Collection::value_type& v) {
        iterator(v.second, v.first);
    });
}


template<typename Collection, typename Function>
typename std::enable_if<!util::IsMappedContainer<Collection>::value, bool>::type
all(const Collection& obj, Function iterator) {
    return std::all_of(std::begin(obj), std::end(obj), iterator);
}

template<typename Collection, typename Function>
typename std::enable_if<util::IsMappedContainer<Collection>::value, bool>::type
all(const Collection& obj, Function iterator) {
    return std::all_of(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        return iterator(v.second, v.first);
    });
}


template<typename Collection, typename Function>
typename std::enable_if<!util::IsMappedContainer<Collection>::value, bool>::type
any(const Collection& obj, Function iterator) {
    return std::any_of(std::begin(obj), std::end(obj), iterator);
}

template<typename Collection, typename Function>
typename std::enable_if<util::IsMappedContainer<Collection>::value, bool>::type
any(const Collection& obj, Function iterator) {
    return std::any_of(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        return iterator(v.second, v.first);
    });
}


template<typename Collection, typename Function>
typename std::enable_if<!util::IsMappedContainer<Collection>::value, Collection>::type
filter(const Collection& obj, Function iterator) {
    Collection result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        if (iterator(v)) {
            util::add(result, v);
        }
    });
    return result;
}

template<typename Collection, typename Function>
typename std::enable_if<util::IsMappedContainer<Collection>::value, Collection>::type
filter(const Collection& obj, Function iterator) {
    Collection result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        if (iterator(v.second, v.first)) {
            util::add(result, v);
        }
    });
    return result;
}


template<template<class T, class Allocator = std::allocator<T>> 
         class RetCollection = std::vector, 
         typename Collection, 
         typename Function> auto
map(const Collection& obj, Function iterator) 
    -> typename std::enable_if<!util::IsMappedContainer<Collection>::value,
                               RetCollection<decltype(iterator(typename Collection::value_type()))>
                              >::type {

    using R = decltype(iterator(typename Collection::value_type()));
    RetCollection<R> result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        util::add(result, iterator(v));
    });
    return result;
}

template<template<class T, class Allocator = std::allocator<T>> 
         class RetCollection = std::vector, 
         typename Collection, 
         typename Function> auto
map(const Collection& obj, Function iterator) 
    -> typename std::enable_if<util::IsMappedContainer<Collection>::value,
                               RetCollection<decltype(iterator(typename Collection::mapped_type(), 
                                                               typename Collection::key_type()))>
                              >::type {

    using R = decltype(iterator(typename Collection::mapped_type(), typename Collection::key_type()));
    RetCollection<R> result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        util::add(result, iterator(v.second, v.first));
    });
    return result;
}


} // namespace _

#endif // UNDERSCORE_H

