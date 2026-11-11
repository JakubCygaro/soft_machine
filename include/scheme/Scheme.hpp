#pragma once
#include "s7.h"
#include <cstdint>
#include <functional>

namespace scheme {
template <class T>
class SchemeWrap {
public:
    using output_port_fn_t = void (*)(s7_scheme* sc, uint8_t c, s7_pointer port);

private:
    ::s7_scheme* m_s7 = nullptr;
    ::s7_pointer m_default_output_port { };
    ::s7_pointer m_default_error_port { };

public:
    SchemeWrap(const SchemeWrap&) = delete;
    SchemeWrap& operator=(const SchemeWrap&) = delete;
    SchemeWrap(SchemeWrap&& o)
        : m_default_output_port { o.m_default_output_port }
        , m_default_error_port { o.m_default_error_port }
    {
        m_s7 = o.m_s7;
        o.m_s7 = nullptr;
        o.m_default_output_port = 0;
        o.m_default_error_port = 0;
    }
    SchemeWrap& operator=(SchemeWrap&& o)
    {
        m_s7 = o.m_s7;
        o.m_s7 = nullptr;
        m_default_output_port = o.m_default_output_port;
        m_default_error_port = o.m_default_error_port;
        o.m_default_output_port = 0;
        o.m_default_error_port = 0;
    }
    SchemeWrap()
        : m_s7 { ::s7_init() }
    {
        m_default_output_port = ::s7_current_output_port(m_s7);
        m_default_error_port = ::s7_current_error_port(m_s7);
        static_assert(
            requires(T t, ::s7_scheme* scm) { t.init(scm); },
            "T has no init() method");
        static_cast<T*>(this)->init(m_s7);
    }
    virtual inline ~SchemeWrap()
    {
        if (m_s7)
            ::s7_free(m_s7);
    }
    virtual inline void execute_string(const std::string& str)
    {
        ::s7_eval_c_string(m_s7, str.c_str());
    }
    virtual inline void clear_error_sink(void) const
    {
        ::s7_set_current_error_port(m_s7, m_default_error_port);
    }
    virtual inline void clear_output_sink(void) const
    {
        ::s7_set_current_output_port(m_s7, m_default_output_port);
    }
    virtual inline void set_error_port(output_port_fn_t fn) const
    {
        auto err = ::s7_open_output_function(m_s7, fn);
        ::s7_set_current_error_port(m_s7, err);
    }
    virtual inline void set_output_port(output_port_fn_t fn) const
    {
        auto out = ::s7_open_output_function(m_s7, fn);
        ::s7_set_current_output_port(m_s7, out);
    }
};
}
