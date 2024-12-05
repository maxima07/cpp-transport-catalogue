#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "json_reader.h"

namespace req_handl {

struct StatRequest {
    int id;
    json_reader::RequestType type;
    std::string name;
};

class RequestHandler {
public:
    explicit RequestHandler (trans_cat::TransportCatalogue catalogue, map_render::MapRender map_renderer) 
        : catalogue_(catalogue)
        , map_renderer_(map_renderer) {
    }

    void ProcessStatRequest (const json::Node& stat_request) const;
    svg::Document MapRender () const;

private:
    const trans_cat::TransportCatalogue catalogue_;
    const map_render::MapRender map_renderer_;

    json::Node PrintStop (const StatRequest& request) const;
    json::Node PrintBus  (const StatRequest& request) const;
    json::Node PrintMap  (const StatRequest& request) const;
};

} // namespace req_handl

