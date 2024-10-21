#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "domain.h"
#include "geo.h"

namespace trans_cat {

class TransportCatalogue {
public:
	struct DistHasher {
	public:
		size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const;
	private:
		std::hash<const void*> dist_hasher_;
	};
	
	// Добавление автобусов / остановок / дистанций между остановками в БД
	void AddBus (const std::string& bus_name, const std::vector<domain::Stop*>& stops_for_bus, bool is_roundtrip);
	void AddStop (const std::string& stop_name, geo::Coordinates stop_coord);
	void SetDistBetweenStops (const domain::Stop* stop_from, const domain::Stop* stop_to, size_t distance);

	// Получение информации из БД

	// Получение автобуса / остановки по имени
	domain::Bus* GetBusByName (std::string_view bus_name) const;
	domain::Stop* GetStopByName (std::string_view stop_name) const;
	
	// Запрос свойств для конкретного автобуса
	domain::BusStat GetBusPropertyByName (std::string_view bus_name) const;

	// Запрос свойств для конкретной остановки
	const std::set<std::string>& GetStopPropertyByName(std::string_view stop_name) const;

	// Получение расстояния между остановками
	size_t GetDistBetweenStops (const domain::Stop* stop_from, const domain::Stop* stop_to) const;
	
	std::map<std::string_view, domain::Bus*> GetSortedBusDirectory () const;

private:
	// БД автобусов и остановок
	std::deque<domain::Bus> bus_data_;
	std::deque<domain::Stop> stop_data_;

	// справочник автобусов / остановок
	std::unordered_map<std::string_view, domain::Bus*> bus_directory_;
	std::unordered_map<std::string_view, domain::Stop*> stop_directory_;

	// Справочник автобусов для остановки
	std::unordered_map<std::string_view, std::set<std::string>> bus_list_for_stop_;

	// Справочник расстояний между остановками
	std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, size_t, DistHasher> dist_directory_;
	
	int 	GetBusAllStopCount 	 (const domain::Bus& bus) const;
	double	GetBusGeoRouteLength (const domain::Bus& bus) const;
	int 	GetBusRouteLength 	 (const domain::Bus& bus) const;
	int 	GetBusUniqStopCount  (const domain::Bus& bus) const;
};

}

