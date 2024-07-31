#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    trans_cat::TransportCatalogue catalogue;

    {
        input_reader::InputReader reader;
        reader.ReadInput(cin, catalogue);
    }

    {
        stat_reader::ReadAndProcessRequest(cin, cout, catalogue);
    }
}