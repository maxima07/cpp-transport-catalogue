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
		double route_geo_lenght = 0.0;
		double route_lenght = 0.0;
	};

	struct DistHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const;
	private:
		std::hash<const void*> dist_hasher_;
	};
		
	
	void AddBus (const std::string& bus_name, std::vector<Stop*>&& stops_for_bus); 				// Добавление в БД автобусов.
	void AddStop (const std::string& stop_name, geo::Coordinates stop_coord); 					// Добавление в БД остановок.
	void SetDistBetweenStops (const Stop* stop_from, const Stop* stop_to, size_t distance); 	// Добавление расстояния между двумя остановками

	Bus* GetBus (std::string_view bus_name) const;
	Stop* GetStop (std::string_view stop_name) const;
	BusStat GetBusProperty (std::string_view bus_name) const; 									// Запрос свойств для конкретного автобуса
	size_t GetDistBetweenStops (const Stop* stop_from, const Stop* stop_to) const; 				// Запрос расстояния между остановками
	const std::set<std::string>& GetStopProperty(std::string_view stop_name) const; 			// Запрос списка автобусов для конкретной остановки
	
private:
	std::deque<Bus> bus_data_;																	// БД автобусов
	std::deque<Stop> stop_data_;																// БД остановок
	std::unordered_map<std::string_view, Bus*> bus_directory_; 									// справочник автобусов
	std::unordered_map<std::string_view, Stop*> stop_directory_;								// Справочник для остановок
	std::unordered_map<std::string_view, std::set<std::string>> bus_list_for_stop_;				// Справочник автобусов для остановки
	std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, DistHasher> dist_directory_;// Справочник расстояний между остановками

	
	int GetBusAllStopCount (const TransportCatalogue::Bus& bus) const; 							// Запрос кол-ва всех остановок
	double GetBusGeoRouteLength (const TransportCatalogue::Bus& bus) const; 					// Запрос длины маршрута по географическим координатам
	double GetBusRouteLength (const TransportCatalogue::Bus& bus) const; 						// Запрос длины маршрута по фактическому пути
	int GetBusUniqStopCount (const TransportCatalogue::Bus& bus) const; 						// Запрос кол-ва уникальных остановок
	
	
	
};

}

