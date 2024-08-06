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

    Node::Node(Array array) : data_(std::move(array)) {}
    Node::Node(Dict map) : data_(std::move(map)) {}
    Node::Node(int value) : data_(value) {}
    Node::Node(double value) : data_(value) {}
    Node::Node(std::string value) : data_(std::move(value)) {}
    Node::Node(bool value) : data_(value) {}

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
        if (AsInt())
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
        int l_index = this->data_.index();
        int r_index = rhs.data_.index();
        return (l_index == r_index &&
                std::get<l_index>(data_) == std::get<r_index>(rhs_))
    }
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

void Print(const Document &doc, std::ostream &output)
{
    (void)&doc;
    (void)&output;

    // Реализуйте функцию самостоятельно
}

} // namespace json