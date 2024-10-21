#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
    
    Node () = default;
    Node (std::nullptr_t null);
    Node (int value);
    Node (double value);
    Node (std::string value);
    Node (bool value);
    Node (Array value);
    Node (Dict value);

    // проверка значения внутри 
    bool IsNull()       const;
    bool IsInt()        const;
    bool IsDouble()     const;
    bool IsPureDouble() const;
    bool IsString()     const;
    bool IsBool()       const;
    bool IsArray()      const;
    bool IsMap()        const;

    // возвращает значение из variant
    int     AsInt()                  const;
    double  AsDouble()               const;
    const   std::string& AsString()  const;
    bool    AsBool()                 const;
    const   Array& AsArray()         const;
    const   Dict& AsMap()            const;

    const Value& GetValue() const;

private:
    Value value_;
};

bool operator== (const Node& lhs, const Node& rhs);
bool operator!= (const Node& lhs, const Node& rhs);

// -----------Document---------------

class Document {
public:
    explicit Document(Node root);
    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

// -----------Print---------------

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const;

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const;
};

void PrintValue (std::nullptr_t, const PrintContext& ctx);

// PrintValue для int и double
template <typename Value>
void PrintValue (const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue (std::string value, const PrintContext& ctx);
void PrintValue (bool value,        const PrintContext& ctx);
void PrintValue (Array values,      const PrintContext& ctx);
void PrintValue (Dict values,       const PrintContext& ctx);

void PrintNode (const Node& node, const PrintContext& ctx);
void Print(const Document& doc, std::ostream& out);

}  // namespace json