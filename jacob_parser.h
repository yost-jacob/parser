//
// Created by jacob on 10/29/20.
//

#ifndef PARSER_JACOB_PARSER_H
#define PARSER_JACOB_PARSER_H

#include <string>
#include <list>
#include <map>
#include <memory_resource>
#include "xml_constants.h"

class print_mem_resource : public std::pmr::memory_resource {
public:
    print_mem_resource(std::string name, std::pmr::memory_resource *upstream = std::pmr::new_delete_resource()) :
            m_name(std::move(name)), m_upstream(upstream) {}

private:
    std::string m_name;
    std::pmr::memory_resource *m_upstream;

    void *do_allocate(std::size_t bytes, std::size_t alignment) override {
        auto result = m_upstream->allocate(bytes, alignment);
        std::cout << "Allocating name : " << m_name
                  << " size : " << bytes
                  << " alignment : " << alignment
                  << " address : " << result
                  << std::endl;
        return result;
    };

    void do_deallocate(void *p, size_t bytes, std::size_t alignment) override {
        std::cout << "De-Allocating name : " << m_name
                  << " size : " << bytes
                  << " alignment : " << alignment
                  << " address : " << p
                  << std::endl;
        m_upstream->deallocate(p, bytes, alignment);
    }

    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
        return this == &other;
    }
};


// Forward declarations
template<class Ch>
class xml_node;

template<class Ch, std::size_t Buff>
class xml_document;


enum class node_type {
    document,
    prolog,
    element,
    data,
    cdata,
    comment,
    xmldecl,
    doctype,
    pi,
    elementdecl,
    attlist,
    entity,
    cond_sect,
    notation,
    unknown
};

std::ostream &operator<<(std::ostream &lhs, node_type rhs) {
    switch (rhs) {
        case node_type::document:
            return lhs << "Document Node Type";
        case node_type::prolog:
            return lhs << "Prolog Node Type";
        case node_type::element:
            return lhs << "Element Node Type";
        case node_type::data:
            return lhs << "Data Node Type";
        case node_type::cdata:
            return lhs << "CDATA Node Type";
        case node_type::comment:
            return lhs << "Comment Node Type";
        case node_type::xmldecl:
            return lhs << "XML Declaration Node Type";
        case node_type::doctype:
            return lhs << "Document Type Declaration Node Type";
        case node_type::pi:
            return lhs << "Processing Instruction Node Type";
        case node_type::elementdecl:
            return lhs << "Element Declaration Node Type";
        case node_type::attlist:
            return lhs << "Attribute List Node Type";
        case node_type::entity:
            return lhs << "Entity Declaration Node Type";
        case node_type::cond_sect:
            return lhs << "Conditional Section Node Type";
        case node_type::notation:
            return lhs << "Notation Node Type";
        case node_type::unknown:
        default:
            return lhs << "UNKNOWN Node Type";
    }
}

/*
 * Element                  <></>              name value attributes children
 * Cdata section            <![CDATA[  ]]>>    value
 * COMMENT                  <!--  -->          value
 * DECLARATION              <?xml  ?>          attributes
 * Doctype                  <!DOCTYPE >        name value children <-- this is the root of all pain
 * Processing instruction   <?   ?>            name value
 * element                   <!ELEMENT   >      name value
 * attribute list             <!ATTLIST   >   <<--  This one may be hard to implement
 * entity                     <!ENTITY  >         name value
 * Conditional Section      <![ INCLUDE [ ]]> or <![ IGNORE [ ]]> value
 */


template<typename CharT>
node_type identify_node_type(const std::basic_string_view<CharT> i) {
    switch (i[1]) {
        default:
            return node_type::element;
        case CharT('?'):
            //  parse_declaration() or
            if (xml_const_compare<CharT, true>(std::basic_string_view<CharT>(&i[2]), "xml"))
                return node_type::xmldecl;

            //  parse_pi()
            return node_type::pi;
        case CharT('!'):
            switch (i[2]) {
                case CharT('-'):
                    //  parse_comment()
                    return node_type::comment;
                case CharT('['):
                    //  parse_cdata()
                    return node_type::cdata;
                case CharT('D'):
                    //  parse_doctype
//                    return node_type::doctype;
                case CharT('\0'):
                    /*error*/
                default:
                    break;
            }
            return node_type::unknown;
        case CharT('\0'):
            /*error*/
            break;
    }
    return node_type::unknown;
}

#include "xml_traits.h"

template<typename CharT=char>
class xml_node {
    friend class xml_traits<CharT>;

    using grammar = xml_traits<CharT>;
    using string_type = std::pmr::basic_string<CharT>;
    using view_type = std::basic_string_view<CharT>;
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
protected:
    ///  utilizing std::pmr::list because the reallocations of the vector
    ///  plays holy hell with the monotonic buffer
    using node_container = std::pmr::list<xml_node<CharT>>;
//    using attr_container = std::pmr::list<xml_attribute<CharT>>;

    ///  utilizing std::map because the order does not matter and all attributes should be unique
    using attr_container = std::pmr::map<string_type, string_type>;

public:
    template<class C>

    xml_node() = delete;

    explicit xml_node(node_type n) : m_type(n), m_attr(),
                                     m_children(), m_name(), m_value() {}

    xml_node(node_type n, const allocator_type &alloc) : m_alloc(alloc), m_type(n), m_attr(alloc),
                                                         m_children(alloc), m_name(alloc), m_value(alloc) {}


    node_type type() { return m_type; }

    void clear() {
        m_attr.clear();
        m_children.clear();
    };

    template<class... Args>
    inline xml_node<CharT> &emplace_back_child(Args &&... args) {
        std::cout << "emplacing node into node" << std::endl;
        return m_children.emplace_back(std::forward<Args>(args)...);
    }

    template<class... Args>
    inline auto child_push_back(Args &&... args) {
        std::cout << "pushing node into node" << std::endl;
        return m_children.push_back(std::forward<Args>(args)...);
    }

    template<class... Args>
    inline auto insert_attribute(Args &&... args) {
        std::cout << "inserting attribute" << std::endl;
        return m_attr.insert(std::forward<Args>(args)...);
    }


    template<class... Args>
    inline auto emplace_attribute(Args &&... args) {
        std::cout << "emplacing attribute" << std::endl;
        return m_attr.emplace(std::make_pair(std::forward<Args>(args)...));
    }

    template<class... Args>
    inline auto assign_name(Args &&... args){
        return m_name.assign(std::forward<Args>(args)...);
    }

    template<class... Args>
    inline auto assign_value(Args &&... args){
        return m_value.assign(std::forward<Args>(args)...);
    }

    const allocator_type &get_alloc() {
        return m_alloc;
    }

    [[nodiscard]] const string_type &name() const { return m_name; }

    [[nodiscard]] const string_type &value() const { return m_value; }

    const node_container &children() const { return m_children; }

    const attr_container &attributes() const { return m_children; }

    xml_node<CharT> create_node(node_type n) {
        return {n, get_alloc()};
    }

protected:
    const allocator_type &m_alloc;
    const node_type m_type;

    attr_container m_attr;
    node_container m_children;
    string_type m_name;
    string_type m_value;
    bool m_attr_quot = true; //  attributes use either ' or "
};

template<std::size_t Buff>
class xml_mem_resource : public std::pmr::memory_resource {
public:
    xml_mem_resource() : m_buffer(),
                         t_alloc(m_buffer.data(), m_buffer.size()),
                         m_memresource("Monotronic", &t_alloc) {}  // misspelling is on purpose I think its funny

private:
    std::array<std::byte, Buff> m_buffer;
    std::pmr::monotonic_buffer_resource t_alloc;
    print_mem_resource m_memresource;

    void *do_allocate(std::size_t bytes, std::size_t alignment) override {
        return m_memresource.allocate(bytes, alignment);
    }

    void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) override {
        m_memresource.deallocate(p, bytes, alignment);
    }

    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
        return this == &other;
    }
};

template<typename CharT=char, std::size_t Buff = 4096>
class xml_document {
    using grammar = xml_traits<CharT>;
    using view_type = std::basic_string_view<CharT>;
    using node_container = std::pmr::list<xml_node<CharT>>;
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
public:
    xml_document() : m_memresource(std::make_optional<xml_mem_resource<Buff>>()),
                     m_alloc(&*m_memresource), // address of the object contained by the optional
                     m_prolog(node_type::prolog, m_alloc),
                     m_root(node_type::document, m_alloc) {}

    explicit xml_document(const view_type v) : m_memresource(std::make_optional<xml_mem_resource<Buff>>()),
                                               m_alloc(&*m_memresource),
                                               m_prolog(node_type::prolog, m_alloc),
                                               m_root(node_type::document, m_alloc) { parse(v); }

    explicit xml_document(const allocator_type &alloc) : m_memresource(std::nullopt),
                                                m_alloc(alloc),
                                                m_prolog(node_type::prolog, m_alloc),
                                                m_root(node_type::document, m_alloc) {}

    xml_document(const view_type v, const allocator_type &alloc) : m_memresource(std::nullopt),
                                                                   m_alloc(alloc),
                                                                   m_prolog(node_type::prolog, m_alloc),
                                                                   m_root(node_type::document, m_alloc) { parse(v); }

    std::size_t parse(const std::string &str) { return parse(view_type(str)); };

    std::size_t parse(view_type sv);

    std::size_t parse(const CharT *c, std::size_t len) { return parse(view_type(c, len)); };

    void clear() {
        m_root.clear();
        m_prolog.clear();
    }

    const xml_node<CharT> &prolog() const { return m_prolog; };

    const xml_node<CharT> &root() const { return m_prolog; };

    const allocator_type &get_alloc() {return m_alloc;}

private:
    std::optional<xml_mem_resource<Buff>> m_memresource;
    allocator_type m_alloc;
    xml_node<CharT> m_prolog;
    xml_node<CharT> m_root;
};


template<typename CharT, std::size_t Buff>
std::size_t xml_document<CharT, Buff>::parse(view_type sv) {
    std::size_t pos = 0;
    // clear any existing contents
    this->clear();

    //  Parse BOM
    {
        auto t_out = grammar::BOM(sv);
        pos += t_out;  //  there are no errors
    }

    //  Parse ProLog
    {
        auto t_out = grammar::Prolog(&m_prolog, sv.substr(pos));
        if (t_out) return static_cast<std::size_t>(t_out);
        pos += t_out;
    }

    //  Parse Children
    {
        auto t_out = grammar::Document(&m_root, sv.substr(pos));
        if (t_out) return static_cast<std::size_t>(t_out);
        pos += t_out;
    }
    return pos;
}

#endif //PARSER_JACOB_PARSER_H
