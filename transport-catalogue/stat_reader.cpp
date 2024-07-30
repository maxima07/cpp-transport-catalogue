#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>
#include <fstream>

void stat_reader::PrintBusProperty(const trans_cat::TransportCatalogue& transport_catalogue, std::string_view bus_id, std::ostream& output){
    using namespace std::string_literals;
    const trans_cat::TransportCatalogue::BusStat bus_stat = transport_catalogue.GetBusProperty(bus_id);
    
    if(bus_stat.all_stop_count == 0){
        output <<  "Bus "s << bus_id << ": not found\n"s;
        return;
    }

    output  << "Bus "s << bus_id << ": "s
            << bus_stat.all_stop_count  << " stops on route, "s
            << bus_stat.uniq_stop_count << " unique stops, "s
            << bus_stat.route_lenght    << " route length\n"s;
}

void stat_reader::PrintStopProperty(const trans_cat::TransportCatalogue& tansport_catalogue, std::string_view stop_id, std::ostream& output){
    using namespace std::string_literals;
    trans_cat::TransportCatalogue::Stop* stop = tansport_catalogue.GetStop(stop_id);
    std::set<std::string> bus_list = tansport_catalogue.GetStopProperty(stop_id);

    if(stop == nullptr){
        output << "Stop "s << stop_id << ": not found\n";
        return;
    }
    
    if(bus_list.empty()){
        output << "Stop "s << stop_id << ": no buses\n"s;
    } else {
        output << "Stop "s << stop_id << ": buses";

        for (const std::string& bus : bus_list){
            output << " " << bus;
        }

        output << "\n";
    }
}

void stat_reader::ParseAndPrintStat(const trans_cat::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    using namespace std::string_literals;
    auto wc_pos = request.find(' ');

    if(wc_pos == std::string_view::npos){
        output << "Invalid request format\n"s;
        return;
    }

    std::string_view request_type   = request.substr(0, wc_pos);
    std::string_view request_id     = request.substr(wc_pos + 1, request.size());

    switch (input_reader::request::GetRequestType(request_type)){
        case input_reader::request::RequestType::Bus : {
            PrintBusProperty (transport_catalogue, request_id, output);
            break;
        }

        case input_reader::request::RequestType::Stop : {
            PrintStopProperty (transport_catalogue, request_id, output);
            break;
        }

        case input_reader::request::RequestType::Unknown : {
            break;
        }
    }
}


