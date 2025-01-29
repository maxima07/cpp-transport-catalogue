
#include "request_handler.h"

#include <sstream>

namespace req_handl {

void RequestHandler::ProcessStatRequest (const json::Node& stat_request) const {
    using namespace std::literals;
    
    json::Array result;
    
    for (const auto& query : stat_request.AsArray()) {
        StatRequest request;
        request.id = query.AsMap().at("id"s).AsInt();
        request.type = json_reader::GetRequestType(query.AsMap().at("type").AsString());
        
        if (query.AsMap().count("name"s)) {
            request.name = query.AsMap().at("name"s).AsString();
        }

        if (query.AsMap().count("from")) {
            request.from = query.AsMap().at("from"s).AsString();
        }

        if (query.AsMap().count("to"s)) {
            request.to = query.AsMap().at("to"s).AsString();
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
            case json_reader::RequestType::Route : {
                result.push_back(PrintRoute (request));
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
    return map_renderer_.GetMapRender (catalogue_.GetBusDirectory ());
}

json::Node RequestHandler::PrintStop (const StatRequest& request) const {
    using namespace std::literals;
    json::Node result;

    if (catalogue_.GetStopByName (request.name) != nullptr) {
        json::Array buses;

        for (const auto& bus_name : catalogue_.GetStopPropertyByName (request.name)) {
            buses.push_back(json::Node(bus_name));
        }

        result = json::Builder {}
            .StartDict()
                .Key ("request_id").Value (request.id)
                .Key ("buses").Value (buses)
            .EndDict ()
        .Build ();
    
    } else {
        result = json::Builder {}
            .StartDict ()
                .Key ("request_id"s).Value (request.id)
                .Key ("error_message"s).Value ("not found"s)
            .EndDict ()
        .Build ();
    }
    return result;
}

json::Node RequestHandler::PrintBus (const StatRequest& request) const {
    using namespace std::literals;  
    json::Node result;

    if (catalogue_.GetBusByName (request.name) != nullptr) {
        domain::BusStat bus_property = catalogue_.GetBusPropertyByName (request.name);
        result = json::Builder {}
            .StartDict ()
                // .Key("request_id").Value(request_id)
                .Key ("request_id"s).Value (request.id)
                .Key ("curvature"s).Value (bus_property.route_length / bus_property.route_geo_length)
                .Key ("route_length"s).Value (bus_property.route_length)
                .Key ("stop_count"s).Value (bus_property.all_stop_count)
                .Key ("unique_stop_count"s).Value (bus_property.uniq_stop_count)
            .EndDict ()
        .Build ();

    } else {
        result = json::Builder {}
            .StartDict()
                // .Key("request_id").Value(request_id)
                .Key ("request_id"s).Value (request.id)
                .Key ("error_message"s).Value ("not found"s)
            .EndDict ()
        .Build ();
    }
    return result;
}

json::Node RequestHandler::PrintMap (const StatRequest& request) const {
    using namespace std::literals;
    
    std::ostringstream strm;
    svg::Document map = MapRender ();
    map.Render (strm);
    return json::Builder {}
        .StartDict()
            .Key ("request_id"s).Value (request.id)
            .Key ("map"s).Value (strm.str())
        .EndDict ()
    .Build ();
}

json::Node RequestHandler::PrintRoute (const StatRequest& request) const {
    using namespace std::literals;

    json::Node result;
    const auto route = router_.GetRouteBetweenStops (request.from, request.to);
    
    if (route.has_value()) {
        json::Array items;

        if (route.value().items.empty()) {
            result = json::Builder{}
                    .StartDict()
                        .Key ("request_id"s).Value (request.id)
                        .Key ("total_time"s).Value (route.value().total_time)
                        .Key ("items")
                            .StartArray ()
                            .EndArray ()
                    .EndDict ()
                .Build ();
        }

        for (auto& item : route.value().items) {
            if (item.type == "Wait"s){
                items.push_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key ("stop_name"s).Value (item.name)
                        .Key ("time"s).Value (item.time)
                        .Key ("type"s).Value ("Wait"s)
                    .EndDict()
                .Build()));

            } else {
                items.push_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key ("bus"s).Value (item.name)
                        .Key ("span_count").Value (item.span_count)
                        .Key ("time"s).Value (item.time)
                        .Key ("type"s).Value ("Bus"s)
                    .EndDict()
                .Build()));
                
            }
            result = json::Builder{}
                .StartDict()
                    .Key ("request_id"s).Value (request.id)
                    .Key ("total_time"s).Value (route.value().total_time)
                    .Key ("items").Value (items)
                .EndDict ()
            .Build ();
        }
        
    } else {
        result = json::Builder {}
            .StartDict()
                .Key ("request_id"s).Value (request.id)
                .Key ("error_message"s).Value ("not found"s)
            .EndDict ()
        .Build ();
    }
    return result;
} 

} // namespace req_handl