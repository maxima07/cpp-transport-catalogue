#pragma once

#include <stdexcept>

#include "json.h"

namespace json {

class Builder {
private:
    class BaseContext;
    class KeyContext;
    class DictContext;
    class ArrayContext;

public:
    Builder ();
    KeyContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node* GetCurrentNode ();
    Node* AddNode (const Node& node);
    Node GetNode (Node::Value value);

    class BaseContext {
    public:
        BaseContext (Builder& builder);
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
    private:
        Builder& builder_;
    };

    class KeyContext : public BaseContext {
    public:
        KeyContext (Builder& builder);
        DictContext Value(Node::Value value);

        KeyContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;

    };

    class DictContext : public BaseContext {
    public:
        DictContext (Builder& builder);
        
        Builder& Value(Node::Value value) = delete;
        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        Builder& EndArray() = delete;
    };

    class ArrayContext : public BaseContext {
    public:
        ArrayContext (Builder& builder);
        ArrayContext Value(Node::Value value);
        
        KeyContext& Key(std::string key) = delete;
        Builder& EndDict() = delete;
    };
};

} // namespace json