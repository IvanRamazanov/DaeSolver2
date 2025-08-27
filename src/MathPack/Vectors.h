#ifndef MATHPACK_VECTORS_H
#define MATHPACK_VECTORS_H

#include <memory>
#include <vector>
#include <concepts>
#include <type_traits>
#include <ranges>

namespace MathPack{
    template <typename T, typename V>
    inline size_t lastIndexOf(V const& arr, T *value){
        for (size_t i=arr.size(); i-->0;){
            if (arr[i].get() == value) {
                return i;
            }
        }
        return -1;
    }


    template<typename T>
    concept is_range_gettable = requires (T t){
        t.at(0).get();
    };

    template<typename T>
    concept is_not_range_gettable = is_range_gettable<T> == false;

    template<is_range_gettable V, typename T>
    inline size_t indexOf(V const& arr, T *value){
        for (size_t i=0; i<arr.size();i++){
            if (arr[i].get() == value) {
                return i;
            }
        }
        return -1;
    }

    template<is_not_range_gettable T, typename V>
    inline size_t indexOf(T const& arr, V *value){
        for (size_t i=0; i<arr.size();i++){
            if (arr[i] == value) {
                return i;
            }
        }
        return -1;
    }

    template <typename T, typename V>
    inline pair<size_t, size_t> indexOf_mat(vector<vector<V>> const& arr, T* const& value){
        for (size_t i=0; i<arr.size();i++){
            for (size_t j=0; j<arr[i].size();j++){
                if (arr[i][j] == value) {
                    return {i, j};
                }
            }
        }
        return {-1, -1};
    }

    template<typename T>
    concept appendable = requires (T t){t.insert(t.end(), t.begin(), t.end());};

    template<appendable T, appendable V>
    inline void append(T & a, V const& b){
        a.insert(a.end(), b.begin(), b.end());
    }

    template <ranges::range T, typename V>
    inline bool contains(T const& vec,  V const& item){
        for (auto& e:vec){
            if (e == item){
                return true;
            }
        }
        return false;
    }

    template <ranges::range T, typename V>
    inline void remove(T & vec,  V const& item){
        for (auto i=vec.begin(); i != vec.end(); i++){
            bool match;
            if constexpr (is_range_gettable<T>){
                match = (*i).get() == item;
            }else{
                match = *i == item;
            }
            if (match){
                vec.erase(i);
                break;
            }
        }
    }
}

#endif
