#include "json_reader.h"
#include "request_handler.h"

#include <tuple>

namespace json_reader {

RequestType GetRequestType (std::string request){
    if(request == "Stop"){
        return RequestType::Stop;
    } else if (request == "Bus"){
        return RequestType::Bus;
    } else if (request == "Map"){
        return RequestType::Map;
    }
    return RequestType::Unknown;
}

const json::Node& JsonReader::GetBaseRequest() {
    if (input_.GetRoot().AsMap().count("base_requests")) {
        return input_.GetRoot().AsMap().at("base_requests");
    }
    return null_;
}

const json::Node& JsonReader::GetStatRequest() {
    if (input_.GetRoot().AsMap().count("stat_requests")) {
        return input_.GetRoot().AsMap().at("stat_requests");
    }
    return null_;
}

const json::Node& JsonReader::GetRenderSettings() {
    if (input_.GetRoot().AsMap().count("render_settings")) {
        return input_.GetRoot().AsMap().at("render_settings");
    }
    return null_;
}

void JsonReader::ProcessBaseRequest (trans_cat::TransportCatalogue& catalogue) {
    const json::Array& request = GetBaseRequest().AsArray();
    std::vector<json::Node> bus_buffer;
    std::vector<json::Node> stop_buffer;
    
    for(const auto& node : request){
        switch (GetRequestType(node.AsMap().at("type").AsString())){
            case RequestType::Stop : {
                std::pair<std::string, geo::Coordinates> stop = GetStopFromRequest(node.AsMap());
                stop_buffer.push_back(node);
                catalogue.AddStop(stop.first, stop.second);
                break;
            }
            case RequestType::Bus : {
                bus_buffer.push_back(node);
                break;
            }
            case RequestType::Map : {
                break;
            }
            case RequestType::Unknown : {
                break;
            }
        }
    }
    SetDistanceFromRequest(catalogue, stop_buffer);

    for(const auto& bus_node : bus_buffer){
        auto [bus_name, stops_for_bus, is_roundtrip] = GetBusFromRequest(catalogue, bus_node.AsMap());
        catalogue.AddBus(bus_name, stops_for_bus, is_roundtrip);
    }
}

map_render::RenderSettings json_reader::JsonReader::ProcessRenderSetting (const json::Dict& request) {
    map_render::RenderSettings render_settings;

    render_settings.width = request.at("width").AsDouble();
    render_settings.height = request.at("height").AsDouble();
    render_settings.padding = request.at("padding").AsDouble();
    render_settings.line_windth = request.at("line_width").AsDouble();
    render_settings.stop_radius = request.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size = request.at("bus_label_font_size").AsInt();
    
    json::Array bus_label_offset = request.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};
    
    render_settings.stop_label_font_size = request.at("stop_label_font_size").AsInt();
    
    json::Array stop_label_offset = request.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    if (request.at("underlayer_color").IsString()) {
        render_settings.underlayer_color = request.at("underlayer_color").AsString();
    } else if (request.at("underlayer_color").IsArray()) {
        const json::Array underlayer_color = request.at("underlayer_color").AsArray();

        if (underlayer_color.size() == 3) {
            render_settings.underlayer_color = svg::Rgb(static_cast<uint8_t> (underlayer_color[0].AsInt()), 
                                                        static_cast<uint8_t> (underlayer_color[1].AsInt()), 
                                                        static_cast<uint8_t> (underlayer_color[2].AsInt()));
        }

        if (underlayer_color.size() == 4) {
            render_settings.underlayer_color = svg::Rgba(static_cast<uint8_t> (underlayer_color[0].AsInt()), 
                                                         static_cast<uint8_t> (underlayer_color[1].AsInt()), 
                                                         static_cast<uint8_t> (underlayer_color[2].AsInt()), 
                                                         underlayer_color[3].AsDouble());
        }
    }
    render_settings.underlayer_width   = request.at("underlayer_width").AsDouble();

    json::Array color_palette = request.at("color_palette").AsArray();
    for (const auto& color : color_palette) {
        
        if (color.IsString()) {
            render_settings.color_palette.push_back(color.AsString());
        } else if (color.IsArray()) {
            json::Array color_type = color.AsArray();

            if (color_type.size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(static_cast<uint8_t> (color_type[0].AsInt()),
                                                                 static_cast<uint8_t> (color_type[1].AsInt()),
                                                                 static_cast<uint8_t> (color_type[2].AsInt())));
            } else if (color_type.size() == 4) {
                render_settings.color_palette.push_back(svg::Rgba(static_cast<uint8_t> (color_type[0].AsInt()),
                                                                  static_cast<uint8_t> (color_type[1].AsInt()),
                                                                  static_cast<uint8_t> (color_type[2].AsInt()),
                                                                  color_type[3].AsDouble()));
            }
        }
    }
    return render_settings;
}

std::pair<std::string, geo::Coordinates> JsonReader::GetStopFromRequest (const json::Dict& request) {
    std::string stop_name = request.at("name").AsString();
    geo::Coordinates stop_coord = {request.at("latitude").AsDouble(), request.at("longitude").AsDouble()};
    return std::make_pair(stop_name, stop_coord);
}

std::tuple<std::string, std::vector<domain::Stop*>, bool> JsonReader::GetBusFromRequest (trans_cat::TransportCatalogue& catalogue, const json::Dict& request) {
    std::string bus_name = request.at("name").AsString();
    std::vector<domain::Stop*> stops_for_bus;

    for (const auto& stop : GetStopsForBusFromRequest(catalogue, request)) {
        stops_for_bus.push_back(stop);
    }
    
    bool is_roundtrip = request.at("is_roundtrip").AsBool();
    return std::make_tuple(bus_name, stops_for_bus, is_roundtrip);
}

void JsonReader::SetDistanceFromRequest (trans_cat::TransportCatalogue& catalogue, json::Array& request){
    for (const auto& stops_dict : request) {
        std::string stop_from = stops_dict.AsMap().at("name").AsString();
        std::string stop_to;
        size_t distance = 0;

        for (const auto& [stop, dist] : stops_dict.AsMap().at("road_distances").AsMap()) {
            stop_to = stop;
            distance = static_cast<size_t>(dist.AsInt());
            catalogue.SetDistBetweenStops(catalogue.GetStopByName(stop_from), catalogue.GetStopByName(stop_to), distance);
        }
    }
}

std::vector<domain::Stop*> JsonReader::GetStopsForBusFromRequest (trans_cat::TransportCatalogue& catalogue, const json::Dict& request) {
    std::vector<domain::Stop*> result;
    
    for (const auto& stop : request.at("stops").AsArray()) {
        result.push_back(catalogue.GetStopByName(stop.AsString()));
    }
    return result;
}

} //namespace json_reader