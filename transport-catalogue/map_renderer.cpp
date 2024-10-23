#include "map_renderer.h"

#include <algorithm>
#include <set>
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace map_render {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document MapRender::GetMapRender (const std::map<std::string_view, domain::Bus*>& buses) const {
    svg::Document result;
    std::vector<geo::Coordinates> bus_stops_coord;
    std::map<std::string_view, domain::Stop*> all_stops;

    for (const auto& [bus_name, bus] : buses) {
        
        if (bus->stops_for_bus.empty()) {
            continue;
        }

        for (const auto& stop : bus->stops_for_bus) {
            bus_stops_coord.push_back(stop->stop_coord);
            all_stops[stop->stop_name] = stop;
        }
    }

    SphereProjector projector (bus_stops_coord.begin(), bus_stops_coord.end()
                               , render_settings_.width, render_settings_.height 
                               , render_settings_.padding);

    for(const auto& line : DrawRoughtlines (buses, projector)) {
        result.Add(line);
    }

    for(const auto& bus_name : DrawBusNames (buses, projector)) {
        result.Add(bus_name);
    }

    for(const auto& stop_symbol : DrawStopSymbols (all_stops, projector)) {
        result.Add(stop_symbol);
    }

    for(const auto& stop_name : DrawStopNames (all_stops, projector)) {
        result.Add(stop_name);
    }

    return result;
}

std::vector<svg::Polyline> MapRender::DrawRoughtlines (const std::map<std::string_view, domain::Bus*>& buses, const SphereProjector& projector) const {
    std::vector<svg::Polyline> result;
    size_t color_num = 0;

    for (const auto& [bus_name, bus] : buses) {
        svg::Polyline line;
        std::vector<domain::Stop*> stops_for_bus {bus->stops_for_bus.begin(), bus->stops_for_bus.end()};
        
        if (!bus->is_roundtrip) {
            stops_for_bus.insert(stops_for_bus.end(), std::next(bus->stops_for_bus.rbegin()), bus->stops_for_bus.rend());   
        }

        for (const auto& stop : stops_for_bus) {
            line.AddPoint (projector(stop->stop_coord));
        }

        line.SetStrokeColor (render_settings_.color_palette[color_num]);
        line.SetFillColor ("none");
        line.SetStrokeWidth (render_settings_.line_windth);
        line.SetStrokeLineCap (svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin (svg::StrokeLineJoin::ROUND);

        result.push_back (line);

        if(color_num < render_settings_.color_palette.size() - 1) {
            ++color_num;
        } else {
            color_num = 0;
        }
        
    }
    return result;
}

std::vector<svg::Text> MapRender::DrawBusNames (const std::map<std::string_view, domain::Bus*>& buses, const SphereProjector& projector) const {
    using namespace std::literals;
    
    std::vector<svg::Text> result;
    size_t color_num = 0;

    svg::Text frst_underlayer;
    svg::Text frst_final_stop;

    for (const auto& [bus_name, bus] : buses) {
        std::vector<domain::Stop*> stops_for_bus {bus->stops_for_bus.begin(), bus->stops_for_bus.end()};
        
        frst_underlayer.SetPosition (projector(stops_for_bus.begin().operator*()->stop_coord));
        frst_underlayer.SetOffset (render_settings_.bus_label_offset);
        frst_underlayer.SetFontSize (static_cast<uint32_t>(render_settings_.bus_label_font_size));
        frst_underlayer.SetFontFamily ("Verdana"s);
        frst_underlayer.SetFontWeight ("bold");
        frst_underlayer.SetData (bus->bus_name);
        frst_underlayer.SetFillColor (render_settings_.underlayer_color);
        frst_underlayer.SetStrokeColor (render_settings_.underlayer_color);
        frst_underlayer.SetStrokeWidth (render_settings_.underlayer_width);
        frst_underlayer.SetStrokeLineCap (svg::StrokeLineCap::ROUND);
        frst_underlayer.SetStrokeLineJoin (svg::StrokeLineJoin::ROUND);

        frst_final_stop.SetPosition (projector(stops_for_bus.begin().operator*()->stop_coord));
        frst_final_stop.SetOffset (render_settings_.bus_label_offset);
        frst_final_stop.SetFontSize (static_cast<uint32_t>(render_settings_.bus_label_font_size));
        frst_final_stop.SetFontFamily ("Verdana"s);
        frst_final_stop.SetFontWeight ("bold");
        frst_final_stop.SetData (bus->bus_name);
        frst_final_stop.SetFillColor (render_settings_.color_palette[color_num]);

        result.push_back (frst_underlayer);
        result.push_back (frst_final_stop);

        if (!bus->is_roundtrip && stops_for_bus.begin().operator*() != stops_for_bus.rbegin().operator*()) {
            svg::Text lst_underlayer {frst_underlayer};
            svg::Text lst_final_stop {frst_final_stop};

            lst_underlayer.SetPosition (projector(stops_for_bus.rbegin().operator*()->stop_coord));
            lst_final_stop.SetPosition (projector(stops_for_bus.rbegin().operator*()->stop_coord));

            result.push_back (lst_underlayer);
            result.push_back (lst_final_stop);
        }

        if(color_num < render_settings_.color_palette.size() - 1) {
            ++color_num;
        } else {
            color_num = 0;
        }
    }
    return result;
}

std::vector<svg::Circle> MapRender::DrawStopSymbols (const std::map<std::string_view, domain::Stop*>& all_stops, const SphereProjector& projector) const {
    using namespace std::literals;
    std::vector<svg::Circle> result;

    for (const auto& [stop_name, stop] : all_stops) {
        svg::Circle stop_symbol;

        stop_symbol.SetCenter (projector(stop->stop_coord));
        stop_symbol.SetRadius (render_settings_.stop_radius);
        stop_symbol.SetFillColor ("white"s);

        result.push_back(stop_symbol);
    } 
    return result;
}

std::vector<svg::Text> MapRender::DrawStopNames (const std::map<std::string_view, domain::Stop*>& all_stops, const SphereProjector& projector) const {
    using namespace std::literals;
    std::vector<svg::Text> result;
    svg::Text stop_underlayer;
    svg::Text stop_name;

    for (const auto& [name, stop] : all_stops) {
        stop_underlayer.SetPosition (projector(stop->stop_coord));
        stop_underlayer.SetOffset (render_settings_.stop_label_offset);
        stop_underlayer.SetFontSize (static_cast<uint32_t>(render_settings_.stop_label_font_size));
        stop_underlayer.SetFontFamily("Verdana"s);
        stop_underlayer.SetData (stop->stop_name);
        stop_underlayer.SetFillColor (render_settings_.underlayer_color);
        stop_underlayer.SetStrokeColor (render_settings_.underlayer_color);
        stop_underlayer.SetStrokeWidth (render_settings_.underlayer_width);
        stop_underlayer.SetStrokeLineCap (svg::StrokeLineCap::ROUND);
        stop_underlayer.SetStrokeLineJoin (svg::StrokeLineJoin::ROUND);

        stop_name.SetPosition (projector(stop->stop_coord));
        stop_name.SetOffset (render_settings_.stop_label_offset);
        stop_name.SetFontSize (static_cast<uint32_t>(render_settings_.stop_label_font_size));
        stop_name.SetFontFamily("Verdana"s);
        stop_name.SetData (stop->stop_name);
        stop_name.SetFillColor ("black");

        result.push_back (stop_underlayer);
        result.push_back (stop_name);
    }

    return result;
}

} // namespace map_render


