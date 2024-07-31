#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace input_reader {

namespace request {
    
enum class RequestType {
    Bus,
    Stop,
    Unknown
};

RequestType GetRequestType (std::string_view request);

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

}

class InputReader {
public:
    // Парсит строку в структуру CommandDescription и сохраняет результат в commands_
    void ParseLine(std::string_view line);

    // Наполняет данными транспортный справочник, используя команды из commands_
    void ApplyCommands(trans_cat::TransportCatalogue& catalogue) const;

    // Чтение входного потока, наполнение транспортного каталога
    void ReadInput (std::istream& input, trans_cat::TransportCatalogue& catalogue);

private:
    std::vector<request::CommandDescription> commands_;
}; 

}
