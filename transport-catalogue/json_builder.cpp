#include "json_builder.h"

namespace json {

/*==============Builder=============*/

Builder::Builder ()
    : root_() {
}

Builder::KeyContext Builder::Key(std::string key) {
    using namespace std::literals;
    if (nodes_stack_.empty()) {
        throw std::logic_error ("Unable to create the key"s);
    }
    if (GetCurrentNode()->IsMap()) {
        Dict& current_node = const_cast<Dict&>(GetCurrentNode()->AsMap());
        current_node[key] = key;
        nodes_stack_.push_back(&current_node[key]);
    } else {
        throw std::logic_error("Key is outside the Dict"s);
    }
    return KeyContext(*this);
}

Builder& Builder::Value (Node::Value value) {
    AddNode (GetNode(value));
    return *this;
}

Builder::DictContext Builder::StartDict() {
    nodes_stack_.push_back(AddNode(Dict{}));
    return DictContext(*this);
}

Builder::ArrayContext Builder::StartArray() {
    nodes_stack_.push_back(AddNode(Array{}));
    return ArrayContext(*this);
}

Builder& Builder::EndDict() {
    using namespace std::literals;
    if (nodes_stack_.empty()) {
        throw std::logic_error ("Unable to create the Dict"s);
    }

    if (!GetCurrentNode()->IsMap()) {
        throw std::logic_error ("Last element is not a Dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    using namespace std::literals;

    if (nodes_stack_.empty()) {
        throw std::logic_error ("Unable to create the Array"s);
    }

    if (!GetCurrentNode()->IsArray()) {
        throw std::logic_error ("Last element is not a Array"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    using namespace std::literals;
    if (!nodes_stack_.empty() || root_.IsNull()) {
        throw std::logic_error("Wrong Build()"s);
    }
    return root_;
}

Node* Builder::GetCurrentNode () {
    return nodes_stack_.back();
}

Node* Builder::AddNode (const Node& node) {
    using namespace std::literals;
    if (nodes_stack_.empty()) {
        if (!root_.IsNull()) {
            throw std::logic_error("Root has been added"s);
        }
        root_ = node;
        return &root_;
    } else {
        if (GetCurrentNode()->IsArray()) {
            Array& arr = const_cast<Array&>(GetCurrentNode()->AsArray());
            arr.push_back(node);
            return &arr.back();
        } else if (GetCurrentNode()->IsString()) {
            const std::string key = GetCurrentNode()->AsString();
            nodes_stack_.pop_back();
            if (GetCurrentNode()->IsMap()) {
                Dict& dict = const_cast<Dict&>(GetCurrentNode()->AsMap());
                dict[key] = node;
                return &dict[key];
            } else {
                throw std::logic_error("Key is outside the dict"s);
            }
        } else {
            throw std::logic_error("Data format is incorrect"s);
        }
    }
}

Node Builder::GetNode (Node::Value value) {
    if (std::holds_alternative<Array>(value)) {
        return Node(std::get<Array>(value));
    } 
    if (std::holds_alternative<Dict>(value)) {
        return Node(std::get<Dict>(value));
    } 
    if (std::holds_alternative<bool>(value)) {
        return Node(std::get<bool>(value));
    } 
    if (std::holds_alternative<int>(value)) {
        return Node(std::get<int>(value));
    } 
    if (std::holds_alternative<double>(value)) {
        return Node(std::get<double>(value));
    } 
    if (std::holds_alternative<std::string>(value)) {
        return Node(std::get<std::string>(value));
    } 
    return Node();
}

/*==============BaseContext=============*/

Builder::BaseContext::BaseContext (Builder& builder) 
    : builder_(builder) {
};

Builder::KeyContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder& Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::DictContext Builder::BaseContext::StartDict() {
    return Builder::DictContext (builder_.StartDict());
}

Builder::ArrayContext Builder::BaseContext::StartArray() {
    return ArrayContext (builder_.StartArray());
}

Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

/*==============KeyContext=============*/

Builder::KeyContext::KeyContext (Builder& builder) 
    : BaseContext (builder) {
};

Builder::DictContext Builder::KeyContext::Value(Node::Value value) {
    return BaseContext::Value(value);
}

/*==============DictContext=============*/

Builder::DictContext::DictContext (Builder& builder) 
    : BaseContext (builder) {
};

/*==============ArrayContext=============*/

Builder::ArrayContext::ArrayContext (Builder& builder) 
    : BaseContext (builder) {
};

Builder::ArrayContext Builder::ArrayContext::Value(Node::Value value) {
    return BaseContext::Value(value);
}

} //  namespace json

