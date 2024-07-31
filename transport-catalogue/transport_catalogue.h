#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace trans_cat {

class TransportCatalogue {
public:
	struct Stop {
		std::string stop_name;
		geo::Coordinates stop_coord;
	};
	
	struct Bus {
		std::string bus_name;
		std::vector<Stop*> stops_for_bus;
	};

	struct BusStat{
		int all_stop_count 	= 0;
		int uniq_stop_count = 0;
		double route_lenght = 0.0;
	};
	
	
	// добавление в БД автобусов и остановок.
	void AddBus (const std::string& bus_name, std::vector<Stop*>&& stops_for_bus);
	void AddStop (const std::string& stop_name, geo::Coordinates stop_coord);

	// Запросы к БД
	Bus* GetBus (std::string_view bus_name) const;
	Stop* GetStop (std::string_view stop_name) const;
	
	// Запрос свойств для конкретного автобуса
	BusStat GetBusProperty (std::string_view bus_name) const;
	
	// Запрос списка автобусов для конкретной остановки
	const std::set<std::string>& GetStopProperty(std::string_view stop_name) const;

private:
	// БД для автобусов и остановок
	std::deque<Bus> bus_data_;
	std::deque<Stop> stop_data_;

	// Справочники для автобусов и остановок
	std::unordered_map<std::string_view, Bus*> bus_directory_;
	std::unordered_map<std::string_view, Stop*> stop_directory_;
	
	// Справочник автобусов для остановки
	std::unordered_map<std::string_view, std::set<std::string>> bus_list_for_stop_;

	// Запрос кол-ва уникальных остановок
	int GetBusUniqStopCount (const TransportCatalogue::Bus& bus) const;

	// Запрос кол-ва всех остановок
	int GetBusAllStopCount (const TransportCatalogue::Bus& bus) const;

	// Запрос длины маршрута
	double GetBusRouteLength (const TransportCatalogue::Bus& bus) const;
};



}

