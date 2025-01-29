#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct StopVertex {
    graph::VertexId wait;
    graph::VertexId bus;
};

struct RouteSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

class TransportRouter {
public:
    struct Item {
        std::string type;
        std::string name;
        double time = 0.0;
        int span_count = 0;
    };

    struct RouteItems {
        double total_time = 0.0;
        std::vector<Item> items;
    };

    TransportRouter () = default;
    TransportRouter (const trans_cat::TransportCatalogue& catalogue, const RouteSettings& settings);

    std::optional<RouteItems> GetRouteBetweenStops (std::string_view from_stop_name, std::string_view to_stop_name) const;

private:
    RouteSettings route_settings_;
    const trans_cat::TransportCatalogue& catalogue_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::map<domain::Stop*, StopVertex> vertexes_;
    std::map<graph::EdgeId, Item> edges_;

    void AddAllStopsToGraph ();
    StopVertex GetVertexByStop (domain::Stop* stop) const;
    void AddBusEdge (domain::Stop* stop_from, domain::Stop* stop_to, std::string_view bus_name, int span, double distance);
    void AddRouteToGraph ();
    void BuildAllRoutes ();
};

} // namespace transport_router

