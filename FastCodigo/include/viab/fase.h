#pragma once
#include <string>
namespace viab {
struct Fase {
    std::string nome;
    double minT;
    double maxT;
    double optMinT;
    double optMaxT;
    int durMin;
    int durMax;
};
} // namespace viab