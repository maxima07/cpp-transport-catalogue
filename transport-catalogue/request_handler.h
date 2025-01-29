#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "json_reader.h"

namespace req_handl {

struct StatRequest {
    int id;
    json_reader::RequestType type;
    std::string name;
    std::string from;
    std::string to;
};

class RequestHandler {
public:
    explicit RequestHandler (const trans_cat::TransportCatalogue& catalogue, const map_render::MapRender& map_renderer, const transport_router::TransportRouter& router) 
        : catalogue_ (catalogue)
        , map_renderer_ (map_renderer)
        , router_ (router) {
    }

    void ProcessStatRequest (const json::Node& stat_request) const;
    svg::Document MapRender () const;

private:
    const trans_cat::TransportCatalogue& catalogue_;
    const map_render::MapRender& map_renderer_;
    const transport_router::TransportRouter& router_;

    json::Node PrintStop   (const StatRequest& request) const;
    json::Node PrintBus    (const StatRequest& request) const;
    json::Node PrintMap    (const StatRequest& request) const;
    json::Node PrintRoute  (const StatRequest& request) const;
};

} // namespace req_handl

