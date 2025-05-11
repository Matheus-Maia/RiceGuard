
#include "fase.h"

namespace model::viab {
    Fase::Fase(std::string nome, double minT, double maxT, 
               double optMinT, double optMaxT, int durMin, int durMax)
        : nome(std::move(nome))
        , minT(minT)
        , maxT(maxT)
        , optMinT(optMinT)
        , optMaxT(optMaxT)
        , durMin(durMin)
        , durMax(durMax) {}
}