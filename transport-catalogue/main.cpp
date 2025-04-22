#include <iostream>
#include <fstream>
#include <cassert>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

int main() {
    using namespace std::literals;
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */

    trans_cat::TransportCatalogue tc;
    json_reader::JsonReader reader(std::cin);

    reader.ProcessBaseRequest(tc);

    const auto& render_settings_node = reader.GetRenderSettings().AsMap();
    const auto& router_settings_node = reader.GetRouteSettings ().AsMap ();

    const auto& render_settings = reader.ProcessRenderSetting (render_settings_node);
    const auto& route_settings  = reader.ProcessRouterSetting (router_settings_node);

    map_render::MapRender mr (render_settings);

    transport_router::TransportRouter router (tc, route_settings);
    
    req_handl::RequestHandler rh (tc, mr, router);

    rh.ProcessStatRequest (reader.GetStatRequest ());
}