#pragma once
#include "s7.h"

namespace scheme {
template <class T>
class SchemeWrap {
    ::s7_scheme* m_s7 = nullptr;

public:
    SchemeWrap(const SchemeWrap&) = delete;
    SchemeWrap& operator=(const SchemeWrap&) = delete;
    SchemeWrap(SchemeWrap&& o)
    {
        m_s7 = o.m_s7;
        o.m_s7 = nullptr;
    }
    SchemeWrap& operator=(SchemeWrap&& o)
    {
        m_s7 = o.m_s7;
        o.m_s7 = nullptr;
    }
    SchemeWrap()
        : m_s7 { ::s7_init() }
    {
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
};
}
