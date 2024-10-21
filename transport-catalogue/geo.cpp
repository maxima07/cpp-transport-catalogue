#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

}  // namespace geo