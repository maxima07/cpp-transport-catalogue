#include "transport_catalogue.h"

#include <unordered_set>

// Хеширование дистанции между остановками
size_t trans_cat::TransportCatalogue::DistHasher::operator()(const std::pair<const Stop*, const Stop*>& stops) const {
    size_t h_first_stop = dist_hasher_(stops.first);
    size_t h_sec_stop = dist_hasher_(stops.second);
    return h_first_stop + h_sec_stop * 37;
}

// добавление в БД автобусов и остановок.
void trans_cat::TransportCatalogue::AddBus (const std::string& bus_name, std::vector<Stop*>&& stops_for_bus){
    bus_data_.push_back(Bus{bus_name, std::move(stops_for_bus)});   // Добавление в БД автобуса
    bus_directory_[bus_data_.back().bus_name] = &bus_data_.back();  // Обновление справочника автобуса

    for(const Stop* stop : bus_data_.back().stops_for_bus){
        bus_list_for_stop_[stop->stop_name].insert(bus_data_.back().bus_name);  // Добавление автобуса в справочник для конкретной остановки
    }
}

// Добавление расстояния между двумя остановками
void trans_cat::TransportCatalogue::SetDistBetweenStops (const Stop* stop_from, const Stop* stop_to, size_t distance){
    dist_directory_[std::make_pair(stop_from, stop_to)] = distance;
}
	
void trans_cat::TransportCatalogue::AddStop (const std::string& stop_name, geo::Coordinates stop_coord){
    stop_data_.push_back(Stop{stop_name, stop_coord});                  // Добавление в БД остановки
    stop_directory_[stop_data_.back().stop_name] = &stop_data_.back();  // Обновление справочника остановок
    bus_list_for_stop_[stop_data_.back().stop_name] = {};               // Обновление справочника для конкретной останови
}

// Запросы к БД
trans_cat::TransportCatalogue::Bus* trans_cat::TransportCatalogue::GetBus (std::string_view bus_name) const {
    auto it = bus_directory_.find(bus_name);
    
    if(it != bus_directory_.end()){
        return it -> second;
    }

    return nullptr;
}
	
trans_cat::TransportCatalogue::Stop* trans_cat::TransportCatalogue::GetStop (std::string_view stop_name) const {
    auto it = stop_directory_.find(stop_name);

    if(it != stop_directory_.end()){
        return it->second;
    }

    return nullptr;
}

trans_cat::TransportCatalogue::BusStat trans_cat::TransportCatalogue::GetBusProperty (std::string_view bus_name) const {
    const Bus* bus = GetBus(bus_name);

    if(!bus){
        return {0, 0, 0.0, 0.0};
    }

    return {GetBusAllStopCount(*bus), GetBusUniqStopCount(*bus), GetBusGeoRouteLength(*bus), GetBusRouteLength(*bus)};
}

const std::set<std::string>& trans_cat::TransportCatalogue::GetStopProperty(std::string_view stop_name) const {
    static std::set<std::string> result;
    auto it = bus_list_for_stop_.find(stop_name);

    if(it != bus_list_for_stop_.end()){
        result = it -> second;
    }

    return result;
}

size_t trans_cat::TransportCatalogue::GetDistBetweenStops (const Stop* stop_from, const Stop* stop_to) const {
    auto it = dist_directory_.find({stop_from, stop_to});
    size_t result = 0;

    if(it != dist_directory_.end()){
        result = it->second;
    } else {
        it = dist_directory_.find({stop_to, stop_from});

        if(it != dist_directory_.end()){
            result = it->second;
        }
    }

    return result;
}

int trans_cat::TransportCatalogue::GetBusUniqStopCount(const TransportCatalogue::Bus& bus) const {
    std::unordered_set<std::string> stops;

    for(const auto& stop : bus.stops_for_bus){
        stops.insert(stop->stop_name);
    }

    return static_cast<int>(stops.size());
}

int trans_cat::TransportCatalogue::GetBusAllStopCount(const TransportCatalogue::Bus& bus) const {
    return static_cast<int>(bus.stops_for_bus.size());
}

double trans_cat::TransportCatalogue::GetBusGeoRouteLength(const TransportCatalogue::Bus& bus) const {
    double result = 0.0;

    for(size_t i = 1; i < bus.stops_for_bus.size(); ++i){
        result += geo::ComputeDistance(bus.stops_for_bus[i - 1]->stop_coord, bus.stops_for_bus[i]->stop_coord);
    }

    return result;
}

double trans_cat::TransportCatalogue::GetBusRouteLength (const TransportCatalogue::Bus& bus) const {
    double result = 0.0;

    for(size_t i = 0; i < bus.stops_for_bus.size() - 1; ++i){
        result += static_cast<double>(GetDistBetweenStops(bus.stops_for_bus[i], bus.stops_for_bus[i + 1]));
    }

    return result;
} 