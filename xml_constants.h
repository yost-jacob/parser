//
// Created by jacob on 12/31/20.
//

#ifndef PARSER_XML_CONSTANTS_H
#define PARSER_XML_CONSTANTS_H

#include <string_view>
#include <string>
#include <functional>
#include "result.h"
#include "xml_error_category.h"

enum class constant {
    CharComment,
    NameStartChar,
    NameChar,
    S,
    AttValue_quot,
    AttValue_apos,
    CharPI,
    CharCDATA,
    CharData,
    digit,
    EncNameStart,
    EncName
};

std::ostream & operator<< (std::ostream & lhs, constant rhs){
    switch (rhs) {
        case constant::CharComment:
            return lhs << "CharComment";
        case constant::NameStartChar:
            return lhs << "NameStartChar";
        case constant::NameChar:
            return lhs << "NameChar";
        case constant::S:
            return lhs << "S";
        case constant::AttValue_quot:
            return lhs << "AttValue_quot";
        case constant::AttValue_apos:
            return lhs << "AttValue_apos";
        case constant::CharPI:
            return lhs << "CharPI";
        case constant::CharCDATA:
            return lhs << "CharCDATA";
        case constant::CharData:
            return lhs << "CharData";
        case constant::digit:
            return lhs << "digit";
        case constant::EncNameStart:
            return lhs << "EncNameStart";
        case constant::EncName:
            return lhs << "EncName";
    }
    return lhs;
}

enum class action {
    continue_,
    error_,
    return_
};

std::ostream & operator<<(std::ostream & lhs, action rhs){
    switch (rhs){
        case action::continue_:
            return lhs << "Action::Continue";
        case action::error_:
            return lhs << "Action::Error";
        case action::return_:
            return lhs << "Action::Return";
    }
    return lhs;
}

template<typename CharT, constant cnst>
constexpr std::basic_string_view<CharT> get_const() {
    //  **********  String Literals  **********
    //  char string literal ""
    //  wchar_t string Literal L""
    //  u8 string literal u8"" (c++20)
    //  utf-16 string literal u""
    //  utf-32 string literal U""
    //  raw string literal R"" (no escaped characters)



    if constexpr (cnst == constant::CharComment) {
        if constexpr (std::is_same_v<CharT, char>) return "-";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"-";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"-";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"-";
    }

    if constexpr (cnst == constant::CharPI) {
        if constexpr (std::is_same_v<CharT, char>) return "?";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"?";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"?";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"?";
    }

    if constexpr (cnst == constant::CharCDATA) {
        if constexpr (std::is_same_v<CharT, char>) return "]";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"]";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"]";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"]";
    }

    if constexpr (cnst == constant::CharData) {
        if constexpr (std::is_same_v<CharT, char>) return "<&]";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"<&]";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"<&]";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"<&]";
    }

    if constexpr (cnst == constant::S) {
        if constexpr (std::is_same_v<CharT, char>) return "\t\n\v\f\r ";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"\t\n\v\f\r ";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"\t\n\v\f\r ";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"\t\n\v\f\r ";
    }

    if constexpr (cnst == constant::digit) {
        if constexpr (std::is_same_v<CharT, char>) return "1234567890.";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"1234567890.";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"1234567890.";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"1234567890.";
    }

    if constexpr (cnst == constant::EncNameStart) {
        if constexpr (std::is_same_v<CharT, char>) return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }

    if constexpr (cnst == constant::EncName) {
        if constexpr (std::is_same_v<CharT, char>)
            return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-";
        if constexpr (std::is_same_v<CharT, wchar_t>)
            return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-";
        if constexpr (std::is_same_v<CharT, char16_t>)
            return u"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-";
        if constexpr (std::is_same_v<CharT, char32_t>)
            return U"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-";
    }

    if constexpr (cnst == constant::NameStartChar) {
        if constexpr (std::is_same_v<CharT, char>)
            return "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,-./0123456789;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, wchar_t>)
            return L"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,-./0123456789;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, char16_t>)
            return u"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,-./0123456789;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, char32_t>)
            return U"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,-./0123456789;<=>?@[\134]^`{|}~\177";
    }

    if constexpr (cnst == constant::NameChar) {
        if constexpr (std::is_same_v<CharT, char>)
            return "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,/;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, wchar_t>)
            return L"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,/;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, char16_t>)
            return u"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,/;<=>?@[\134]^`{|}~\177";
        if constexpr (std::is_same_v<CharT, char32_t>)
            return U"\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\40!\42#$%&\47()*+,/;<=>?@[\134]^`{|}~\177";
    }

    if constexpr (cnst == constant::AttValue_quot) {
        if constexpr (std::is_same_v<CharT, char>) return "<&\"";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"<&\"";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"<&\"";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"<&\"";
    }

    if constexpr (cnst == constant::AttValue_apos) {
        if constexpr (std::is_same_v<CharT, char>) return "<&\'";
        if constexpr (std::is_same_v<CharT, wchar_t>) return L"<&\'";
        if constexpr (std::is_same_v<CharT, char16_t>) return u"<&\'";
        if constexpr (std::is_same_v<CharT, char32_t>) return U"<&\'";
    }
}

template<typename CharT, bool FO, constant cnst>
struct xml_constant {
    using view_type = std::basic_string_view<CharT>;
    static constexpr std::basic_string_view<CharT> s = get_const<CharT, cnst>();

    static constexpr bool contains(const CharT c) noexcept {
        if constexpr (FO) {
            if (s.find(c) != std::basic_string_view<CharT>::npos) return false;
        } else {
            if (s.find(c) == std::basic_string_view<CharT>::npos) return false;
        }
        return true;
    }

    static result<std::size_t, xml_error> skip(const std::basic_string_view<CharT> sv,
                                               std::function<action(const view_type)> p = [](
                                                       const view_type) { return action::return_; }) {
        std::size_t out;

        if constexpr (FO) {
            out = sv.find_first_of(s);
        } else {
            out = sv.find_first_not_of(s);
        }

        if (out == std::basic_string_view<CharT>::npos) {
            return {std::basic_string_view<CharT>::npos, xml_error::unexpected};
        }

        auto t = p(&sv[out]);
        switch (t) {
            case action::error_:
                return {std::basic_string_view<CharT>::npos, xml_error::unexpected};
            case action::continue_:
                ++out;
                out += skip(sv.substr(out), p);
                [[fallthrough]];
            case action::return_:
            default:
                return {out, std::error_condition()};
        }
    }

};

template<typename CharT, bool INSENS = false>
bool xml_const_compare(std::basic_string_view<CharT> sv, const char *st) noexcept {
    const std::string_view cnst{st};
    if (cnst.length() > sv.length()) return false;  //  stringview [] operator does not check bounds
    // todo use an alogrothim probably mismatch

    for (std::size_t i = 0; i < cnst.length(); ++i) {
        if constexpr (!INSENS) {
            if (CharT(cnst[i]) != sv[i]) return false;
        } else {
            if ((CharT(tolower(cnst[i])) != sv[i]) && (CharT(toupper(cnst[i])) != sv[i])) return false;
        }
    }
    return true;
}

#endif //PARSER_XML_CONSTANTS_H
