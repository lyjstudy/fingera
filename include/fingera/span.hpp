#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>
#include <iterator>
#include <stdexcept>

namespace fingera {

template<typename T>
class span {
    static_assert(std::is_standard_layout<T>() && !std::is_class<T>() , "not allowed");
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // constructors
    inline constexpr span() noexcept
        : _ptr(nullptr), _len(0) {
    }
    inline constexpr span(std::nullptr_t) noexcept
        : _ptr(nullptr), _len(0) {
    }
    inline constexpr span(T *ptr, const size_t len) noexcept
        : _ptr(ptr), _len(len) {
    }
    template<size_t N>
    inline constexpr span(T (&src)[N]) noexcept
        : _ptr(src), _len(N) {
    }
    template<typename Container>
    inline constexpr span(Container &v) noexcept
        : _ptr(v.data()), _len(v.size()) {
    }
    inline constexpr span(const span &o) noexcept
        : _ptr(o._ptr), _len(o._len) {
    }

    inline constexpr span& operator=(const span &o) noexcept {
        _ptr = o._ptr; _len = o._len;
    }

    // iterators:
    inline constexpr iterator begin() noexcept {
        return _ptr;
    }
    inline constexpr const_iterator begin() const noexcept {
        return _ptr;
    }
    inline constexpr iterator end() noexcept {
        return _ptr + _len;
    }
    inline constexpr const_iterator end() const noexcept {
        return _ptr + _len;
    }
 
    inline constexpr reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(end());
    }
    inline constexpr const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(end());
    }
    inline constexpr reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(begin());
    }
    inline constexpr const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(begin());
    }
 
    inline constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    inline constexpr const_iterator cend() const noexcept {
        return end();
    }
    inline constexpr const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }
    inline constexpr const_reverse_iterator crend() const noexcept {
        return rend();
    }

    // capacity:
    inline constexpr size_type size() const noexcept {
        return _len;
    }
    inline constexpr size_type max_size() const noexcept {
        return _len;
    }
    inline constexpr bool empty() const noexcept {
        return _ptr == nullptr || _len == 0;
    }
 
    // element access:
    inline constexpr reference operator[](size_type n) {
        if (n >= _len) throw std::range_error("overflow");
        return _ptr[n];
    }
    inline constexpr const_reference operator[](size_type n) const {
        if (n >= _len) throw std::range_error("overflow");
        return _ptr[n];
    }
    inline constexpr const_reference at(size_type n) const {
        return (*this)[n];
    }
    inline constexpr reference at(size_type n) {
        return (*this)[n];
    }
    inline constexpr reference front() {
        return (*this)[0];
    }
    inline constexpr const_reference front() const {
        return (*this)[0];
    }
    inline constexpr reference back() {
        return (*this)[_len - 1];
    }
    inline constexpr const_reference back() const {
        return (*this)[_len - 1];
    }
 
    inline constexpr T * data() noexcept {
        return _ptr;
    }
    inline constexpr const T * data() const noexcept {
        return _ptr;
    }
private:
    T* _ptr;
    size_t _len;
};


} // namespace fingera
