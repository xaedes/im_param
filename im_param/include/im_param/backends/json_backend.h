#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/backend.h"
#include "im_param/detail/nlohmann_json.hpp"

namespace im_param {

    struct JsonSerializerBackend {

        JsonSerializerBackend()
        {
            clear();
        }

        void clear()
        {
            stack = {nlohmann::json()};
        }
        
        std::string serialized()
        {
            return json_string();
        }
        
        nlohmann::json& json() { return stack[0]; }
        std::string json_string(int indent = 0) 
        { 
            auto json = this->json();
            return indent ? json.dump(indent) : json.dump();
        }

        #pragma region specializations for named parameter multi channel values (floats, ints, bools, strings, etc)
        // JsonSerializerBackend: named parameter multi channel values (floats, ints, bools, strings, etc)
        template<
            typename value_type,
            typename size_type = std::size_t,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(const std::string& name, value_type* ptr, size_type count, Args... args)
        {
            stack.back()[name] = nlohmann::json::array();
            for (int i=0; i<count; i++)
            {
                stack.back()[name].emplace_back(ptr[i]);
            }
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named parameter values (floats, ints, bools, etc)
        // JsonSerializerBackend: named parameter values (floats, ints, bools, etc)
        template<
            typename value_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(const std::string& name, value_type& value, Args... args)
        {
            stack.back()[name] = value;
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named parameter container
        // JsonSerializerBackend: named parameter container
        template<
            typename T, 
            typename U, 
            class... Args, 
            std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true
        >
        JsonSerializerBackend& parameter(
            const std::string& name, T& params, const TypeHolder<U>& typeholder, Args... args)
        {
            stack.emplace_back();
            im_param::parameter(*this, params, typeholder, std::forward<Args>(args)...);
            stack[stack.size()-2][name] = stack.back();
            stack.pop_back();

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter containers
        // JsonSerializerBackend: named list of parameter containers
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class type_holder_type,
            class... Args,
            std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            TypeHolder<type_holder_type> typeholder,
            Args... args
        )
        {
            stack.back()[name] = nlohmann::json::array();
            auto& arr = stack.back()[name];
            stack.emplace_back();
            for(auto it = collection.begin(); it != collection.end(); ++it)
            {
                stack.back() = nlohmann::json();
                im_param::parameter(*this, *it, typeholder);
                arr.emplace_back(stack.back());
            }
            stack.pop_back();
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter multi channel values (floats, ints, bools, etc)
        // JsonSerializerBackend: named list of parameter multi channel values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class callback_type,
            class size_type = std::size_t,
            class... Args,
            std::enable_if_t<!Backend::is_type_holder<callback_type>::value, bool> = true
            // std::enable_if_t<Backend::is_base_value<return_type<callback_type>>::value, bool> = true
            // std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            callback_type callback_get_first_val,
            size_type num_channels, 
            Args... args
        )
        {
            stack.back()[name] = nlohmann::json::array();
            auto& arr = stack.back()[name];
            for(auto it = collection.begin(); it != collection.end(); ++it)
            {
                auto values = nlohmann::json::array();
                const auto* ptr = callback_get_first_val(*it);
                for (int i=0; i<num_channels; i++)
                {
                    values.emplace_back(ptr[i]);
                }
                arr.emplace_back(values);
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter values (floats, ints, bools, etc)
        // JsonSerializerBackend: named list of parameter values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            Args... args
        )
        {
            stack.back()[name] = nlohmann::json::array();
            auto& arr = stack.back()[name];
            for(auto it = collection.begin(); it != collection.end(); ++it)
            {
                arr.emplace_back(*it);
            }
            return *this;
        }

        #pragma endregion

    protected:

        std::vector<nlohmann::json> stack;

        
    };

    struct JsonDeserializerBackend {

        JsonDeserializerBackend()
            : JsonDeserializerBackend(nlohmann::json())
        {}
        JsonDeserializerBackend(const std::string& json_str)
            : JsonDeserializerBackend(nlohmann::json::parse(json_str))
        {}
        JsonDeserializerBackend(const nlohmann::json& json)
        {
            stack.push_back(json);
        }
        void clear()
        {
            stack = {nlohmann::json()};
            changed = false;
        }

        void deserialize(const std::string& serialized_string)
        {
            parse(serialized_string);
        }

        nlohmann::json& json() { return stack[0]; }
        nlohmann::json& parse(const std::string& json_str) 
        { 
            stack = {nlohmann::json::parse(json_str)};
            changed = false;
            return json(); 
        }
        
        bool changed = false;

        #pragma region protected members
    protected:
        template<class value_type, class T>
        bool is_type_ok(T& deserialized)
        {
            return (
                (std::is_same<bool, value_type>::value && deserialized.is_boolean())
             || (!std::is_same<bool, value_type>::value && std::is_arithmetic<value_type>::value && deserialized.is_number())
            );
        }
        template<class value_type, class T, class U>
        bool deserialize_base_value(T& deserialized, U ptr)
        {
            if (is_type_ok<value_type>(deserialized))
            {
                *ptr = deserialized;  
                return true;
            }
            return false;
        }
        template<class value_type, class T, class U, class S>
        bool deserialize_base_values(T& deserialized_array, U ptr, S num_channels)
        {
            S n = deserialized_array.size() < num_channels ? deserialized_array.size() : num_channels;
            bool changed = false;
            for (S i=0; i<n; i++)
            {
                auto& deserialized = deserialized_array[i];
                if (is_type_ok<value_type>(deserialized))
                {
                    changed |= (deserialized != *ptr);
                    *ptr = deserialized;
                }
                ++ptr;
            }
            return changed;
        }
        #pragma endregion
        
    public:

        #pragma region specializations for named parameter multi channel values (floats, ints, bools, etc)
        // JsonDeserializerBackend: named parameter multi channel values (floats, ints, bools, etc)
        template<
            typename value_type,
            typename size_type = std::size_t,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(const std::string& name, value_type* ptr, size_type num_channels, Args... args)
        {
            if (stack.back().count(name) && stack.back()[name].is_array())
            {
                auto deserialized_array = stack.back()[name];
                changed |= deserialize_base_values<value_type>(deserialized_array, ptr, num_channels);
            }
            return *this;
        }

        #pragma endregion

        #pragma region specializations for named parameter values (floats, ints, bools, etc)
        // JsonDeserializerBackend: named parameter values (floats, ints, bools, etc)
        template<
            typename value_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(const std::string& name, value_type& value, Args... args)
        {
            if (stack.back().count(name))
            {
                auto deserialized = stack.back()[name];
                if (deserialized != value)
                {
                    value = deserialized;
                    changed |= true;
                }
            }
            return *this;
        }


        #pragma endregion

        #pragma region specializations for named parameter container
        // JsonDeserializerBackend: named parameter container
        template<typename T, typename U, class... Args, std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true>
        JsonDeserializerBackend& parameter(const std::string& name, T& params, const U& typeholder, Args... args)
        {
            if (stack.back().count(name))
            {
                stack.push_back(stack.back()[name]);
                im_param::parameter(*this, params, typeholder);
                stack.pop_back();
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter containers
        // JsonDeserializerBackend: named list of parameter containers
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class type_holder_type,
            class... Args,
            std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            TypeHolder<type_holder_type> typeholder,
            Args... args
        )
        {
            if (stack.back().count(name) && stack.back()[name].is_array())
            {
                auto& deserialized_array = stack.back()[name];

                // save copy of items to which we can compare the new items
                // to know whether values actually changed.
                // could be more efficient, but it should be ok for now.
                auto old_items = collection.collection; 
                std::size_t deserialized_size = deserialized_array.size();
                std::size_t old_size = collection.size();
                collection.clear();
                auto inserter = collection.inserter();
                value_type empty_value;
                for(std::size_t i = 0; i < deserialized_size; ++i)
                {
                    // reuse old items (they may contain important data that is not processed by this call)
                    value_type* value = (i < old_items.size()) ? &(old_items[i]) : &empty_value;

                    stack.push_back(deserialized_array[i]);
                    im_param::parameter(*this, *value, typeholder);
                    stack.pop_back();

                    *inserter = *value;
                    ++inserter;
                }
                changed |= old_size != collection.size();
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter multi channel values (floats, ints, bools, etc)
        // JsonDeserializerBackend: named list of parameter multi channel values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class callback_type,
            class size_type = std::size_t,
            class... Args,
            std::enable_if_t<!Backend::is_type_holder<callback_type>::value, bool> = true

            // std::enable_if_t<Backend::is_base_value<return_type<callback_type>>::value, bool> = true
            // std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            callback_type callback_get_first_val,
            size_type num_channels, 
            Args... args
        )
        {
            if (stack.back().count(name) && stack.back()[name].is_array())
            {
                auto deserialized_array = stack.back()[name];

                // save copy of items to which we can compare the new items
                // to know whether values actually changed.
                // could be more efficient, but it should be ok for now.
                auto old_items = collection.collection; 
                std::size_t deserialized_size = deserialized_array.size();
                std::size_t old_size = collection.size();
                collection.clear();
                auto inserter = collection.inserter();
                value_type empty_value;
                for(std::size_t i = 0; i < deserialized_size; ++i)
                {
                    // reuse old items (they may contain important data that is not processed by this call)
                    value_type* value = (i < old_items.size()) ? &(old_items[i]) : &empty_value;
                    auto* ptr = callback_get_first_val(*value);
                    using base_value_type = std::remove_pointer_t<decltype(ptr)>;
                    changed |= (deserialize_base_values<base_value_type>(deserialized_array[i], ptr, num_channels));
                    *inserter = *value;
                    ++inserter;
                }
                changed |= old_size != collection.size();
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter values (floats, ints, bools, etc)
        // JsonDeserializerBackend: named list of parameter values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            Args... args
        )
        {
            if (stack.back().count(name) && stack.back()[name].is_array())
            {
                auto deserialized_array = stack.back()[name];

                // save copy of items to which we can compare the new items
                // to know whether values actually changed.
                // could be more efficient, but it should be ok for now.
                auto old_items = collection.collection; 
                std::size_t deserialized_size = deserialized_array.size();
                std::size_t old_size = collection.size();
                collection.clear();
                auto inserter = collection.inserter();
                for(std::size_t i = 0; i < deserialized_size; ++i)
                {
                    if (deserialize_base_value<value_type>(deserialized_array[i], inserter))
                    {
                        ++inserter;
                    }
                }
                changed = old_items != collection.collection;
            }

            return *this;
        }

        
        #pragma endregion


    protected:

        std::vector<nlohmann::json> stack;
        
    };

    // [deprecated] use JsonSerializerBackend instead
    struct JsonStringStreamSerializerBackend {
        JsonStringStreamSerializerBackend()
        {
            clear();
        }
        void clear()
        {
            stack = {0};
        }

        std::string json_string() {
            sstream_done.clear();
            sstream_done << "{\n";
            sstream_done << sstream.str();
            sstream_done << "\n}\n";
            return sstream_done.str();
        }
        std::string indent = "  ";

        #pragma region specializations for named parameters (floats, ints, bools, etc)
        template<
            typename value_type,
            typename size_type = std::size_t,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonStringStreamSerializerBackend& parameter(const std::string& name, value_type* ptr, size_type count, Args... args)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': " << "[\n";

            for (int i=0; i<count; i++)
            {
                if (i > 0)
                {
                    sstream << ", \n";
                }
                make_indent(+1);
                sstream << ptr[i];
            }
            sstream << "\n";
            make_indent();
            sstream << "]";
            return *this;
        }


        template<
            typename value_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        JsonStringStreamSerializerBackend& parameter(const std::string& name, value_type& value, Args... args)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': " << value;
            stack.back()++;
            return *this;
        }

        #pragma endregion

        #pragma region specializations for named parameter container
        template<typename T, typename U, std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true>
        JsonStringStreamSerializerBackend& parameter(const std::string& name, T& params, const U& typeholder, HierarchyType hierarchy_type = HierarchyType::Tree)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': { \n";
            stack.push_back(0);
            im_param::parameter(*this, params, typeholder);
            stack.pop_back();
            sstream << "\n";
            make_indent();
            sstream << "}";
            stack.back()++;
            return *this;
        }
        #pragma endregion
    protected:
        void make_indent(int delta=0)
        {
            for (int i =0; i < stack.size()+delta; ++i)
            {
                sstream << indent;
            }
        }
        std::vector<int> stack;
        std::ostringstream sstream;
        std::ostringstream sstream_done;
    };
    
} // namespace im_param
