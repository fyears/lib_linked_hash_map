# `linked_hash_map.hpp`

An attempt to implement the linked hash map data structure in C++ 11 as a library. The `ppstd::linked_hash_map` has similar semantics with `std::unordered_map`, but preserves the insertion orders of the key value pairs. It's similar to Java's `java.util.LinkedHashMap`.

**Very important! The library should be considered alpha quality. Use it at your own risk!**


## usage

The library is header-only. Include the header file, initialize some instances of `ppstd::linked_hash_map`, and checkout the corresponding APIs of [`std::unordered_map`](http://en.cppreference.com/w/cpp/container/unordered_map) for reference. To achieve the insertion order, use the iterator.

See [`./test`](./test) folder for some examples.

The library is developed and tested on Visual Studio 2015's MSVC, g++ 4.8, clang++ 3.8.


## attention

**Very important! The library should be considered alpha quality. Use it at your own risk!**

That being said, it should be ok to be used in normal ways... And feel free to check out the short source code and fix any buges.


## limitations

- Support for C++ 11 or later standards only.

- No `Allocator` support. Mainly because I, the library creator, don't have full understanding of it. Thus any APIs making good use or requiring `Allocator` are not implemented. The classes use `std`'s default allocators.

- No corresponding C++ 17 APIs.

- `ppstd::linked_hash_map::iterator` and `ppstd::linked_hash_map::const_iterator` should hopefully be usable. For example, your can do `auto iter = m.begin()`, `++iter`, `iter--`, `std::cout << iter->first << ' ' << iter->second << '\n'` etc. But they don't fully follow the requirements of `std`'s containers' iterators.

- Some space overhead compared with pure `std::unordered_map`. And don't expect that it could run really fast.


## implementation details

Internally, a `linked_hash_map<Key, T>` uses one `std::unordered_map` and the doubly linked list `std::list`. The elements of `std::unordered_map` are the key value pairs of actual `const Key` values and the iterators of `std::list`, while the elements of `std::list` store the actual `T` values and a pointer to the `const Key` in `std::unordered_map` (so that the key can be tracked by using elements in `std::list`).

While inserting a new `std::pair<const Key, T>`, the key is inserted into the inernal map, and the value is inserted into the end of the internal list. While erasing a pair, manipulate both the internal map and list.

As a result, the searches, hashing, etc, follow `std::unordered_map`'s semantics. But the iterators follow `std::list`'s semantics, and keep the insertation orders.

This is actually the usual design of the linked hash map data structure. As far as I know, many Java implementations implement `LinkedHashMap` in similar way.

```txt
+-----------------------------+
|  unordered_map              |
|                             |
|       key1        key2      |
|        |           |        | 
|        V           V        |
|      iter1       iter2      |
|        |           |        |
+--------+-----------+--------+
         |           |
+--------+-----------+--------+
|        |           |        |
|        V           V        |
| --> value1 <---> value2 <-- |
|                             |
|   list                      |
+-----------------------------+
```


## faq

### why develop this?

C++ lacks standard container similar to Java's `java.util.LinkedHashMap`.

### why choose namespace `ppstd`?

It's a joke. `ppstd == plus_plus_std == ++std`. Feel free to change it according to your project.


## license

MIT License.

Some codes are modified from LLVM's `libc++`'s source codes, that also follow MIT License.

A copy of `std::make_unique` implmentation for C++ 11 compatibility is taken from [Stephan T. Lavavej's draft N3656](https://isocpp.org/files/papers/N3656.txt).
