#include "transport_catalogue.h"

#include <unordered_set>

// добавление в БД автобусов и остановок.
void trans_cat::TransportCatalogue::AddBus (const std::string& bus_name, std::vector<Stop*>&& stops_for_bus){
    bus_data_.push_back(Bus{bus_name, std::move(stops_for_bus)});   // Добавление в БД автобуса
    bus_directory_[bus_data_.back().bus_name] = &bus_data_.back();  // Обновление справочника автобуса

    for(const Stop* stop : bus_data_.back().stops_for_bus){
        bus_list_for_stop_[stop->stop_name].insert(bus_data_.back().bus_name);  // Добавление автобуса в справочник для конкретной остановки
    }
}
	
void trans_cat::TransportCatalogue::AddStop (const std::string& stop_name, geo::Coordinates stop_coord){
    stop_data_.push_back(Stop{stop_name, stop_coord});                  // Добавление в БД остановки
    stop_directory_[stop_data_.back().stop_name] = &stop_data_.back();  // Обновление справочника остановок
    bus_list_for_stop_[stop_data_.back().stop_name] = {};               // Обновление справочника для конкретной останови
}

// Запросы к БД
trans_cat::TransportCatalogue::Bus* trans_cat::TransportCatalogue::GetBus (std::string_view bus_name) const {
    if(bus_directory_.count(bus_name) > 0){
        return bus_directory_.at(bus_name);
    }

    return nullptr;
}
	
trans_cat::TransportCatalogue::Stop* trans_cat::TransportCatalogue::GetStop (std::string_view stop_name) const {
    if(stop_directory_.count(stop_name) > 0){
        return stop_directory_.at(stop_name);
    }

    return nullptr;
}

trans_cat::TransportCatalogue::BusStat trans_cat::TransportCatalogue::GetBusProperty (std::string_view bus_name) const {
    const Bus* bus = GetBus(bus_name);

    if(!bus){
        return {0, 0, 0.0};
    }

    return {GetBusAllStopCount(*bus), GetBusUniqStopCount(*bus), GetBusRouteLength(*bus)};
}

std::set<std::string> trans_cat::TransportCatalogue::GetStopProperty(std::string_view stop_name) const {

    if(!bus_list_for_stop_.count(stop_name)){
        return {};
    }

    return bus_list_for_stop_.at(stop_name);
}

int trans_cat::GetBusUniqStopCount(const TransportCatalogue::Bus& bus){
    std::unordered_set<std::string> stops;

    for(const auto& stop : bus.stops_for_bus){
        stops.insert(stop->stop_name);
    }

    return static_cast<int>(stops.size());
}

int trans_cat::GetBusAllStopCount(const TransportCatalogue::Bus& bus){
    return static_cast<int>(bus.stops_for_bus.size());
}

double trans_cat::GetBusRouteLength(const TransportCatalogue::Bus& bus){
    double result = 0.0;

    for(size_t i = 1; i < bus.stops_for_bus.size(); ++i){
        result += geo::ComputeDistance(bus.stops_for_bus[i - 1]->stop_coord, bus.stops_for_bus[i]->stop_coord);
    }

    return result;
}