#include "transport_catalogue.h"

#include <unordered_set>

size_t trans_cat::TransportCatalogue::DistHasher::operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const {
    size_t h_first_stop = dist_hasher_(stops.first);
    size_t h_sec_stop = dist_hasher_(stops.second);
    return h_first_stop + h_sec_stop * 37;
}

void trans_cat::TransportCatalogue::AddBus (const std::string& bus_name, const std::vector<domain::Stop*>& stops_for_bus, bool is_roundtrip){
    bus_data_.push_back(domain::Bus{bus_name, stops_for_bus, is_roundtrip});
    bus_directory_[bus_data_.back().bus_name] = &bus_data_.back();

    for(const domain::Stop* stop : bus_data_.back().stops_for_bus){
        bus_list_for_stop_[stop->stop_name].insert(bus_data_.back().bus_name);
    }
}

void trans_cat::TransportCatalogue::SetDistBetweenStops (const domain::Stop* stop_from, const domain::Stop* stop_to, size_t distance){
    dist_directory_[std::make_pair(stop_from, stop_to)] = distance;
}
	
void trans_cat::TransportCatalogue::AddStop (const std::string& stop_name, geo::Coordinates stop_coord){
    stop_data_.push_back(domain::Stop{stop_name, stop_coord});
    stop_directory_[stop_data_.back().stop_name] = &stop_data_.back();
    bus_list_for_stop_[stop_data_.back().stop_name] = {};
}

domain::Bus* trans_cat::TransportCatalogue::GetBusByName (std::string_view bus_name) const {
    auto it = bus_directory_.find(bus_name);
    
    if(it != bus_directory_.end()){
        return it -> second;
    }

    return nullptr;
}
	
domain::Stop* trans_cat::TransportCatalogue::GetStopByName (std::string_view stop_name) const {
    auto it = stop_directory_.find(stop_name);

    if(it != stop_directory_.end()){
        return it->second;
    }

    return nullptr;
}

domain::BusStat trans_cat::TransportCatalogue::GetBusPropertyByName (std::string_view bus_name) const {
    // domain::BusStat res;
    const domain::Bus* bus = GetBusByName(bus_name);

    if(!bus){
        return {0, 0, 0.0, 0};
    } 

    int all_stop_count      = GetBusAllStopCount(*bus);
    int uniq_stop_count     = GetBusUniqStopCount(*bus);
    double route_geo_lenght = GetBusGeoRouteLength(*bus);
    int route_lenght        = GetBusRouteLength(*bus);

    return {all_stop_count, uniq_stop_count, route_geo_lenght, route_lenght};
}

const std::set<std::string>& trans_cat::TransportCatalogue::GetStopPropertyByName(std::string_view stop_name) const {
    static std::set<std::string> result;
    auto it = bus_list_for_stop_.find(stop_name);

    if(it != bus_list_for_stop_.end()){
        result = it -> second;
    }

    return result;
}

size_t trans_cat::TransportCatalogue::GetDistBetweenStops (const domain::Stop* stop_from, const domain::Stop* stop_to) const {
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

std::map<std::string_view, domain::Bus*> trans_cat::TransportCatalogue::GetSortedBusDirectory () const {
    std::map<std::string_view, domain::Bus*> result;

    for (const auto& bus : bus_directory_) {
        result.emplace(bus);
    }

    return result;
}

int trans_cat::TransportCatalogue::GetBusUniqStopCount(const domain::Bus& bus) const {
    std::unordered_set<std::string> stops;

    for(const auto& stop : bus.stops_for_bus){
        stops.insert(stop->stop_name);
    }

    return static_cast<int>(stops.size());
}

int trans_cat::TransportCatalogue::GetBusAllStopCount(const domain::Bus& bus) const {
    int result;
    
    if (bus.is_roundtrip) {
        result = static_cast<int>(bus.stops_for_bus.size());
    } else {
        result = static_cast<int>(bus.stops_for_bus.size() * 2 - 1);
    }
    return result;
}

double trans_cat::TransportCatalogue::GetBusGeoRouteLength(const domain::Bus& bus) const {
    double result = 0.0;

    for(size_t i = 1; i < bus.stops_for_bus.size(); ++i){
        if (bus.is_roundtrip) {
            result += geo::ComputeDistance(bus.stops_for_bus[i - 1]->stop_coord, bus.stops_for_bus[i]->stop_coord);
        } else {
            result += geo::ComputeDistance(bus.stops_for_bus[i - 1]->stop_coord, bus.stops_for_bus[i]->stop_coord) * 2;
        }
    }
    return result;
}

int trans_cat::TransportCatalogue::GetBusRouteLength (const domain::Bus& bus) const {
    int result = 0;    

    for(size_t i = 0; i < bus.stops_for_bus.size() - 1; ++i){
        result += static_cast<int>(GetDistBetweenStops(bus.stops_for_bus[i], bus.stops_for_bus[i + 1]));
    }
    if (!bus.is_roundtrip) {
        for(size_t j = bus.stops_for_bus.size() - 1; j > 0; --j){
            result += static_cast<int>(GetDistBetweenStops(bus.stops_for_bus[j], bus.stops_for_bus[j - 1]));
        }
    }

    return result;
} 
