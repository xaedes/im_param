#pragma once

#include <iterator>
#include <functional>

namespace im_param {

    /**
     * @brief      Holds read and write iterators to collection.
     * 
     * This template be further specialized for specific collect types.
     *
     * @tparam     collection_t       The type of the collection
     * @tparam     value_t            The type of values
     * @tparam     iterator_t         The value iterator type
     * @tparam     insert_iterator_t  The insert iterator type, where to add new items
     */
    template <
        class collection_t,
        class value_t           = typename collection_t::value_type,
        class iterator_t        = typename collection_t::iterator,
        class insert_iterator_t = typename std::back_insert_iterator<collection_t>
    >
    struct Collection
    {
        using collection_type      = collection_t;
        using value_type           = value_t;
        using iterator_type        = iterator_t;
        using insert_iterator_type = insert_iterator_t;
        using difference_type      = typename std::iterator_traits<iterator_type>::difference_type;

        Collection(collection_type& collection) : collection(collection) {}

        collection_type& collection;

        iterator_type        begin()      { return collection.begin();             }
        iterator_type        end()        { return collection.end();               }
        insert_iterator_type inserter()   { return std::back_inserter(collection); }
        void                 clear()      { return collection.clear();             }
        std::size_t          size() const { return collection.size();              }
    };

} // namespace im_param
