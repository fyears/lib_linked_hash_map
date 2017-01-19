/*
 * linked_hash_map
 *
 * An attempt to implement the linked hash map data structure
 * in C++ 11 as a library,
 * which has similar semantics with std::unordered_map
 * but preserves the insertion orders of the key value pairs,
 * similar to Java's java.util.LinkedHashMap.
 *
 * This file is licensed under the MIT license.
 * Some codes are modified from those in LLVM's libc++.
 *
 */


#ifndef PPSTD_LINKED_HASH_MAP_H_
#define PPSTD_LINKED_HASH_MAP_H_

#include <cassert>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <iostream>

#if defined(_MSC_VER) && _MSC_VER < 1800 || !defined(_MSC_VER) && __cplusplus < 201402L
// https://isocpp.org/files/papers/N3656.txt
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
namespace std {
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };
    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };
    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };
    template<class T, class... Args>
    typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
    template<class T>
    typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
        typedef typename remove_extent<T>::type U;
        return unique_ptr<T>(new U[n]());
    }
    template<class T, class... Args>
    typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}
#endif // __cplusplus < 201402L 


namespace ppstd
{

namespace detail
{

template <typename K, typename V>
struct BackMapped
{
    V value;
    K* ptr_key;
    BackMapped(const V& v = V{}, K* pk = nullptr) : value{ v }, ptr_key{ pk } {}
    BackMapped(V&& v = V{}, K* pk = nullptr) : value{ v }, ptr_key{ pk } {}
};

template <typename K, typename V>
class Iterator
{
private:
    using List = typename std::conditional<
        std::is_const<V>::value,
        const std::list< detail::BackMapped< K, typename std::remove_const<V>::type > >,
        std::list< detail::BackMapped<K, V> >
        >::type;
    using ListIterator = typename std::conditional<
        std::is_const<V>::value,
        typename List::const_iterator,
        typename List::iterator
        >::type;
        
    List* ptr_back_list_;
    ListIterator iter_back_list_;

public:
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator(List* const pbl = nullptr, const ListIterator& ibl = ListIterator{})
        : ptr_back_list_{ pbl }, iter_back_list_{ ibl } {}

    Iterator(const Iterator& rhs)
        : ptr_back_list_{ rhs.ptr_back_list_ }, iter_back_list_{ rhs.iter_back_list_ } {}

    Iterator(Iterator&& rhs) : Iterator()
    {
        swap(rhs);
    }

    ~Iterator() {}

    Iterator& operator=(Iterator rhs)
    {
        swap(rhs);
        return *this;
    }

    Iterator& operator++()
    {
        ++iter_back_list_;
        return *this;
    }

    Iterator operator++(int)
    {
        Iterator orig(*this);
        ++(*this);
        return orig;
    }

    Iterator& operator--()
    {
        --iter_back_list_;
        return *this;
    }

    Iterator operator--(int)
    {
        Iterator orig(*this);
        --(*this);
        return orig;
    }

    std::pair<K&, V&> operator*()
    {
        return std::make_pair(std::ref(*(iter_back_list_->ptr_key)), std::ref(iter_back_list_->value));
    }

    std::unique_ptr<std::pair<K&, V&>> operator->()
    {
        return std::make_unique<std::pair<K&, V&>>(
            *(iter_back_list_->ptr_key), iter_back_list_->value);
    }

    void swap(Iterator& rhs)
    {
        using std::swap;
        swap(ptr_back_list_, rhs.ptr_back_list_);
        swap(iter_back_list_, rhs.iter_back_list_);
    }

    friend bool operator==(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.ptr_back_list_ == rhs.ptr_back_list_ &&
            lhs.iter_back_list_ == rhs.iter_back_list_;
    }

    friend bool operator!=(const Iterator& lhs, const Iterator& rhs)
    {
        return !(lhs == rhs);
    }

    friend void swap(Iterator& lhs, Iterator& rhs)
    {
        lhs.swap(rhs);
    }
}; // class Iterator

} // namespace detail


template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
class linked_hash_map
{
private:
    using BackList = std::list<detail::BackMapped<const Key, T>>;
    using BackListIterator = typename BackList::iterator;
    using BackListConstIterator = typename BackList::const_iterator;
    BackList back_list_;
    using BackMap = std::unordered_map<Key, BackListIterator, Hash, Pred>;
    using BackMapIterator = typename BackMap::iterator;
    using BackMapConstIterator = typename BackMap::const_iterator;
    BackMap back_map_;

public:
    using key_type = Key;
    using mapped_type = T;
    using hasher = Hash;
    using key_equal = Pred;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    using iterator = detail::Iterator<const key_type, mapped_type>;
    using const_iterator = detail::Iterator<const key_type, const mapped_type>;


    linked_hash_map()
        noexcept(
            std::is_nothrow_default_constructible<hasher>::value &&
            std::is_nothrow_default_constructible<key_equal>::value) {}

    explicit linked_hash_map(size_type n, const hasher& hf = hasher(),
        const key_equal& eql = key_equal()) : back_map_(n, hf, eql) {}

    template <class InputIterator>
    linked_hash_map(InputIterator first, InputIterator last,
        size_type n = 0, const hasher& hf = hasher(),
        const key_equal& eql = key_equal()) : linked_hash_map(n, hf, eql)
    {
        insert(first, last);
    }

    linked_hash_map(const linked_hash_map& rhs) : linked_hash_map()
    {
        insert(rhs.begin(), rhs.end());
    }

    linked_hash_map(linked_hash_map&& rhs)
        noexcept(
            std::is_nothrow_move_constructible<hasher>::value &&
            std::is_nothrow_move_constructible<key_equal>::value) : linked_hash_map()
    {
        swap(rhs);
    }

    linked_hash_map(std::initializer_list<value_type> ilist, size_type n = 0,
        const hasher& hf = hasher(), const key_equal& eql = key_equal())
        : linked_hash_map(n, hf, eql)
    {
        insert(ilist.begin(), ilist.end());
    }

    linked_hash_map(size_type n) : linked_hash_map(n, hasher(), key_equal()) {}

    linked_hash_map(size_type n, const hasher& hf) : linked_hash_map(n, hf, key_equal()) {}

    template <class InputIterator>
    linked_hash_map(InputIterator f, InputIterator l, size_type n)
        : linked_hash_map(f, l, n, hasher(), key_equal()) {}

    template <class InputIterator>
    linked_hash_map(InputIterator f, InputIterator l, size_type n, const hasher& hf)
        : linked_hash_map(f, l, n, hf, key_equal()) {}

    linked_hash_map(std::initializer_list<value_type> il, size_type n)
        : linked_hash_map(il, n, hasher(), key_equal()) {}

    linked_hash_map(std::initializer_list<value_type> il, size_type n, const hasher& hf)
        : linked_hash_map(il, n, hf, key_equal()) {}

    ~linked_hash_map() {}

    linked_hash_map& operator=(linked_hash_map rhs)
    {
        swap(rhs);
        return *this;
    }

    linked_hash_map& operator=(linked_hash_map&& rhs)
    {
        swap(rhs);
        return *this;
    }

    linked_hash_map& operator=(std::initializer_list<value_type> ilist)
    {
        insert(ilist.begin(), ilist.end());
        return *this;
    }


    bool empty() const noexcept { return back_map_.empty(); }
    size_type size() const noexcept { return back_map_.size(); }
    size_type max_size() const noexcept { return back_map_.max_size(); }


    iterator begin() noexcept { return iterator(&back_list_, back_list_.begin()); }
    iterator end() noexcept { return iterator(&back_list_, back_list_.end()); }
    const_iterator begin() const noexcept { return const_iterator(&back_list_, back_list_.begin()); }
    const_iterator end() const noexcept { return const_iterator(&back_list_, back_list_.end()); }
    const_iterator cbegin() const noexcept { return const_iterator(&back_list_, back_list_.cbegin()); }
    const_iterator cend() const noexcept { return const_iterator(&back_list_, back_list_.cend()); }


    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return insert(std::move(value_type{ std::forward<Args>(args)... }));
    }

    template <class... Args>
    iterator emplace_hint(const_iterator position, Args&&... args)
    {
        // a simplified version for this api, because we always insert it to the last
        // maybe should compare the key value and get rejected if equal early
        return emplace(std::forward<Args>(args)...).first;
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        if (back_map_.count(value.first) == 0)
        {
            detail::BackMapped<const key_type, mapped_type> v{ value.second };
            back_list_.push_back(std::move(v));
            BackListIterator it = --back_list_.end();
            std::pair<BackMapIterator, bool> back_res = back_map_.insert({ value.first, it });
            it->ptr_key = &((back_res.first)->first);
            return std::pair<iterator, bool>{iterator(&back_list_, it), true};
        }
        else
        {
            BackListIterator it = back_map_.find(value.first)->second;
            return std::pair<iterator, bool>{iterator(&back_list_, it), false};
        }
    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        insert(ilist.begin(), ilist.end());
    }


    hasher hash_function() const { return back_map_.hash_function(); }
    key_equal key_eq() const { return back_map_.key_eq(); }


    iterator erase(const_iterator position)
    {
        iterator res = find(position->first);
        return erase(res);
    }

    iterator erase(iterator position)
    {
        // very inefficient in this implement
        iterator res{ position };
        ++res;
        const key_type k = position->first;
        back_list_.erase(back_map_.at(k));
        back_map_.erase(k);
        return res;
    }

    size_type erase(const key_type& k)
    {
        if (count(k) == 0)
        {
            return 0;
        }
        erase(find(k));
        return 1;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        iterator res;
        for (; first != last; ++first)
        {
            res = erase(first);
        }
        assert(res->first == last->first || "the range for erase() is invalid");
        return res;
    }

    void clear() noexcept { back_map_.clear(); back_list_.clear(); }


    void swap(linked_hash_map& rhs) noexcept
    {
        using std::swap;
        swap(back_list_, rhs.back_list_);
        swap(back_map_, rhs.back_map_);
    }


    iterator find(const key_type& k)
    {
        BackMapIterator map_it = back_map_.find(k);
        return map_it == back_map_.end() ?
            iterator(&back_list_, back_list_.end()) :
            iterator(&back_list_, map_it->second);
    }

    const_iterator find(const key_type& k) const
    {
        BackMapConstIterator map_it = back_map_.find(k);
        return map_it == back_map_.end() ?
            const_iterator(&back_list_, back_list_.end()) :
            const_iterator(&back_list_, map_it->second);
    }

    size_type count(const key_type& k) const { return back_map_.count(k); }

    std::pair<iterator, iterator> equal_range(const key_type& k)
    {
        iterator lower_it = find(k);
        iterator upper_it = lower_it;
        if (upper_it != end())
        {
            ++upper_it;
        }
        return std::pair<iterator, iterator>{std::move(lower_it), std::move(upper_it)};
    }


    mapped_type& operator[](const key_type& k)
    {
        try
        {
            return back_map_.at(k)->value;
        }
        catch (const std::out_of_range&)
        {
            return insert({ k, T{} }).first->second;
        }
    }

    mapped_type& operator[](key_type&& k)
    {
        try
        {
            return back_map_.at(k)->value;
        }
        catch (const std::out_of_range&)
        {
            return insert({ k, T{} }).first->second;
        }
    }


    mapped_type& at(const key_type& k) { return back_map_.at(k)->value; }
    const mapped_type& at(const key_type& k) const { return back_map_.at(k)->value; }


    size_type bucket_count() const noexcept { return back_map_.bucket_count(); }
    size_type max_bucket_count() const noexcept { return back_map_.max_bucket_count(); }


    size_type bucket_size(size_type n) const { return back_map_.bucket_size(n); }
    size_type bucket(const key_type& k) const { return back_map_.bucket(k); }


    float load_factor() const noexcept { return back_map_.load_factor(); }
    float max_load_factor() const noexcept { return back_map_.max_load_factor(); }
    void max_load_factor(float z) { return back_map_.max_load_factor(z); }
    void rehash(size_type n) { return back_map_.rehash(n); }
    void reserve(size_type n) { return back_map_.reserve(n); }


    friend bool operator==(linked_hash_map& lhs, linked_hash_map& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (auto lhs_it = lhs.begin(), rhs_it = rhs.begin();
            lhs_it != lhs.end() && rhs_it != rhs.end();
            ++lhs_it, ++rhs_it)
        {
            if (lhs_it->first != rhs_it->first || lhs_it->second != rhs_it->second)
            {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(linked_hash_map& lhs, linked_hash_map& rhs)
    {
        return !(lhs == rhs);
    }

    friend void swap(linked_hash_map& lhs, linked_hash_map& rhs)
    {
        lhs.swap(rhs);
    }

}; // class linked_hash_map

} // namespace ppstd

#endif // PPSTD_LINKED_HASH_MAP_H_
