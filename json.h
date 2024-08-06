#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

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
        Node() = default;
        explicit Node(Array array);
        explicit Node(Dict map);
        explicit Node(int value);
        explicit Node(double value);
        explicit Node(std::string value);
        explicit Node(bool value);

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

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;
    private:
        std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict, std::nullptr_t> data_;
    };

    class Document
    {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

} // namespace json