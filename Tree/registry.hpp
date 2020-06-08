#pragma once

#include <vector>
#include <utility>
#include <optional>
#include <algorithm>
#include <stdexcept>


template<class T>
class registry 
{
public:

    template<typename... Args>
    std::size_t emplace(Args&&... args)
    {
        const std::size_t currID = id_;
        elems_.emplace_back(currID, std::forward<Args>(args)...);
        ++size_;
        ++id_;
        return currID;
    }

    const T& value(std::size_t id) const
    {
        const auto p = std::lower_bound(std::begin(elems_), std::end(elems_), id, [](const auto& a, const auto& b) { return a.first < b; });
        if (p == std::end(elems_) || p->first != id || !p->second)
            throw std::invalid_argument("value with this id not found");

        return *p->second;
    }

    T& value(std::size_t id) { return const_cast<T&>(const_cast<const registry*>(this)->value(id)); }

    void erase(std::size_t id) 
    {
        const auto p = std::lower_bound(std::begin(elems_), std::end(elems_), id, [](const auto& a, const auto& b) { return a.first < b; });

        if (p == std::end(elems_) || p->first != id) return;

        p->second.reset();
        --size_;

        if (size_ < (std::size(elems_) / 2))
            elems_.erase(std::remove_if(std::begin(elems_), std::end(elems_), [](const auto& e) { return !e.second; }), std::end(elems_));
    }

    template<class F>
    void for_each(F f) 
    {
        for (const auto& e : elems_)
            if (e.second) f(*e.second);
    }

private:
    std::size_t id_ = 0;
    std::size_t size_ = 0;
    std::vector<std::pair<std::size_t, std::optional<T>>> elems_;
};