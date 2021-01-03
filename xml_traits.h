//
// Created by jacob on 11/1/20.
//

#ifndef PARSER_XML_TRAITS_H
#define PARSER_XML_TRAITS_H

#include <string_view>
#include <locale>
#include "xml_error_category.h"
#include "result.h"
#include "xml_constants.h"

//  Note on style:  Somewhere I was watching a CppCon video, probably Kate Gregory, who indicated that out parameters
//                  should be passed by pointer to differentiate them from other variables, and make it explicit that
//                  we are changing data that the function doesn't own.  Or something like that, or someone like that.

template<typename CharT>
class xml_traits {
private:
    using xml_result = result<std::size_t, xml_error>;

public:
    using string_type = std::pmr::basic_string<CharT>;
    using view_type = std::basic_string_view<CharT>;
    using pair_type = std::pair<string_type, string_type>;

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();


    //  ************ parse functions ********************

    //     NameStartChar    C
    using NameStartChar = xml_constant<CharT, true, constant::NameStartChar>;

    //     NameChar         C
    using NameChar = xml_constant<CharT, true, constant::NameChar>;

    //  1  Name
    static xml_result
    Name(const view_type sv) noexcept {
        if (!NameStartChar::contains(sv.front())) { return {npos, xml_error::unexpected}; }
        auto pos = NameChar::skip(sv);
        return {pos, xml_error::no_error};
    }

    static xml_result
    Name(string_type * st, const view_type sv) noexcept {
        if (!NameStartChar::contains(sv.front())) { return {npos, xml_error::unexpected}; }
        auto pos = NameChar::skip(sv);
        st->assign(&sv[0], pos);
        return {pos, xml_error::no_error};
    }

    //  2  S                C
    using S_ = xml_constant<CharT, false, constant::S>;

    static inline xml_result
    S(const view_type sv) noexcept {
        return {S_::skip(sv), std::error_condition()};
    }

    //  3  Eq
    static xml_result
    Eq(const view_type sv) noexcept {
        std::size_t pos{0};
        if (S_::contains(sv.front())) pos = S(sv);
        if (sv[pos] == CharT('=')) ++pos; else { return {npos, xml_error::unexpected}; }
        if (S_::contains(sv[pos])) pos += S(sv.substr(pos));
        return {pos, std::error_condition()};
    }

    //  4  CharRef
    static xml_error
    CharRef(string_type * st, const view_type sv) noexcept {
        if (sv.front() == CharT('x') || sv.front() == CharT('X')) {
            ///  Hexidecial
            unsigned long code = 0;
            auto result = std::from_chars(&sv[1], sv.end(), code, 16);
            if (result.ptr == sv.end()) insert_coded_character(st, code); else {return xml_error::other_fatal;}
            std::cout << code << std::endl;
        } else {
            ///  Decimal
            unsigned long code = 0;
            auto result = std::from_chars(&sv[0], sv.end(), code, 10);
            if (result.ptr == sv.end()) insert_coded_character(st, code); else {return xml_error::other_fatal;}
            std::cout << code << std::endl;
        }
        return xml_error::no_error;
    }

    //  5  EntityRef
    static void
    EntityRef(string_type *st, const view_type sv) noexcept {
        std::cout << "entity ref : " << sv << std::endl;
        switch (sv[1]) {
            // &amp; &apos;
            case CharT('a'):
                if (xml_const_compare(sv, "&amp;")) {
                    *st += CharT('&');
                }
                if (xml_const_compare(sv, "&apos;")) {
                    *st += CharT('\'');
                }
                break;

                // &quot;
            case CharT('q'):
                if (xml_const_compare(sv, "&quot;")) {
                    *st += CharT('\"');
                }
                break;

                // &gt;
            case CharT('g'):
                if (xml_const_compare(sv, "&gt;")) {
                    *st += CharT('>');
                }
                break;

                // &lt;
            case CharT('l'):
                if (xml_const_compare(sv, "&lt;")) {
                    *st += CharT('<');
                }
                break;

                // Something else
            default:
                //  & Name ;
                st->append(sv);
                break;
        }
    }

//  6  Reference
    static xml_result
    Reference(string_type *st, const view_type sv) noexcept {
        std::size_t pos = sv.find(CharT(';'));
        if (pos == sv.npos) {/*error*/}
        if (sv[1] == CharT('#')) {
            auto t_out = CharRef(st, sv.substr(2, pos - 2));
            if (t_out != xml_error::no_error) return {npos, t_out};
        } else {
            EntityRef(st, sv.substr(0, pos + 1));
        }
        return {++pos, std::error_condition()};
    }

    using AttValue_apos = xml_constant<CharT, true, constant::AttValue_apos>;
    using AttValue_quot = xml_constant<CharT, true, constant::AttValue_quot>;

//  7  AttValue
    static xml_result
    AttValue(string_type &st, const view_type sv) noexcept {
        if ((sv[0] != CharT('\"')) && (sv[0] != CharT('\''))) { return {npos, xml_error::unexpected}; }
        const CharT delim = sv.front();
        std::size_t start = 1;
        std::size_t end = start;
        while (sv[end] != delim) {  // todo goal no raw loops
            if (delim == CharT('\"')) {
                end += AttValue_quot::skip(sv.substr(end));
            } else {
                end += AttValue_apos::skip(sv.substr(end));
            }
            switch (sv[end]) {
                case CharT('\"'):
                case CharT('\''):
                    if (sv[end] == delim) st.append(&sv[start], end - start);
                    break;

                case CharT('&'):
                    st.append(&sv[start], end - start);
                    end += Reference(&st, sv.substr(end));
                    start = end;
                    break;

                case CharT('<'):
                default:
                    return {npos, xml_error::unexpected};
            }
        }
        ++end;
        std::cout << "Attribute Value : " << st << std::endl;
        return {end, std::error_condition()};
    }

//  8  attribute
    static xml_result
    Attribute(pair_type *pair, bool *quot, const view_type sv) noexcept {
        std::size_t pos = 0;

        //  skip whitespace
        pos += S(sv);

        //  Parse attrib name
        pos += Name(&pair->first, sv.substr(pos));
        std::cout << "attribute name : " << pair->first << std::endl;

        //  Parse Eq
        pos += Eq(sv.substr(pos));

        // parse attrib value
        if (sv[pos] == CharT('\'')) {
            *quot = false;
            pos += AttValue(pair->second, sv.substr(pos));
        } else if (sv[pos] == CharT('\"')) {
            *quot = true;
            pos += AttValue(pair->second, sv.substr(pos));
        } else { return {npos, xml_error::unexpected}; }

        return {pos, std::error_condition()};
    }

//  9  stag-emptytag
    static std::pair<bool, xml_result>
    Stag_Emptytag(xml_node<CharT> *node, const view_type sv) noexcept {
        //  Stag:  '<' Name (S Attribute)* S? '>'
        //  Attribute:  Name eq attvalue
        std::size_t pos = 0;
        bool empty_tag = false;

        //  skip <
        if (sv[pos] == CharT('<')) ++pos; else { return {true, {npos, xml_error::unexpected}}; }

        //  skip whitespace
        pos += S_::skip(sv.substr(pos));

        //  extract name
        auto end = pos + Name(&node->m_name, sv.substr(pos));
        std::cout << "stag name : " << node->name() << " Length : " << node->m_name.length() << std::endl;

        xml_parser_attribute_parse:
        //  skip whitespace
        end += S_::skip(sv.substr(end));

        // parse tag closing
        switch (sv[end]) {
            case CharT('>'):
                ++end;
                break;
            case CharT('/'):
                empty_tag = true;
                if (sv[++end] == CharT('>')) ++end;
                break;
            default: {
                pair_type attr = std::make_pair(string_type(node->get_alloc()), string_type(node->get_alloc()));
                end += Attribute(&attr, &(node->m_attr_quot), sv.substr(end));
                std::cout << std::boolalpha << "Attrib quot : " << node->m_attr_quot << std::endl;
                node->insert_attribute(std::make_pair(std::move(attr.first), std::move(attr.second)));
            }
                goto xml_parser_attribute_parse;// todo find a better way to do this.  Either a recursive function or a loop
        }
        return {empty_tag, {end, xml_error::no_error}};
    }


//  10 etag
    static inline xml_result
    Etag(const string_type &st, const view_type sv) noexcept {
        std::size_t pos = 0;

        //  check for </
        if ((sv[pos] == CharT('<')) && (sv[pos + 1] == CharT('/'))) {
            pos += 2;
        } else { return {npos, xml_error::unexpected}; }

        //  skip whitespace
        auto s = pos + S_::skip(sv.substr(pos));

        //  get name
        pos = s + Name(sv.substr(s));
        std::cout << "end tag name : " << sv.substr(s, pos - s) << " : " << st << std::endl;
        if (sv.substr(s, pos - s) != st) { return {npos, xml_error::unexpected}; }

        //  skip whitespace
        pos += S_::skip(sv.substr(pos));

        //  parse tag closure
        if (sv[pos] == CharT('>')) ++pos; else { return {npos, xml_error::unexpected}; }

        return {pos, std::error_condition()};
    }

//  11 Char             C
    using Char_Comment_ = xml_constant<CharT, true, constant::CharComment>;

    static xml_result
    Char_Comment(const view_type sv) noexcept {
        return Char_Comment_::skip(sv, [](const view_type c) {
            if (c[0] == CharT('-')) {
                if (c[1] == CharT('-')) {
                    if (c[2] == CharT('>')) return action::return_; else return action::error_;
                } else {
                    return action::continue_;
                }
            }
            return action::return_;
        });
    }

//  12 Comment
    static xml_result
    Comment(xml_node<CharT> *node, const view_type sv) noexcept {
        std::size_t pos = 0;
        string_type output{node->get_alloc()};

        //  Verify comment start
        //  the first 3 characters validated by the node_type check
        if (sv[3] != CharT('-')) {
            return {npos, xml_error::unexpected};
        } else {
            pos += 4;
        }

        auto cnt = Char_Comment(sv.substr(pos));
        if (cnt) { return {npos, xml_error::unexpected}; }
        else {
            handle_CharData(node, output.assign(&sv[pos], cnt));
            pos += cnt;
            pos += 3;
        }

        return {pos, std::error_condition()};
    }

    using Char_PI_ = xml_constant<CharT, true, constant::CharPI>;

    static xml_result
    Char_PI(const view_type sv) noexcept {
        return Char_PI_::skip(sv,
                              [](const view_type c) {
                                  if (c[0] == CharT('?')) {
                                      if (c[1] == CharT('>')) return action::return_;
                                      else return action::continue_;
                                  }
                                  return action::return_;
                              });
    }

//  13 PTTarget
    static inline xml_result
    PITarget(string_type &st, const view_type sv) noexcept {
        return Name(&st, std::forward<const view_type>(sv));
    }

//  14 PI
    static xml_result
    PI(xml_node<CharT> *node, const view_type sv) noexcept {
        // '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
        std::size_t pos = 2;

        //  get PI Target
        {
            auto t_out = PITarget(node->m_name, &sv[pos]);
            if (t_out) { return {npos, xml_error::unexpected}; }
            pos += t_out;
        }

        // skip white space
        {
            auto t_out = S(sv.substr(pos));
            if (t_out) { return {npos, xml_error::unexpected}; }
            pos += t_out;
        }

        //  get target
        {
            auto t_out = Char_PI(sv.substr(pos));
            if (t_out) { return {npos, xml_error::unexpected}; }
            node->m_value.assign(&sv[pos], t_out);
            pos += t_out;
        }
        std::cout << "PITarget : " << node->m_name << " -- Data : " << node->m_value << std::endl;
        pos += 2;
        return {pos, std::error_condition()};
    }

//    struct Char_CDATA;
    using Char_CDATA_ = xml_constant<CharT, true, constant::CharCDATA>;

    static xml_result
    Char_CDATA(const view_type sv) noexcept {
        return Char_CDATA_::skip(sv, [](const view_type c) {
            if (c[0] == CharT(']')) {
                if (xml_const_compare(c, "]]>")) return action::return_; else return action::continue_;
            }
            return action::error_;
        });
    }

//  15 CDEnd
    static xml_result
    CDEnd(const view_type sv) noexcept {
        // ]]>
        if (!(sv[0] == CharT(']')
              && sv[1] == CharT(']')
              && sv[2] == CharT('>'))
                ) { return {npos, xml_error::unexpected}; }
        return {3, std::error_condition()};
    }

//  16 CData
    static xml_result
    CData(xml_node<CharT> *node, const view_type sv) noexcept {
        auto pos = Char_CDATA(sv);

        if (pos) { return {npos, xml_error::unexpected}; }
        node->m_value.assign(&sv.front(), pos);
        std::cout << "CDATA : " << sv.substr(0, pos) << std::endl;
        return {static_cast<std::size_t>(pos), std::error_condition()};
    }

//  17 CDStart
    static xml_result
    CDStart(const view_type sv) noexcept {
        // <![CDATA[
        if (!xml_const_compare<CharT>(sv, "<![CDATA[")) return {npos, xml_error::unexpected};
        return {9, std::error_condition()};
    }

//  18 CDSect
    static xml_result
    CDSect(xml_node<CharT> *node, const view_type sv) noexcept {
        //  CDSect	   ::=   	CDStart CData CDEnd
        std::size_t pos{0};
        //  skip start
        auto t_out = CDStart(sv);
        if (t_out) { return {npos, xml_error::unexpected}; }
        pos += t_out;

        //  skip data and insert into node
        t_out = CData(node, sv.substr(pos));
        if (t_out) { return {npos, xml_error::unexpected}; }
        pos += t_out;

        //  skip pos
        t_out = CDEnd(sv.substr(pos));
        if (t_out) { return {npos, xml_error::unexpected}; }
        pos += t_out;
        std::cout << "cdata len : " << pos << std::endl;
        return {pos, std::error_condition()};
    }

//  19 CharData         C
//    struct CharData;
    using CharData_ = xml_constant<CharT, true, constant::CharData>;

    static xml_result
    CharData(const view_type sv) noexcept {
        return CharData_::skip(sv, [](const view_type c) {
            if (c[0] == CharT(']')) {
                if (!xml_const_compare(c, "]]>")) return action::continue_; else return action::return_;
            }
            return action::return_;
        });
    }

//  20 content
    static xml_result
    content(xml_node<CharT> *node, const view_type sv) noexcept {
        //  CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
        std::size_t start = 0, end = 0;
        string_type output{node->get_alloc()};

        while (!((sv[end] == CharT('<')) && (sv[end + 1] == CharT('/')))) {  // todo goal no raw loops
            end += CharData(sv.substr(end));
            if (end == sv.npos) { return {npos, xml_error::unexpected}; }

            switch (sv[end]) {
                case CharT('?'):
                    if (end > start) output.append(&sv[start], end - start);
                    {
                        auto t_out = Reference(&output, sv.substr(end));
                        if (t_out == npos) { return {npos, xml_error::unexpected}; }
                        end += t_out;
                    }
                    start = end;
                    break;

                case CharT('<'): {
                    if (sv[end + 1] == CharT('/')) break;
                    if (end > start) output.append(&sv[start], end - start);
                    if (!output.empty()) handle_CharData(node, output);
                    {
                        node_type nt = identify_node_type<CharT>(sv.substr(end));
                        xml_node<CharT> ref = node->create_node(nt);
                        auto t_out = parse_node(&ref, sv.substr(end));
                        if (t_out) { return {npos, xml_error::unexpected}; }
                        node->child_push_back(std::move(ref));
                        end += t_out;
                    }
                    start = end;
                    break;
                }

                case CharT('\0'):
                default: {
                    return {npos, xml_error::unexpected};
                }
            }
        }
        if (end > start) output.append(&sv[start], end - start);
        if (!output.empty()) handle_CharData(node, output);
        return {end, std::error_condition()};
    }

//  21 Element
    static xml_result
    Element(xml_node<CharT> *node, const view_type sv) noexcept {
        std::size_t pos = 0;

        //  parse start tag
        auto out = Stag_Emptytag(node, sv);
        if (out.second) { return {npos, xml_error::unexpected}; }
        pos += out.second;
        std::cout << "start tag: " << sv.substr(0, pos) << " : " << sv[pos] << out.first << std::endl;

        if (!out.first) {  //  If the Tag is not an EmptyTag then parse contents and Etag
            //  parse content
            auto cnt = content(node, sv.substr(pos));
            if (cnt) { return {npos, xml_error::unexpected}; }
            pos += cnt;

            //  parse end tag
            cnt = Etag(node->m_name, sv.substr(pos));
            if (cnt) { return {npos, xml_error::unexpected}; }
            pos += cnt;
        }

        return {pos, xml_error::no_error};
    }

//  22 cp
//  23 seq
//  24 choice
//  25 children
//  26 Mixed
//  27 contentspec
//  28 elementdecl
//  29 defaultdecl
//  30 Nmtoken
//  31 Enumeration
//  32 NotationType
//  33 EnumeratedType
//  34 TokenizedType
//  35 StringType
//  36 attType
//  37 attlistdecl
//  38 markupdecl
//  39 intSubSet
//  40 PubidLiteral
//  41 SystemLiteral
//  42 ExternalID
//  todo the root of all pain
//  43 doctypedecl

//  SDDecl
    static xml_result
    SDDecl(xml_node<CharT> *node, const view_type sv) noexcept {
        // parse attribute
        pair_type attr = std::make_pair(string_type(node->get_alloc()), string_type(node->get_alloc()));
        std::size_t pos = Attribute(&attr, &(node->m_attr_quot), sv);
        // verify name
        if (!xml_const_compare(view_type(attr.first), "standalone")) return {npos, xml_error::unexpected};

        // verify value
        if ((!xml_const_compare(view_type(attr.second), "yes")) &&
            (!xml_const_compare(view_type(attr.second), "no")))
            return {npos, xml_error::unexpected};

        node->insert_attribute(std::make_pair(std::move(attr.first), std::move(attr.second)));
        return {pos, std::error_condition()};
    }

    static bool
    EncName(const view_type sv) noexcept {
        auto i = sv.begin();
        if (!xml_constant<CharT, false, constant::EncNameStart>::contains(*i)) return false;
        ++i;
        auto pos = xml_constant<CharT, false, constant::EncName>::skip(view_type(&*i));
        if (pos != npos)
            return false; // this is hinky but if its npos then all the characters until the end are EncName
        return true;
    }

//  EncodingDecl
    static xml_result
    EncodingDecl(xml_node<CharT> *node, const view_type sv) noexcept {
        // parse attribute
        pair_type attr = std::make_pair(string_type(node->get_alloc()), string_type(node->get_alloc()));
        std::size_t pos = Attribute(&attr, &(node->m_attr_quot), sv);
        // verify name
        if (!xml_const_compare(view_type(attr.first), "encoding")) return {npos, xml_error::unexpected};

        // verify value
        if (!EncName(attr.second)) return {npos, xml_error::unexpected};

        node->insert_attribute(std::make_pair(std::move(attr.first), std::move(attr.second)));
        return {pos, std::error_condition()};
    }

//  44 VersionInfo
    static xml_result
    VersionInfo(xml_node<CharT> *node, const view_type sv) noexcept {
        // parse attribute
        pair_type attr = std::make_pair(string_type(node->get_alloc()), string_type(node->get_alloc()));
        std::size_t pos = Attribute(&attr, &(node->m_attr_quot), sv);
        // verify name
        if (!xml_const_compare(view_type(attr.first), "version")) return {npos, xml_error::unexpected};

        // verify value
        {
            auto t_out = xml_constant<CharT, false, constant::digit>::skip(view_type(attr.second));
            if (t_out != npos) return {npos, xml_error::unexpected};
        }
        node->insert_attribute(std::make_pair(std::move(attr.first), std::move(attr.second)));
        return {pos, std::error_condition()};
    }

//  45 Misc
    static xml_result
    Misc(xml_node<CharT> *node, const view_type sv) noexcept {
        std::size_t pos = 0;
        for (;;) {  // todo can i replace this with an alogorithm?
            pos += S(sv.substr(pos));
            if (sv[pos] != CharT('<')) break;
            auto nt = identify_node_type<CharT>(sv.substr(pos));
            if (nt != node_type::comment && nt != node_type::pi) break;
            {
                xml_node<CharT> ref = node->create_node(nt);
                auto t_out = parse_node(&ref, sv.substr(pos));
                if (t_out != npos) {
                    node->child_push_back(std::move(ref));
                    pos += t_out;
                    std::cout << std::endl;
                } else return {npos, xml_error::unexpected};
            }
        }
        return {pos, std::error_condition()};
    }

//  46 XMLDecl
    static xml_result
    XMLDecl(xml_node<CharT> *node, const view_type sv) noexcept {
        std::size_t pos = 5;

        // skip whitespace
        pos += S(sv.substr(pos));

        // parse versioninfo (attribute)
        {
            if (sv[pos] != CharT('v')) return {npos, xml_error::unexpected};
            auto t_out = VersionInfo(node, sv.substr(pos));
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }

        // parse EncodingDecl if present (attribute)
        pos += S(sv.substr(pos));
        if (sv[pos] == CharT('e')) {
            auto t_out = EncodingDecl(node, sv.substr(pos));
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }

        // parse SDDecl if present (attribute)
        pos += S(sv.substr(pos));
        if (sv[pos] == CharT('s')) {
            auto t_out = SDDecl(node, sv.substr(pos));
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }

        // skip whitespace
        pos += S(sv.substr(pos));

        // parse closure
        if (xml_const_compare(sv.substr(pos), "?>")) pos += 2;
        else
            return {npos, xml_error::unexpected};
        return {pos, std::error_condition()};
    }

//  47 ProLog
    static xml_result
    Prolog(xml_node<CharT> *node, const view_type sv) noexcept {
        std::size_t pos = 0;
        // skip whitespace
        {
            auto t_out = S(sv);
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }

        // parse xml declaration if present
        {
            if (sv[pos] != CharT('<')) return {npos, xml_error::unexpected};
            auto nt = identify_node_type<CharT>(sv.substr(pos));
            if (nt == node_type::xmldecl) {
                xml_node<CharT> ref = node->create_node(nt);
                auto t_out = XMLDecl(&ref, sv.substr(pos));
                if (!t_out) {
                    node->child_push_back(std::move(ref));
                    pos += t_out;
                } else { return {npos, xml_error::unexpected}; }
            }
        }
        //  parse misc if present
        {
            auto t_out = Misc(node, sv.substr(pos));
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }

        // parse doc declaration
        {
            // todo  root of all pain
        }

        // parse misc if present
        {
            auto t_out = Misc(node, sv.substr(pos));
            if (t_out) return {npos, xml_error::unexpected};
            pos += t_out;
        }
        std::cout << "exiting prolog" << std::endl << std::endl;
        return {pos, std::error_condition()};
    }

//  48 Document
    static xml_result
    Document(xml_node<CharT> *node, const view_type sv) noexcept {
//        auto i = sv.begin();
        std::size_t pos = 0;
        while (pos < sv.length()) {  // todo goal no raw loops
            //  skip whitespace
            pos += S(sv.substr(pos));

            //  Parse and emplace_back node onto list
            if (sv[pos] == CharT('<')) {
                // get type
                node_type nt = identify_node_type<CharT>(sv.substr(pos));
                xml_node<CharT> ref = node->create_node(nt);
                std::cout << nt << std::endl;
                xml_result t_out = parse_node(&ref, sv.substr(pos));
                if (!t_out) {
                    node->child_push_back(std::move(ref));
                    pos += static_cast<std::size_t>(t_out);
                } else {

                    return {npos, xml_error::unexpected}; }

                std::cout <<std::endl << "main loop : " << sv.substr(pos) << std::endl;
            } else { return {npos, xml_error::unexpected}; }
        }
        std::cout << "total length : " << std::distance(sv.begin(), sv.end()) << std::endl;
        return {static_cast<unsigned long int>(std::distance(sv.begin(), sv.end())), std::error_condition()};
    }

//  49 BOM
    static xml_result
    BOM(view_type sv) noexcept;

private:

    static void
    insert_coded_character(string_type * st, unsigned long code) noexcept {
        std::array<CharT, 4> text;
        if (code < 0x80)    // 1 byte sequence
        {
            text[0] = static_cast<unsigned char>(code);
            st->append(&text[0], 1);
        } else if (code < 0x800)  // 2 byte sequence
        {
            text[1] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[0] = static_cast<unsigned char>(code | 0xC0u);
            st->append(&text[0], 2);
        } else if (code < 0x10000)    // 3 byte sequence
        {
            text[2] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[1] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[0] = static_cast<unsigned char>(code | 0xE0u);
            st->append(&text[0], 3);
        } else if (code < 0x110000)   // 4 byte sequence
        {
            text[3] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[2] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[1] = static_cast<unsigned char>((code | 0x80u) & 0xBFu);
            code >>= 6u;
            text[0] = static_cast<unsigned char>(code | 0xF0u);
            st->append(&text[0], 4);
        } else    // Invalid, only codes up to 0x10FFFF are allowed in Unicode
        {
            /*error*/
            // todo figure out error handling here
        }

    }

    static void
    handle_CharData(xml_node<CharT> *node, string_type &st) noexcept {
        if (node->m_value.empty()) node->assign_value(st);

        xml_node<CharT> ref = node->create_node(node_type::data);
        std::cout << "Element Value : " << st << std::endl;
        ref.assign_value(std::move(st));
        node->child_push_back(std::move(ref));
        st.clear();
    }

    static xml_result
    parse_node(xml_node<CharT> *node, const view_type sv) noexcept {
        std::cout << "type : " << node->type() << std::endl;
        switch (node->type()) {
            case node_type::element:
                return Element(node, sv);

            case node_type::comment:
                return Comment(node, sv);

            case node_type::cdata:
                return CDSect(node, sv);

            case node_type::pi:
                return PI(node, sv);

            case node_type::xmldecl:
                return XMLDecl(node, sv);
            default:
                break;
        }
        return {npos, xml_error::other_fatal};
    }
};

template<>
result<std::size_t, xml_error> xml_traits<char>::BOM(const view_type sv) noexcept {
    if (static_cast<unsigned char>(sv[0]) == 0xEF
        && static_cast<unsigned char>(sv[1]) == 0xBB
        && static_cast<unsigned char>(sv[2]) == 0xBF)
        return {3, std::error_condition()};
    return {0, std::error_condition()};
}

template<>
result<std::size_t, xml_error> xml_traits<wchar_t>::BOM(const view_type sv) noexcept {
    if (sv[0] == 0xfeff || sv[0] == 0xfffe) return {1, std::error_condition()};
    return {0, std::error_condition()};
}

template<>
result<std::size_t, xml_error> xml_traits<char16_t>::BOM(const view_type sv) noexcept {
    if (sv[0] == 0xfeff || sv[0] == 0xfffe) return {1, std::error_condition()};
    return {0, std::error_condition()};
}

template<>
result<std::size_t, xml_error> xml_traits<char32_t>::BOM(const view_type sv) noexcept {
    if (sv[0] == 0x0000feff || sv[0] == 0xfffe0000) return {1, std::error_condition()};
    return {0, std::error_condition()};
}

#endif //PARSER_XML_TRAITS_H
