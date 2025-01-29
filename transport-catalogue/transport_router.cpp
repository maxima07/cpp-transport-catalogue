#include "transport_router.h"

namespace transport_router {

TransportRouter::TransportRouter (const trans_cat::TransportCatalogue& catalogue, const RouteSettings& settings) 
    : route_settings_ (settings)
    , catalogue_ (catalogue) {
        BuildAllRoutes ();
}

std::optional<TransportRouter::RouteItems> TransportRouter::GetRouteBetweenStops (std::string_view from_stop_name, std::string_view to_stop_name) const {
    RouteItems items_info;
    auto stop_from   = catalogue_.GetStopByName (from_stop_name);
    auto stop_to     = catalogue_.GetStopByName (to_stop_name);
    auto router_info = router_ -> BuildRoute (GetVertexByStop(stop_from).wait, GetVertexByStop(stop_to).wait);

    if (router_info.has_value()) {
        items_info.total_time = router_info.value().weight;
        
        for (const auto& edge : router_info.value().edges) {
            items_info.items.push_back (edges_.at(edge));
        }
        
        return items_info;
    } else {
        return std::nullopt;
    }
}

void TransportRouter::AddAllStopsToGraph () {
    using namespace std::literals;
    graph::VertexId vertex_id = 0;

    // for (const auto& [stop_name, stop] : catalogue_.GetStopDir ()) {
    for (const auto& [stop_name, stop] : catalogue_.GetStopDirectory ()) {
        vertexes_[stop] = {vertex_id, vertex_id + 1};
        auto edge_id = graph_ -> AddEdge ({vertex_id, vertex_id + 1, static_cast<double>(route_settings_.bus_wait_time)});

        Item item;
        item.type = "Wait"s;
        item.name = stop_name;
        item.time = static_cast<double>(route_settings_.bus_wait_time);
        item.span_count = 1;

        edges_[edge_id] = std::move (item);
        vertex_id += 2;
    }
}

StopVertex TransportRouter::GetVertexByStop (domain::Stop* stop) const {
    return vertexes_.at(stop);
}

void TransportRouter::AddBusEdge (domain::Stop* stop_from, domain::Stop* stop_to, std::string_view bus_name, int span, double distance) {
    using namespace std::literals;

    const double conv_meter_per_min = 1000. / 60.;

    Item item;
    item.type = "Bus"s;
    item.name = bus_name;
    item.time = distance / (route_settings_.bus_velocity * conv_meter_per_min);
    item.span_count = span;

    auto vertex_from = GetVertexByStop(stop_from);
    auto vertex_to   = GetVertexByStop(stop_to);
    
    auto edge_id     = graph_ -> AddEdge ({vertex_from.bus, vertex_to.wait, item.time});

    edges_[edge_id]  = std::move (item);    
}

void TransportRouter::AddRouteToGraph () {
    for (const auto& [bus_name, bus] : catalogue_.GetBusDirectory ()){
        for (size_t i = 0; i < bus->stops_for_bus.size() - 1; ++i) {
            double forward_dist  = 0.0;
            double backward_dist = 0.0;
        
            for (size_t j = i; j < bus->stops_for_bus.size() - 1; ++j) {
                forward_dist += static_cast<double>(catalogue_.GetDistBetweenStops(bus->stops_for_bus[j], bus->stops_for_bus[j + 1]));
                AddBusEdge (bus->stops_for_bus[i], bus->stops_for_bus[j + 1], bus_name, static_cast<int>(j - i + 1), forward_dist);
                
                if (!bus->is_roundtrip) {
                    backward_dist += static_cast<double>(catalogue_.GetDistBetweenStops(bus->stops_for_bus[j + 1], bus->stops_for_bus[j]));
                    AddBusEdge (bus->stops_for_bus[j + 1], bus->stops_for_bus[i], bus_name,  static_cast<int>(j - i + 1), backward_dist);
                }
            }        
        }
    }
}

void TransportRouter::BuildAllRoutes () {
    size_t all_stops_count = catalogue_.GetStopDirectory ().size();

    graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(all_stops_count * 2);
    AddAllStopsToGraph ();
    AddRouteToGraph ();
    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

} // namespace transport_router
