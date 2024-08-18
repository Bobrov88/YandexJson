#include "json.h"

using namespace std;

namespace json
{
    namespace
    {
        Node LoadNode(istream &input);

        using Number = std::variant<int, double>;

        Node LoadNull(std::istream &input)
        {
            if ('u' == input.get())
            {
                if ('l' == input.get())
                {
                    if ('l' == input.get())
                    {
                        char end = input.get();
                        if ((end > 45 && end < 92) || (end > 93 && end < 125))
                            throw ParsingError("Redundant symbols after null");
                        input.putback(end);
                        return Node(nullptr);
                    }
                }
            }
            throw ParsingError("Similiar to null value");
        }

        Node LoadBool(std::istream &input)
        {
            char c = input.get();
            if ('t' == c)
            {
                if ('r' == input.get())
                {
                    if ('u' == input.get())
                    {
                        if ('e' == input.get())
                        {
                            char end = input.get();
                            if ((end > 45 && end < 92) || (end > 93 && end < 125))
                                throw ParsingError("Redundant symbols after true");
                            input.putback(end);
                            return Node(true);
                        }
                    }
                }
            }
            else
            {
                if ('f' == c)
                {
                    if ('a' == input.get())
                    {
                        if ('l' == input.get())
                        {
                            if ('s' == input.get())
                            {
                                if ('e' == input.get())
                                {
                                    char end = input.get();
                                    if ((end > 45 && end < 92) || (end > 93 && end < 125))
                                        throw ParsingError("Redundant symbols after false");
                                    input.putback(end);
                                    return Node(false);
                                }
                            }
                        }
                    }
                }
            }
            throw ParsingError("Similiar to boolean value");
        }

        Node LoadNumber(std::istream &input)
        {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
                if (!std::isdigit(input.peek()))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadArray(istream &input)
        {
            Array result;
            char c;
            for (; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']')
            {
                throw ParsingError("Expected ']'");
            }
            return Node(move(result));
        }

        Node LoadString(istream &input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end)
                    {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char)
                    {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadDict(istream &input)
        {
            Dict result;
            char c;
            for (; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (c != '}')
            {
                throw ParsingError("Expected '}'");
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
            else if (c == 'n')
            {
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f')
            {
                input.putback(c);
                return LoadBool(input);
            }
            else if ((c > 47 && c < 58) || c == '.' || c == '+' || c == '-')
            {
                input.putback(c);
                return LoadNumber(input);
            }
            else
            {
                throw ParsingError("Unexpected symbol");
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
        if (std::holds_alternative<double>(this->data_) && std::holds_alternative<double>(rhs.data_))
        {
            return std::abs(std::get<double>(this->data_) - std::get<double>(rhs.data_)) < 0.00001;
        }
        return this->data_ == rhs.data_;
    }
    bool Node::operator!=(const Node &rhs) const
    {
        return !(*this == rhs);
    }

    Document::Document(Node root)
        : root_(move(root)) {}

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    bool Document::operator==(const Document &rhs) const
    {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document &rhs) const
    {
        return !(*this == rhs);
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
        out << "{ "sv;
        bool isSinglePair = true;
        for (const auto &[key, value] : dict)
        {
            if (!isSinglePair)
            {
                out << " , "sv;
            }
            PrintNode(key, out);
            out << " : "sv;
            PrintNode(value, out);
            isSinglePair = false;
        }
        out << " }"sv;
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
        for (char c : str)
        {
            switch (c)
            {
            case '\"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << c;
                break;
            }
        }
    }
} // namespace json