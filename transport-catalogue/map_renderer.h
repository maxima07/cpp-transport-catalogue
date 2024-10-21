#pragma once

#include "domain.cpp"
#include "json.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

namespace map_render {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
                    : padding_(padding) {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    // Ширина изображения в пикселях
    double width = 0.0;
    // Высота изображения в пикселях
    double height = 0.0;
    // Отступ краёв карты от границ SVG-документа
    double padding = 0.0;
    // толщина линий, которыми рисуются автобусные маршруты.
    double line_windth = 0.0;
    // радиус окружностей, которыми обозначаются остановки.
    double stop_radius = 0.0;
    // Размер текста, которым написаны названия автобусных маршрутов.
    int bus_label_font_size = 0;
    // Смещение надписи с названием маршрута относительно координат конечной остановки на карте.
    svg::Point bus_label_offset = {0.0, 0.0};
    // Размер текста, которым отображаются названия остановок.
    int stop_label_font_size = 0;
    // Смещение названия остановки относительно её координат на карте.
    svg::Point stop_label_offset = {0.0, 0.0};
    // Цвет подложки под названиями остановок и маршрутов.
    svg::Color underlayer_color = {svg::NoneColor};
    // Толщина подложки под названиями остановок и маршрутов.
    double underlayer_width = 0.0;
    // Цветовая палитра
    std::vector<svg::Color> color_palette = {};
};

class MapRender {
public:
    MapRender (RenderSettings settings) 
        : render_settings_(settings){
    }
    
    svg::Document GetMapRender (std::map<std::string_view, domain::Bus*> buses) const;

private:
    RenderSettings render_settings_;
    
    std::vector<svg::Polyline> DrawRoughtlines (const std::map<std::string_view, domain::Bus*> buses, const SphereProjector& projector) const;
    std::vector<svg::Text> DrawBusNames (std::map<std::string_view, domain::Bus*> buses, const SphereProjector& projector) const;
    std::vector<svg::Circle> DrawStopSymbols (std::map<std::string_view, domain::Stop*> all_stops, const SphereProjector& projector) const;
    std::vector<svg::Text> DrawStopNames (std::map<std::string_view, domain::Stop*> all_stops, const SphereProjector& projector) const;
};

} // namespace map_render

