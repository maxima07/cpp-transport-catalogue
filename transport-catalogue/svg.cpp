#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- RGB -----------

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) 
    : red(r), green(g), blue(b) {
}

// ---------- RGBA -----------

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o) 
    : Rgb(r, g, b), opacity(o){
}

// ---------- Color ---------

void PrintColor (std::ostream& out, std::monostate){
    out << "none"sv;
}

void PrintColor (std::ostream& out, std::string& color){
    out << color;
}

void PrintColor (std::ostream& out, Rgb& rgb){
    out << "rgb("sv << static_cast<short>(rgb.red)        << ","sv
                    << static_cast<short>(rgb.green)      << ","sv
                    << static_cast<short>(rgb.blue)       << ")"sv;
}

void PrintColor (std::ostream& out, Rgba& rgba){
    out << "rgba("sv << static_cast<short>(rgba.red)      << ","sv
                     << static_cast<short>(rgba.green)    << ","sv
                     << static_cast<short>(rgba.blue)     << ","sv
                     << rgba.opacity  << ")"sv;
}

std::ostream& operator<< (std::ostream& out, const Color& color){
    std::visit(
        [&out](auto value){
            PrintColor(out, value);
        }, color);

    return out;
}

// ---------- StrokeLineCap -----------

std::ostream& operator<< (std::ostream& stream, const StrokeLineCap& stroke_linecap){
    using namespace std::literals::string_view_literals;
    
    switch (stroke_linecap){
        case StrokeLineCap::BUTT :
            stream << "butt"sv;
            break;
        case StrokeLineCap::ROUND :
            stream << "round"sv;
            break;
        case StrokeLineCap::SQUARE :
            stream << "square"sv;
            
        default : 
            break;
    }
    return stream;
}

// ---------- StrokeLineJoin -----------

std::ostream& operator<< (std::ostream& stream, const StrokeLineJoin& stroke_linejoin){
    using namespace std::literals::string_view_literals;

    switch (stroke_linejoin){
        case StrokeLineJoin::ARCS :
            stream << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL :
            stream << "bevel"sv;
            break;       
        case StrokeLineJoin::MITER :
            stream << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP :
            stream << "miter-clip"sv;
            break;     
        case StrokeLineJoin::ROUND :
            stream << "round"sv;
            break;    
        default : 
            break;
    }
    return stream;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center){
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius){
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    
    for(const auto point : points_){
        if(!is_first){
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        is_first = false;
    }
    
    out << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text -------------------

Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    f_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    f_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    f_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

//Замена спецсимолов ",',<,>,& на их HTML код
void Text::ParseSpecChar(std::ostream& out, const std::string& data) const {
    for(char c : data){
        switch (c){
        case '\"' :
            out << "&quot;"sv;
            break;
        case '\'' :
            out << "&apos;"sv;
            break;
        case '<' :
            out << "&lt;"sv;
            break;
        case '>' :
            out << "&gt;"sv;
            break;
        case '&' :
            out << "&amp;"sv;
            break;
        default :
            out << c;
            break;
        }
    }
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;
    RenderAttrs(out);
    out << "x=\""sv << position_.x << "\" "sv;
    out << "y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" "sv;
    out << "dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << f_size_ << "\""sv;
    
    if(!f_family_.empty()){
        out << " font-family=\""sv << f_family_ << "\""sv;
    }

    if(!f_weight_.empty()){
        out << " font-weight=\""sv << f_weight_ << "\""sv;
    }
    
    out << ">"sv;

    if(!data_.empty()){
        ParseSpecChar(out, data_);
    }

    out << "</text>"sv;
}

// ---------- Document --------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

    for(const auto& obj : objects_){
        obj->Render({out, 2, 2});
    }

    out << "</svg>"sv;
}

}  // namespace svg
