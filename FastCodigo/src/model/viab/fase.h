#pragma once
#include <string>

namespace model::viab {
    struct Fase {
        Fase(
                std::string nome,
                double minT,
                double maxT,
                double optMinT,
                double optMaxT,
                int durMin,
                int durMax
        );
        
        std::string nome;
        double minT;
        double maxT;
        double optMinT;
        double optMaxT;
        int durMin;
        int durMax;
    };
} // namespace model::viab
