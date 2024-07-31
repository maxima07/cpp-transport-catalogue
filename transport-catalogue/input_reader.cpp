#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>


input_reader::request::RequestType input_reader::request::GetRequestType (std::string_view request){
    if(request == "Stop"){
        return RequestType::Stop;
    } else if (request == "Bus"){
        return RequestType::Bus;
    }

    return RequestType::Unknown;
}

// Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

// Удаляет пробелы в начале и конце строки
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

// Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }

        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }

        pos = delim_pos + 1;
    }

    return result;
}


// Парсит маршрут.
// Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
// Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

input_reader::request::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void input_reader::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void input_reader::InputReader::ApplyCommands([[maybe_unused]] trans_cat::TransportCatalogue& catalogue) const {
    std::vector<input_reader::request::CommandDescription> buffer;
    
    for(const input_reader::request::CommandDescription& comand : commands_){
        switch (input_reader::request::GetRequestType(comand.command)){
            case input_reader::request::RequestType::Stop : {
                catalogue.AddStop(comand.id, ParseCoordinates(comand.description));\
                break;
            }
            case input_reader::request::RequestType::Bus : {
                buffer.push_back(comand);
                break;
            }
            case input_reader::request::RequestType::Unknown : {
                break;
            }
        }
    }

    for(input_reader::request::CommandDescription comand : buffer){
        std::vector<std::string_view> stop_names = ParseRoute(comand.description);
        std::vector<trans_cat::TransportCatalogue::Stop*> stops_for_bus;
        
        for(const std::string_view stop_name : stop_names){
            if(trans_cat::TransportCatalogue::Stop* stop = catalogue.GetStop(stop_name)){
                stops_for_bus.push_back(stop);
            }
        }

        catalogue.AddBus(comand.id, std::move(stops_for_bus));
    }
}

void input_reader::InputReader::ReadInput (std::istream& input, trans_cat::TransportCatalogue& catalogue){
    int base_request_count;
    input >> base_request_count >> std::ws;
    
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseLine(line);
    }

    ApplyCommands(catalogue);
}