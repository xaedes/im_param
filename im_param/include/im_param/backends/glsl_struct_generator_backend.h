#pragma once

#include <string>
#include <sstream>
#include <cstdint> // fixed width integer types 

#include "imgui.h"
#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/backend.h"

namespace im_param {

    struct GlslStructGeneratorBackend {
        GlslStructGeneratorBackend()
        {
            clear();
        }
        void clear()
        {
            structs.clear();
            structs.resize(1);
            depths.clear();
            depths.resize(1);
            current = 0;
        }

        std::string glsl_string(int start_struct=0) {
            sstream_done.clear();
            for (int i = start_struct; i < structs.size(); ++i)
            {
                sstream_done << structs[i].str();
            }
            // sstream_done << "{\n";
            // sstream_done << "\n}\n";
            return sstream_done.str();
        }
        std::string indent = "  ";


        template <class T>
        struct is_value_type : public std::integral_constant<bool,
            disjunction<
                std::is_same<int32_t, T>,
                std::is_same<uint32_t, T>,
                std::is_same<float, T>,
                std::is_same<double, T>,
                std::is_same<bool, T>
            >::value
        >
        {};
        #pragma region specializations for named parameters (sliders, checkboxes, ...)

        template<typename value_type> void vec_prefix(value_type* ptr);
        template<> void vec_prefix( int32_t  * ptr ) { structs[current] << "i"; }
        template<> void vec_prefix( uint32_t * ptr ) { structs[current] << "u"; }
        template<> void vec_prefix( float    * ptr ) { }
        template<> void vec_prefix( double   * ptr ) { structs[current] << "d"; }
        template<> void vec_prefix( bool     * ptr ) { structs[current] << "b"; }
        template<typename value_type> void type_decl(value_type* ptr);
        template<> void type_decl( int32_t  * ptr ) { structs[current] << "int";    }
        template<> void type_decl( uint32_t * ptr ) { structs[current] << "uint";   }
        template<> void type_decl( float    * ptr ) { structs[current] << "float";  }
        template<> void type_decl( double   * ptr ) { structs[current] << "double"; }
        template<> void type_decl( bool     * ptr ) { structs[current] << "bool";   }

        template<
            typename value_type,
            typename size_type = std::size_t, 
            class... Args,
            enable_if_t<is_value_type<value_type>::value, bool> = true
        >
        GlslStructGeneratorBackend& parameter(const std::string& name, value_type* ptr, size_type count, Args... args)
        {
            if (count == 1)
            {
                this->parameter(name, ptr[0], std::forward<Args>(args)...);
            }
            else
            if ((2 <= count ) && (count <= 4))
            {
                make_indent();  vec_prefix(ptr); structs[current] << "vec" << count << " " << name << ";\n";
            }
            return *this;
        }

        template<
            typename value_type,
            typename size_type = std::size_t, 
            class... Args,
            enable_if_t<is_value_type<value_type>::value, bool> = true
        >
        GlslStructGeneratorBackend& parameter(const std::string& name, value_type& value, Args... args)
        {
            make_indent(); type_decl(&value); structs[current] << " " << name << ";\n";
            return *this;
        }

        #pragma endregion

        #pragma region specializations for named parameter container
        template<typename T, typename U, enable_if_t<!Backend::is_specialized<T>::value, bool> = true>
        GlslStructGeneratorBackend& parameter(const std::string& name, T& params, const U& typeholder, HierarchyType hierarchy_type = HierarchyType::Tree)
        {
            auto old = current;
            current = structs.size();
            structs.resize(structs.size()+1);
            depths.resize(structs.size()+1);
            make_indent(); structs[current] << "struct " << name << "\n";
            make_indent(); structs[current] << "{\n";
            ++depths[current];
            im_param::parameter(*this, params, typeholder);
            --depths[current];
            make_indent(); structs[current] << "};\n";
            current = old;

            make_indent(); structs[current] << name << " " << name << ";\n";
            return *this;
        }
        #pragma endregion
    protected:
        void make_indent(int delta=0)
        {
            for (int i =0; i < depths[current]+delta; ++i)
            {
                structs[current] << indent;
            }
        }
        std::vector<std::ostringstream> structs;
        std::vector<int> depths;
        int current = 0;
        // int depth;
        // std::ostringstream sstream;
        std::ostringstream sstream_done;
    };

} // namespace im_param
