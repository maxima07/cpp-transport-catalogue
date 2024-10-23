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

    // std::string file ("D:\\DEV\\Sprint 10\\S10T1L15_3\\build\\Debug\\s10_final_opentest_1.json"s);

    // std::ifstream inputFile(file, std::ios::in);
    
    // if (!inputFile.is_open())
    //     std::cout << "failed to open " << file << '\n';
    
    // std::cin.rdbuf (inputFile.rdbuf());

    trans_cat::TransportCatalogue tc;
    json_reader::JsonReader reader(std::cin);

    reader.ProcessBaseRequest(tc);

    const auto& stat_request = reader.GetStatRequest ();
    const auto& settings = reader.GetRenderSettings().AsMap();
    const auto& render = reader.ProcessRenderSetting(settings);

    req_handl::RequestHandler rh(tc, render);
    rh.ProcessStatRequest (stat_request);
    // rh.MapRender().Render(std::cout); 
}