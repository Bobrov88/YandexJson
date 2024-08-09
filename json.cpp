#include "json.h"

using namespace std;

namespace json
{
    namespace
    {
        Node LoadNode(istream &input);

        Node LoadArray(istream &input)
        {
            Array result;

            for (char c; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadInt(istream &input)
        {
            int result = 0;
            while (isdigit(input.peek()))
            {
                result *= 10;
                result += input.get() - '0';
            }
            return Node(result);
        }

        Node LoadString(istream &input)
        {
            string line;
            getline(input, line, '"');
            return Node(move(line));
        }

        Node LoadDict(istream &input)
        {
            Dict result;

            for (char c; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            return Node(move(result));
        }

        Node LoadNode(istream &input)
        {
            char c;
            input >> c;

            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                return LoadString(input);
            }
            else
            {
                input.putback(c);
                return LoadInt(input);
            }
        }

    } // namespace

    const Node::Value &Node::GetValue() const { return data_; }

    const Array &Node::AsArray() const
    {
        if (IsArray())
            return std::get<Array>(data_);
        throw std::logic_error("Logic error");
    }
    const Dict &Node::AsMap() const
    {
        if (IsMap())
            return std::get<Dict>(data_);
        throw std::logic_error("Logic error");
    }
    int Node::AsInt() const
    {
        if (IsInt())
            return std::get<int>(data_);
        throw std::logic_error("Logic error");
    }
    const string &Node::AsString() const
    {
        if (IsString())
            return std::get<std::string>(data_);
        throw std::logic_error("Logic error");
    }
    double Node::AsDouble() const
    {
        if (IsPureDouble())
            return std::get<double>(data_);
        if (IsInt())
            return static_cast<double>(AsInt());
        throw std::logic_error("Logic error");
    }
    bool Node::AsBool() const
    {
        if (IsBool())
            return std::get<bool>(data_);
        throw std::logic_error("Logic error");
    }

    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(data_);
    }
    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(data_);
    }
    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(data_);
    }
    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(data_);
    }
    bool Node::IsDouble() const
    {
        return IsPureDouble() || IsInt();
    }
    bool Node::IsNull() const
    {
        return std::holds_alternative<std::nullptr_t>(data_);
    }
    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(data_);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(data_);
    }

    bool Node::operator==(const Node &rhs) const
    {
        return this->data_ == rhs.data_;
    }

    bool Node::operator!=(const Node &rhs) const
    {
        return !(*this == rhs);
    }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    void PrintNode(const Node &node, std::ostream &out);

    template <typename Type>
    void PrintValue(const Type &value, std::ostream &out)
    {
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream &out)
    {
        out << "null"sv;
    }

    void PrintValue(const json::Dict &dict, std::ostream &out)
    {
        out << "{"sv;
        for (const auto &[key, value] : dict)
        {
            PrintValue(key, out);
            out << ","sv;
            PrintNode(value, out);
        }
        out << "}"sv;
    }

    void PrintValue(const std::string &str, std::ostream &out)
    {
        out << "\""sv;
        PrintEscape(str, out);
        out << "\""sv;
    }

    void PrintValue(const json::Array &array, std::ostream &out)
    {
        out << "["sv;
        bool is_first = true;
        for (const auto &value : array)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                out << ","sv;
            }
            PrintNode(value, out);
        }
        out << "]"sv;
    }

    void PrintValue(const bool value, std::ostream &out)
    {
        if (value)
            out << "true";
        else
            out << "false";
    }

    void PrintNode(const Node &node, std::ostream &out)
    {
        std::visit(
            [&out](const auto &value)
            { PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document &doc, std::ostream &out)
    {
        auto &root = doc.GetRoot();
        PrintNode(root, out);
    }

    void PrintEscape(const std::string &str, std::ostream &out)
    {
        auto it = std::istreambuf_iterator<char>(str);
        auto end = std::istreambuf_iterator<char>();
        for (char c : str)
        {
            if (c == '\\')
            {
                out << "\\"sv;
            }
            switch (c)
            {
            case '\\':
                out << '\\';
                break;
                case
            }
        }
    }

} // namespace json