
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
    using namespace std::literals;
    json::Dict dict;
    
    const auto& stop = catalogue_.GetStopByName(request.name);
    dict["request_id"] = request.id;

    if (stop != nullptr) {
        json::Array buses;
       
        for (const auto& bus_name : catalogue_.GetStopPropertyByName(stop->stop_name)) {
            buses.push_back(json::Node(bus_name));
        }

        dict["buses"] = buses;
    } else {
        dict["error_message"]     = "not found"s;
    }
    return json::Node{dict};
}

json::Node RequestHandler::PrintBus (const StatRequest& request) const {
    using namespace std::literals;
    json::Dict dict;
    const auto& bus = catalogue_.GetBusByName(request.name);
    dict["request_id"] = request.id;

    if (bus != nullptr) {
        domain::BusStat bus_property = catalogue_.GetBusPropertyByName(bus->bus_name);
        dict["request_id"]        = request.id;
        dict["curvature"]         = bus_property.route_length / bus_property.route_geo_lenght;
        dict["route_length"]      = bus_property.route_length;
        dict["stop_count"]        = bus_property.all_stop_count;
        dict["unique_stop_count"] = bus_property.uniq_stop_count;
    } else {
        dict["error_message"]     = "not found"s;
    }
    return json::Node{dict};
}

json::Node RequestHandler::PrintMap (const StatRequest& request) const {
    json::Dict dict;

    std::ostringstream strm;
    svg::Document map = MapRender();
    map.Render (strm);
    dict["request_id"] = request.id;
    dict["map"] = strm.str();
    return json::Node{dict};
}

} // namespace req_handl