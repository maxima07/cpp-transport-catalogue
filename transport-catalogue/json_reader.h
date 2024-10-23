#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace json_reader {

enum class RequestType {
    Bus,
    Stop,
    Map,
    Unknown
};

RequestType GetRequestType (std::string request);

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input)){
    };

    // Получение ключа запроса
    const json::Node& GetBaseRequest();
    const json::Node& GetStatRequest();
    const json::Node& GetRenderSettings();
    
    // Обрабока base_processing, заполнение транспортного каталога
    void ProcessBaseRequest (trans_cat::TransportCatalogue& catalogue);

    // Заполнение настроек рендера renderer_settings
    map_render::RenderSettings ProcessRenderSetting (const json::Dict& request);

private:
    json::Document input_;
    json::Node null_ = nullptr;

    // Заполнение каталога из json файла
    // Получение данных об остановке
    std::pair<std::string, geo::Coordinates> GetStopFromRequest (const json::Dict& request);
    
    // Получение данных о маршруте
    std::tuple<std::string, std::vector<domain::Stop*>, bool> GetBusFromRequest (trans_cat::TransportCatalogue& catalogue, const json::Dict& request);
    
    // Получение данных о расстояних между остановками
    void SetDistanceFromRequest (trans_cat::TransportCatalogue& catalogue, json::Array& request);
    
    // Получение списка остановок для маршрута
    std::vector<domain::Stop*> GetStopsForBusFromRequest (trans_cat::TransportCatalogue& catalogue, const json::Dict& request);
};

} // namespace json_reader
