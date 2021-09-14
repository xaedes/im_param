#pragma once

#include <vector>
#include <imgui.h>
#include "imgui_candy/imgui_candy.h"
#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/backend.h"

namespace im_param {

    struct ImGuiBackend {

        bool changed = false;

        #pragma region specializations for named parameters (sliders, checkboxes, ...)
        template<
            typename value_type,
            typename size_type = std::size_t,
            std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true
        >
        ImGuiBackend& parameter(
            const std::string& name, 
            value_type* ptr, 
            size_type count, 
            const value_type* min_scaled=nullptr, 
            const value_type* max_scaled=nullptr, 
            const value_type* unit_scale=nullptr
        )
        {
            static std::vector<value_type> value_scaled;
            // static std::vector<value_type> min_scaled;
            // static std::vector<value_type> max_scaled;
            value_scaled.resize(count);
            for(size_type i=0; i<count; ++i)
            {
                value_scaled[i]  = ptr[i] / unit_scale[i];
            }
            if(ImGuiSliderScalarN<value_type>(name.c_str(), &value_scaled[0], count, min_scaled, max_scaled))
            {
                changed = true;
                for(size_type i=0; i<count; ++i)
                {
                    ptr[i]  = value_scaled[i] * unit_scale[i];
                }
            }
            return *this;
        }

        template<
            typename float_type,
            typename A = float_type, 
            typename B = float_type,
            typename C = float_type,
            std::enable_if_t<std::is_floating_point<float_type>::value, bool> = true
        >
        ImGuiBackend& parameter(const std::string& name, float_type& value, A min_scaled=0, B max_scaled=1, C unit_scale=1)
        {
            if (unit_scale == 1) changed |= ImGuiSliderScalar<float_type>(name.c_str(), &value, min_scaled, max_scaled);
            else 
            {
                float_type value_scaled = value / unit_scale;
                // float_type min_scaled = min; // / unit_scale;
                // float_type max_scaled = max; // / unit_scale;
                if(ImGuiSliderScalar<float_type>(name.c_str(), &value_scaled, min_scaled, max_scaled))
                {
                    changed = true;
                    value = value_scaled * unit_scale;
                }
            }
            return *this;
        }
        template<
            typename int_type,
            typename A = int_type, 
            typename B = int_type,
            typename C = int_type,
            std::enable_if_t<Backend::is_non_bool_integral<int_type>::value, bool> = true
        >
        ImGuiBackend& parameter(const std::string& name, int_type& value, A min_scaled=0, B max_scaled=1, C unit_scale=1)
        {
            if (unit_scale == 1) changed |= ImGuiSliderScalar<int_type>(name.c_str(), &value, min_scaled, max_scaled);
            else 
            {
                C value_scaled = value / unit_scale;
                // C min_scaled = min; // / unit_scale;
                // C max_scaled = max; // / unit_scale;
                if(ImGuiSliderScalar<C>(name.c_str(), &value_scaled, min_scaled, max_scaled))
                {
                    changed = true;
                    value = static_cast<int_type>(value_scaled * unit_scale);
                }
            }
            return *this;
        }

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
        template<typename T, typename U, std::enable_if_t<!Backend::is_specialized<T>::value, bool> = true>
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

        #pragma region templated imgui slider 
        template <class T> ImGuiDataType ImGuiDataTypeOfPtr( T*   val );
        template <> ImGuiDataType ImGuiDataTypeOfPtr( int8_t*   val ) { return ImGuiDataType_S8;     }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( uint8_t*  val ) { return ImGuiDataType_U8;     }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( int16_t*  val ) { return ImGuiDataType_S16;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( uint16_t* val ) { return ImGuiDataType_U16;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( int32_t*  val ) { return ImGuiDataType_S32;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( uint32_t* val ) { return ImGuiDataType_U32;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( int64_t*  val ) { return ImGuiDataType_S64;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( uint64_t* val ) { return ImGuiDataType_U64;    }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( float*    val ) { return ImGuiDataType_Float;  }
        template <> ImGuiDataType ImGuiDataTypeOfPtr( double*   val ) { return ImGuiDataType_Double; }

        template<typename T>
        bool ImGuiSliderScalar(const char* label, T* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0)
        {
            return ImGui::SliderScalar(label, ImGuiDataTypeOfPtr(p_data), p_data, p_min, p_max, format, flags);
        }
        template<typename T>
        bool ImGuiSliderScalar(const char* label, T* p_data, T p_min, T p_max, const char* format = NULL, ImGuiSliderFlags flags = 0)
        {
            return ImGuiSliderScalar(label, p_data, &p_min, &p_max, format, flags);
        }
        #pragma endregion

    };

} // namespace im_param
