#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {

void PrintBusProperty(const trans_cat::TransportCatalogue& tansport_catalogue, std::string_view bus_id, std::ostream& output);

void PrintStopProperty(const trans_cat::TransportCatalogue& tansport_catalogue, std::string_view stop_id, std::ostream& output);

void ParseAndPrintStat(const trans_cat::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output);

}



