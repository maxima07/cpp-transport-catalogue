#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

struct Stop {
    std::string stop_name;
    geo::Coordinates stop_coord;
};

struct Bus {
    std::string bus_name;
    std::vector<Stop*> stops_for_bus;
    bool is_roundtrip;
};

struct BusStat{
    int all_stop_count 	= 0;
    int uniq_stop_count = 0;
    double route_geo_lenght = 0.0;
    int route_length = 0;
};

} // namespace Domain