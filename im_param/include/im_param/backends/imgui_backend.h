#pragma once

#include <vector>
#include <imgui.h>
#include "imgui_candy/imgui_candy.h"
#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/backend.h"

namespace im_param {

    struct ImGuiBackend {

        bool changed = false;

        #pragma region specializations for named parameter multi channel values (floats, ints, bools, etc)
        // ImGuiBackend: named parameter multi channel values (floats, ints, bools, etc)
        template<
            typename value_type,
            typename float_type,
            typename size_type = std::size_t,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        >
        ImGuiBackend& parameter(
            const std::string& name, 
            value_type* ptr, 
            size_type count, 
            const float_type* min_scaled=nullptr, 
            const float_type* max_scaled=nullptr, 
            const float_type* unit_scale=nullptr
        )
        {
            bool scale_one = true;
            for (int i=0; i<count; i++)
            {
                if (unit_scale[i] != 1)
                {
                    scale_one = false;
                    break;
                }
            }
            if (scale_one) 
            {
                changed |= ImGuiSliderScalarN<float_type>(
                    ImGuiCandy::append_id(name, ptr).c_str(), 
                    ptr, count, min_scaled, max_scaled
                );
            }
            else 
            {
                changed |= ImGuiSliderScaledScalarN<float_type>(
                    ImGuiCandy::append_id(name, ptr).c_str(), 
                    ptr, count, unit_scale, min_scaled, max_scaled
                );
            }
            return *this; 
        }
        #pragma endregion

        #pragma region specializations for named parameter values (floats, ints, bools, etc)
        // ImGuiBackend: named parameter floats values
        template<
            typename float_type,
            typename A = float_type, 
            typename B = float_type,
            typename C = float_type,
            std::enable_if_t<std::is_floating_point<float_type>::value, bool> = true
        >
        ImGuiBackend& parameter(const std::string& name, float_type& value, A min_scaled=0, B max_scaled=1, C unit_scale=1)
        {
            if (unit_scale == 1) 
            {
                changed |= ImGuiSliderScalar<float_type>(
                    ImGuiCandy::append_id(name, &value).c_str(), 
                    &value, min_scaled, max_scaled
                );
            }
            else 
            {
                changed |= ImGuiSliderScaledScalar<float_type>(
                    ImGuiCandy::append_id(name, &value).c_str(), 
                    &value, unit_scale, static_cast<C>(min_scaled), static_cast<C>(max_scaled)
                );
            }
            return *this;
        }

        // ImGuiBackend: named parameter int values
        template<
            typename int_type,
            typename A = int_type, 
            typename B = int_type,
            typename C = int_type,
            std::enable_if_t<Backend::is_non_bool_integral<int_type>::value, bool> = true
        >
        ImGuiBackend& parameter(const std::string& name, int_type& value, A min_scaled=0, B max_scaled=1, C unit_scale=1)
        {
            if (unit_scale == 1) 
            {
                changed |= ImGuiSliderScalar<int_type>(
                    ImGuiCandy::append_id(name, &value).c_str(),
                    &value, min_scaled, max_scaled
                );
            }
            else 
            {
                changed |= ImGuiSliderScaledScalar<int_type>(
                    ImGuiCandy::append_id(name, &value).c_str(),
                    &value, unit_scale, static_cast<C>(min_scaled), static_cast<C>(max_scaled)
                );
            }
            return *this;
        }

        // ImGuiBackend: named parameter bool values
        template<
            typename bool_type,
            std::enable_if_t<std::is_same<bool_type, bool>::value, bool> = true
        >
        ImGuiBackend& parameter(const std::string& name, bool_type& value)
        {
            changed |= ImGui::Checkbox(name.c_str(), &value);
            return *this;
        }
        #pragma endregion

        #pragma region specializations for named parameter container
        // ImGuiBackend: named parameter container
        template<typename T, typename U, std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true>
        ImGuiBackend& parameter(const std::string& name, T& params, const TypeHolder<U>& typeholder, HierarchyType hierarchy_type = HierarchyType::Tree)
        {
            switch(hierarchy_type)
            {
            case HierarchyType::Flat:
            {
                ImGui::Text(name.c_str());
                im_param::parameter(*this, params, typeholder);
            }
            break;
            case HierarchyType::CollapsingHeader:
            {
                if (ImGui::CollapsingHeader(name.c_str()))
                {
                    im_param::parameter(*this, params, typeholder);
                    // ImGui::NewLine();
                    // ImGui::Separator();
                }
            }
            break;
            case HierarchyType::Tree:
            {
                if (ImGui::TreeNode(name.c_str()))
                {
                    im_param::parameter(*this, params, typeholder);
                    // ImGui::NewLine();
                    // ImGui::Separator();

                    ImGui::TreePop();
                }
            }
            break;
            default:
                throw std::runtime_error("not implemented");
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter containers
        // ImGuiBackend: named list of parameter containers
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class type_holder_type,
            class... Args,
            std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true

            // typename T, 
            // typename I, 
            // typename U, 
            // std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true
        >
        ImGuiBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            TypeHolder<type_holder_type> typeholder,
            HierarchyType list_hierarchy_type = HierarchyType::Tree,
            HierarchyType item_hierarchy_type = HierarchyType::Tree,
            Args... args

            // const std::string& name, T&& begin, T&& end, I&& inserter, 
            // const TypeHolder<U>& typeholder, 
            // HierarchyType list_type = HierarchyType::Tree,
            // HierarchyType hierarchy_type = HierarchyType::Tree
        )
        {
            auto begin = collection.begin();
            auto end = collection.end();
            switch(list_hierarchy_type)
            {
            case HierarchyType::Flat:
            {
                ImGui::Text(name.c_str());
                for (auto it = begin; it != end; ++it)
                {
                    im_param::parameter(*this, *it, typeholder, item_hierarchy_type);
                }
            }
            break;
            case HierarchyType::CollapsingHeader:
            {
                if (ImGui::CollapsingHeader(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it)
                    {
                        im_param::parameter(*this, *it, typeholder, item_hierarchy_type);
                    }
                }
            }
            break;
            case HierarchyType::Tree:
            {
                if (ImGui::TreeNode(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it)
                    {
                        im_param::parameter(*this, *it, typeholder, item_hierarchy_type);
                    }

                    ImGui::TreePop();
                }
            }
            break;
            default:
                throw std::runtime_error("not implemented");
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter values (floats, ints, bools, etc)
        // ImGuiBackend: named list of parameter values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class... Args,
            std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true

            // typename T, 
            // typename I, 
            // typename U, 
            // std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true
        >
        ImGuiBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            HierarchyType list_hierarchy_type = HierarchyType::Tree,
            Args... args

            // const std::string& name, T&& begin, T&& end, I&& inserter, 
            // const TypeHolder<U>& typeholder, 
            // HierarchyType list_type = HierarchyType::Tree,
            // HierarchyType hierarchy_type = HierarchyType::Tree
        )
        {
            auto begin = collection.begin();
            auto end = collection.end();
            int index = 0;
            switch(list_hierarchy_type)
            {
            case HierarchyType::Flat:
            {
                ImGui::Text(name.c_str());
                for (auto it = begin; it != end; ++it, ++index)
                {
                    im_param::parameter(*this, std::to_string(index), *it);
                }
            }
            break;
            case HierarchyType::CollapsingHeader:
            {
                if (ImGui::CollapsingHeader(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it, ++index)
                    {
                        im_param::parameter(*this, std::to_string(index), *it);
                    }
                }
            }
            break;
            case HierarchyType::Tree:
            {
                if (ImGui::TreeNode(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it, ++index)
                    {
                        im_param::parameter(*this, std::to_string(index), *it);
                    }
                    ImGui::TreePop();
                }
            }
            break;
            default:
                throw std::runtime_error("not implemented");
            }

            return *this;
        }
        #pragma endregion

        #pragma region specializations for named list of parameter multi channel values (floats, ints, bools, etc)
        // ImGuiBackend: named list of parameter multi channel values (floats, ints, bools, etc)
        template<
            class collection_type,
            class value_type,
            class value_iterator_type,
            class inserter_iterator_type,
            class callback_type,
            class size_type = std::size_t,
            class... Args
            // std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true

            // typename T, 
            // typename I, 
            // typename U, 
            // std::enable_if_t<!Backend::is_base_value<T>::value, bool> = true
        >
        ImGuiBackend& parameter(
            const std::string& name, 
            Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection, 
            callback_type callback_get_first_val,
            size_type num_channels, 
            HierarchyType list_hierarchy_type = HierarchyType::Tree,
            Args... args

            // const std::string& name, T&& begin, T&& end, I&& inserter, 
            // const TypeHolder<U>& typeholder, 
            // HierarchyType list_type = HierarchyType::Tree,
            // HierarchyType hierarchy_type = HierarchyType::Tree
        )
        {
            auto begin = collection.begin();
            auto end = collection.end();
            int index = 0;
            switch(list_hierarchy_type)
            {
            case HierarchyType::Flat:
            {
                ImGui::Text(name.c_str());
                for (auto it = begin; it != end; ++it, ++index)
                {
                    auto* ptr = callback_get_first_val(*it);
                    im_param::parameter(*this, std::to_string(index), *ptr, num_channels);
                }
            }
            break;
            case HierarchyType::CollapsingHeader:
            {
                if (ImGui::CollapsingHeader(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it, ++index)
                    {
                        auto* ptr = callback_get_first_val(*it);
                        im_param::parameter(*this, std::to_string(index), *ptr, num_channels);
                    }
                }
            }
            break;
            case HierarchyType::Tree:
            {
                if (ImGui::TreeNode(name.c_str()))
                {
                    for (auto it = begin; it != end; ++it, ++index)
                    {
                        auto* ptr = callback_get_first_val(*it);
                        im_param::parameter(*this, std::to_string(index), *ptr, num_channels);
                    }
                    ImGui::TreePop();
                }
            }
            break;
            default:
                throw std::runtime_error("not implemented");
            }

            return *this;
        }
        #pragma endregion


    };

} // namespace im_param
