//
// Created by jacob on 12/31/20.
//

#ifndef PARSER_RESULT_H
#define PARSER_RESULT_H

#include "xml_error_category.h"

template<typename T, typename U = int>
struct result {
    // todo add assertion U is convertible to int
    result() = default;

    explicit result(T s) : m_result(s), m_err() {}

    result(T s, std::error_condition e) : m_result(s), m_err(e) {}

    result(T s, U xe) : m_result(s), m_err(static_cast<int>(xe), xml_error_category()) {}

    T m_result;
    std::error_condition m_err;

    explicit operator bool() const noexcept {
        return static_cast<bool>(m_err);
    }

    operator T() const noexcept {
        return m_result;
    }
};
#endif //PARSER_RESULT_H
