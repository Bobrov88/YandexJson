#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json
{
    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node
    {
    public:
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
        Node() = default;
        template <typename Type>
        Node(Type value) : data_(std::move(value)) {}

        const Value &GetValue() const;

        const Array &AsArray() const;
        const Dict &AsMap() const;
        int AsInt() const;
        const std::string &AsString() const;
        double AsDouble() const;
        bool AsBool() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        bool operator==(const Node &rhs) const;
        bool operator!=(const Node &rhs) const;

    private:
        Value data_;
    };

    class Document
    {
    public:
        explicit Document(Node root);
        const Node &GetRoot() const;
        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;
    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);
    void PrintEscape(const std::string &str, std::ostream &out);

} // namespace json