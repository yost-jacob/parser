//
// Created by jacob on 11/19/20.
//

#ifndef PARSER_XML_ERROR_CATEGORY_H
#define PARSER_XML_ERROR_CATEGORY_H

#include <system_error>

enum class xml_error : int {
    no_error = 0,
    unexpected = 1,
    other_fatal = 2
};

std::ostream & operator<<(std::ostream &  lhs, xml_error rhs){
    switch (rhs){
        case xml_error::no_error:
            return lhs << "No Error";
        case xml_error::unexpected:
            return lhs << "Unexpected Char";
        case xml_error::other_fatal:
            return lhs << "Other Fatal Error";
        default:
            return lhs;
    }
}

class xml_error_category : public std::error_category {
    [[nodiscard]] const char *name() const noexcept override {
        return "xml_error_category";
    }

    [[nodiscard]] std::string message(int condition) const override {

        switch (static_cast<xml_error>(condition)) {
            case xml_error::no_error:
                return "No Error";
            case xml_error::unexpected:
                return "Unexpected Character";
            case xml_error::other_fatal:
                return "fatal error";
            default:
                break;
        }
        return "unknown error";
    }
};

#endif //PARSER_XML_ERROR_CATEGORY_H
