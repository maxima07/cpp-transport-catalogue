
#include "request_handler.h"

#include <sstream>

namespace req_handl {

void RequestHandler::ProcessStatRequest (const json::Node& stat_request) const {
    json::Array result;
    
    for (const auto& query : stat_request.AsArray()) {
        StatRequest request;
        request.id = query.AsMap().at("id").AsInt();
        request.type = json_reader::GetRequestType(query.AsMap().at("type").AsString());
        
        if (query.AsMap().count("name")) {
            request.name = query.AsMap().at("name").AsString();
        }
        
        switch (request.type) {
            case json_reader::RequestType::Stop : {
                result.push_back(PrintStop (request));
                break;
            }
            case json_reader::RequestType::Bus : {
                result.push_back(PrintBus (request));
                break;
            }
            case json_reader::RequestType::Map : {
                result.push_back(PrintMap (request));
                break;
            }
            case json_reader::RequestType::Unknown : {
                break;
            }
        }
    }
    json::Print (json::Document{result}, std::cout);
}

svg::Document RequestHandler::MapRender () const {
    return map_renderer_.GetMapRender(catalogue_.GetSortedBusDirectory());
}

json::Node RequestHandler::PrintStop (const StatRequest& request) const {
    json::Node result;

    const int request_id = request.id;
    const std::string& stop_name = request.name;

    if (catalogue_.GetStopByName (stop_name) != nullptr) {
        json::Array buses;

        for (const auto& bus_name : catalogue_.GetStopPropertyByName(stop_name)) {
            buses.push_back(json::Node(bus_name));
        }

        result = json::Builder {}
            .StartDict()
                .Key("request_id").Value(request_id)
                .Key("buses").Value(buses)
            .EndDict()
        .Build();
    
    } else {
        result = json::Builder {}
            .StartDict()
                .Key("request_id").Value(request_id)
                .Key("error_message").Value("not found")
            .EndDict()
        .Build();
    }
    return result;
}

json::Node RequestHandler::PrintBus (const StatRequest& request) const {
    json::Node result;

    const int request_id = request.id;
    const std::string& bus_name = request.name;

    if (catalogue_.GetBusByName(bus_name) != nullptr) {
        domain::BusStat bus_property = catalogue_.GetBusPropertyByName(bus_name);

        result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(request.id)
                .Key("curvature").Value(bus_property.route_length / bus_property.route_geo_length)
                .Key("route_length").Value(bus_property.route_length)
                .Key("stop_count").Value(bus_property.all_stop_count)
                .Key("unique_stop_count").Value(bus_property.uniq_stop_count)
            .EndDict()
        .Build();

    } else {
        result = json::Builder {}
            .StartDict()
                .Key("request_id").Value(request_id)
                .Key("error_message").Value("not found")
            .EndDict()
        .Build();
    }
    return result;
}

json::Node RequestHandler::PrintMap (const StatRequest& request) const {
    std::ostringstream strm;
    svg::Document map = MapRender();
    map.Render (strm);
    return json::Builder{}
        .StartDict()
            .Key("request_id").Value(request.id)
            .Key("map").Value(strm.str())
        .EndDict()
    .Build();
}

} // namespace req_handl