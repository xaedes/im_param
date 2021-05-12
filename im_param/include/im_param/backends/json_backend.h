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
        
        nlohmann::json& json() { return stack[0]; }
        std::string json_string(int indent = 0) 
        { 
            auto json = this->json();
            return indent ? json.dump(indent) : json.dump();
        }

        #pragma region specializations for named parameters (sliders, checkboxes, ...)
        template<
            typename float_type,
            typename A = float_type, typename B = float_type,
            std::enable_if_t<std::is_floating_point<float_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(const std::string& name, float_type& value, A min=0, B max=1)
        {
            stack.back()[name] = value;
            return *this;
        }

        template<
            typename int_type,
            typename A = int_type, typename B = int_type,
            std::enable_if_t<Backend::is_non_bool_integral<int_type>::value, bool> = true
        >
        JsonSerializerBackend& parameter(const std::string& name, int_type& value, A min=0, B max=1)
        {
            stack.back()[name] = value;
            return *this;
        }

        template<
            typename bool_type,
            std::enable_if_t<std::is_same<bool_type, bool>::value, bool> = true
        >
        JsonSerializerBackend& parameter(const std::string& name, bool_type& value)
        {
            stack.back()[name] = value;
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named parameter container
        template<typename T, typename U, std::enable_if_t<!Backend::is_specialized<T>::value, bool> = true>
        JsonSerializerBackend& parameter(const std::string& name, T& params, const U& typeholder, HierarchyType hierarchy_type = HierarchyType::Tree)
        {
            stack.resize(stack.size()+1);
            im_param::parameter(*this, params, typeholder);
            stack[stack.size()-2][name] = stack.back();
            stack.pop_back();

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
        nlohmann::json& json() { return stack[0]; }
        nlohmann::json& parse(const std::string& json_str) 
        { 
            stack = {nlohmann::json::parse(json_str)};
            changed = false;
            return json(); 
        }
        
        bool changed = false;

        #pragma region specializations for named parameters (sliders, checkboxes, ...)
        template<
            typename float_type,
            typename A = float_type, typename B = float_type,
            std::enable_if_t<std::is_floating_point<float_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(const std::string& name, float_type& value, A min=0, B max=1)
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

        template<
            typename int_type,
            typename A = int_type, typename B = int_type,
            std::enable_if_t<Backend::is_non_bool_integral<int_type>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(const std::string& name, int_type& value, A min=0, B max=1)
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

        template<
            typename bool_type,
            std::enable_if_t<std::is_same<bool_type, bool>::value, bool> = true
        >
        JsonDeserializerBackend& parameter(const std::string& name, bool_type& value)
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
        template<typename T, typename U, std::enable_if_t<!Backend::is_specialized<T>::value, bool> = true>
        JsonDeserializerBackend& parameter(const std::string& name, T& params, const U& typeholder, HierarchyType hierarchy_type = HierarchyType::Tree)
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
    protected:

        std::vector<nlohmann::json> stack;
        
    };


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

        #pragma region specializations for named parameters (sliders, checkboxes, ...)
        template<
            typename float_type,
            typename A = float_type, typename B = float_type,
            std::enable_if_t<std::is_floating_point<float_type>::value, bool> = true
        >
        JsonStringStreamSerializerBackend& parameter(const std::string& name, float_type& value, A min=0, B max=1)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': " << value;
            stack.back()++;
            return *this;
        }

        template<
            typename int_type,
            typename A = int_type, typename B = int_type,
            std::enable_if_t<Backend::is_non_bool_integral<int_type>::value, bool> = true
        >
        JsonStringStreamSerializerBackend& parameter(const std::string& name, int_type& value, A min=0, B max=1)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': " << value;
            stack.back()++;
            return *this;
        }

        template<
            typename bool_type,
            std::enable_if_t<std::is_same<bool_type, bool>::value, bool> = true
        >
        JsonStringStreamSerializerBackend& parameter(const std::string& name, bool_type& value)
        {
            if (stack.back() > 0)  sstream << ", \n";
            make_indent();
            sstream << "'" << name << "': " << value;
            stack.back()++;
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named parameter container
        template<typename T, typename U, std::enable_if_t<!Backend::is_specialized<T>::value, bool> = true>
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
        void make_indent()
        {
            for (int i =0; i < stack.size(); ++i)
            {
                sstream << indent;
            }
        }
        std::vector<int> stack;
        std::ostringstream sstream;
        std::ostringstream sstream_done;
        
    };
} // namespace im_param
