#pragma once

#include <cmath>

namespace geo {
    
struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
    
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;

};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    // using namespace std;
    const int EARTH_RAD = 6371000;
    static const double dr = M_PI / 180.;
    
    if (from == to) {
        return 0;
    }

    return acos(sin(from.lat * dr) * sin(to.lat * dr)
        + cos(from.lat * dr) * cos(to.lat * dr) * cos(std::abs(from.lng - to.lng) * dr))
        * EARTH_RAD;
}

} // namespace geo