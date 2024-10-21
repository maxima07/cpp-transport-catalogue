#include "json.h"

using namespace std;

namespace json {

namespace {

using Number = std::variant<int, double>;

Node LoadNode(istream& input);

std::string InToString(std::istream& input){
    std::string result;

    while (std::isalpha(input.peek())){
        result.push_back(static_cast<char>(input.get()));
    }

    return result;
}

json::Node LoadNull(std::istream& input) {
    std::string parse_type = InToString(input);

    if (parse_type == "null"sv){
        return Node(nullptr);
    } else {
        throw ParsingError("The null string could not be parsed"s);
    }
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
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
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadBool(istream& input) {
    std::string parse_type = InToString(input);
    
    if(parse_type == "true"sv){
        return Node(true); 
    } else if (parse_type == "false"sv) {
        return Node(false); 
    } else {
        throw ParsingError ("The bool type could not be parsed"s);
    }
}

Node LoadArray(istream& input) {
    Array result;

    if(input.peek() < 0){
        throw ParsingError("The array could not be parsed"s);
    }

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(std::move(result));
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":

Node LoadDict(istream& input) {
    Dict result;

    if(input.peek() < 0){
        throw ParsingError("The dictionary could not be parsed"s);
    }

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    
    switch (c) {
    case 'n':
        input.putback(c);
        return LoadNull(input);
        break;
    case '"':
        return LoadString(input);
        break;
    case 't' : case 'f' :
        input.putback(c);
        return LoadBool(input);
        break;    
    case '[':
        return LoadArray(input);
        break;
    case '{':
        return LoadDict(input);
        break;    
    default:
        input.putback(c);
        return LoadNumber(input);
        break;
    }
}

}  // namespace

// -----------Node---------------

Node::Node(std::nullptr_t) : value_(nullptr) {}

Node::Node(int value) : value_(value) {}

Node::Node(double value) : value_(value) {}

Node::Node(std::string value) : value_(std::move(value)) {}

Node::Node(bool value) : value_(value){}

Node::Node(Array value) : value_(std::move(value)) {}

Node::Node(Dict value): value_(std::move(value)) {}

// проверка значения внутри variant
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

// возвращает значение из variant
int Node::AsInt() const {
    if(!IsInt()){
        throw std::logic_error("the type is not int"s);
    }
    return std::get<int>(value_);
}

double Node::AsDouble() const{
    if(!IsDouble()) {
        throw std::logic_error("the type is not a double or int"s);  
    } else if (IsInt()){
        return static_cast<double>(std::get<int>(value_));
    }

    return static_cast<double>(std::get<double>(value_)); 
}

const string& Node::AsString() const {
    if(!IsString()){
        throw std::logic_error("the type is not a string"s);
    }
    return std::get<std::string>(value_);
}

bool Node::AsBool() const {
    if(!IsBool()){
        throw std::logic_error("the type is not a string"s);
    }
    return std::get<bool>(value_);
}

const Array& Node::AsArray() const {
    if(!IsArray()){
        throw std::logic_error("the type is not an Array"s); 
    }
    return std::get<Array>(value_);   
}

const Dict& Node::AsMap() const {
    if(!IsMap()){
        throw std::logic_error("the type is not a Dict"s);
    }
    return std::get<Dict>(value_);      
}

const Node::Value& Node::GetValue() const {
    return value_;
}

bool operator== (const Node& lhs, const Node& rhs){
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!= (const Node& lhs, const Node& rhs){
    return !(lhs.GetValue() == rhs.GetValue());
}

// -----------Document---------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator== (const Document& lhs, const Document& rhs){
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!= (const Document& lhs, const Document& rhs){
    return !(lhs.GetRoot() == rhs.GetRoot());
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// -----------Print---------------

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// Возвращает новый контекст вывода с увеличенным смещением
PrintContext PrintContext::Indented() const {
    return {out, indent_step, indent_step + indent};
}

void PrintValue (std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue (std::string value, const PrintContext& ctx) {
    ctx.out << "\""sv;
    for(const char c : value){
        switch(c){
            case '\n' :
                ctx.out << "\\n"sv;
                break;
            case '\r' :
                ctx.out << "\\r"sv;
                break;
            case '\"' :
                ctx.out << "\\\""sv;
                break;
            case '\t' :
                ctx.out << "\\t"sv;
                break;
            case '\\' :
                ctx.out << "\\\\"sv;
                break;
            default :
                ctx.out << c;
                break;
        }
    }
    ctx.out << "\""sv;
}

void PrintValue (bool value, const PrintContext& ctx) {
    if(value){
        ctx.out << "true"sv;
    } else {
        ctx.out << "false"sv;
    }
}

void PrintValue (Array values, const PrintContext& ctx) {
    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    ctx.out << "[\n"sv;

    for(const auto& value : values){
        if(is_first){  
            is_first = false;
        } else {
            ctx.out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(value, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "]"sv;
}

void PrintValue (Dict values, const PrintContext& ctx) {
    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    ctx.out << "{\n"sv;
    
    for(const auto& [key, value] : values){
        
        if(is_first){
            is_first = false;
        } else {
            ctx.out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintValue(key, ctx); 
        ctx.out << ": "sv;
        PrintNode(value, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "}"sv;
}

void PrintNode (const Node& node, const PrintContext& ctx) {
    std::visit([&ctx] (const auto& value) {PrintValue(value, ctx);}
        , node.GetValue());    
}

void Print(const Document& doc, std::ostream& out) {
    PrintContext ctx{out};
    PrintNode (doc.GetRoot(), ctx);
}

}  // namespace json