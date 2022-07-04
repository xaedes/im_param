#pragma once

#include "im_param/im_param.h"
#include <vector>
#include <map>
#include <iterator>

namespace im_param {

    #pragma region std::vector

    template <class value_type, class... Args>
    Collection<std::vector<value_type, Args...>> make_collection(std::vector<value_type, Args...>& items)
    {
        return Collection<std::vector<value_type, Args...>>(items);
    }

    // named std::vector of base values
    template<
        class backend_type,
        class value_type,
        class... Args
        // enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        std::vector<value_type>& items,
        Args&&... args
    )
    {
        auto collection = make_collection(items);
        return parameter(
            backend,
            name, 
            collection,
            std::forward<Args>(args)...
        );
    }

    #pragma endregion

} // namespace
