#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

// ---------- Point -----------
struct Point {
    Point() = default;
    Point(double px, double py)
        : x(px)
        , y(py) {
    }
    double x = 0;
    double y = 0;
};

// ------- RenderContext --------
/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out_)
        : out(out_) {
    }

    RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0)
        : out(out_)
        , indent_step(indent_step_)
        , indent(indent_) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

// ---------- Object -----------

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

// ---------- RGB -----------

class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

// ---------- RGBA -----------

class Rgba : public Rgb{
public:
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o);

    double opacity = 1.0;
};

// ---------- Color ---------

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

void PrintColor (std::ostream& out, std::monostate);
void PrintColor (std::ostream& out, std::string& color);
void PrintColor (std::ostream& out, Rgb& rgb);
void PrintColor (std::ostream& out, Rgba& rgba);

std::ostream& operator<< (std::ostream& out, const Color& color);

// ---------- StrokeLineCap -----------

// Тип формы конца линии
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<< (std::ostream& stream, const StrokeLineCap& stroke_linecap);

// ---------- StrokeLineJoin -----------

// Тип формы соединения линии
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<< (std::ostream& stream, const StrokeLineJoin& stroke_linejoin);

// ---------- PathProps -----------

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor (Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor (Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth (double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }

    Owner& SetStrokeLineCap (StrokeLineCap stroke_linecap) {
        stroke_linecap_ = std::move(stroke_linecap);
        return AsOwner();
    }

    Owner& SetStrokeLineJoin (StrokeLineJoin stroke_linejoin) {
        stroke_linejoin_ = std::move(stroke_linejoin);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_){
            out << "fill=\""sv << *fill_color_ << "\" "sv;
        }

        if (stroke_color_){
            out << "stroke=\""sv << *stroke_color_ << "\" "sv;
        }

        if(stroke_width_){
            out << "stroke-width=\""sv << stroke_width_ << "\" "sv;
        }

        if(stroke_linecap_){
            out << "stroke-linecap=\""sv << *stroke_linecap_ << "\" "sv;
        }

        if(stroke_linejoin_){
            out << "stroke-linejoin=\""sv << *stroke_linejoin_ << "\" "sv;
        }
    } 

private:
    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }

    // Цвет заливки
    std::optional<Color> fill_color_;

    // цвет контура
    std::optional<Color> stroke_color_;

    // Толщина линии
    double stroke_width_ = 0.0;

    // Тип формы конца линии
    std::optional<StrokeLineCap> stroke_linecap_;

    // Тип формы соединения линии
    std::optional<StrokeLineJoin> stroke_linejoin_;
};

// ---------- Circle ------------------

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

// ---------- Polyline ----------------

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

// ---------- Text -------------------

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void ParseSpecChar(std::ostream& out, const std::string& data) const;
    void RenderObject(const RenderContext& context) const override;
    
    Point position_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t f_size_ = 1;
    std::string f_family_;
    std::string f_weight_;
    std::string data_;
};

// ---------- ObjectContainer --------------

class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    template <typename Object>
    void Add(Object object);
};

// ---------- Drawable --------------

class Drawable {
public:
    virtual void Draw (ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

// ---------- Document --------------

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

template <typename Object>
void ObjectContainer::Add (Object object){
    AddPtr(std::make_unique<Object>(std::move(object)));
}

}  // namespace svg