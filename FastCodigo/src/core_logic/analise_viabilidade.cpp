#include "viab/analise_viabilidade.h"
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <stdexcept>
#include <limits>
#include <climits>
#include <cstring>
namespace viab {

struct AnalysisConfig {
    static constexpr long long LIMITE_COMBINACOES = 10000000;
    static constexpr double TMAX_PEN_THR=31.0;
    static constexpr double TMIN_PEN_THR=21.0;
    static constexpr double ESBRANQ_THR=30.0;
    static constexpr double RED_THR=27.0;
};

bool dentro(double x,double lo,double hi){return x>=lo&&x<=hi;}
bool proxima_combinacao(std::vector<int>& e,const std::vector<Fase>& f){
    for(int i=e.size()-1;i>=0;--i){
        if(e[i]+1<=f[i].durMax){e[i]++;for(size_t j=i+1;j<e.size();++j)e[j]=f[j].durMin;return true;}
    }
    return false;
}
static bool avaliar_path(const std::vector<Dia>& dias,size_t si,const std::vector<Fase>& f,const std::vector<int>& c,const AnalysisConfig& cfg,double& pd,double& pn,bool& esb,bool& red,bool& ideal){
    pd=pn=0.0;esb=red=false;ideal=true;int off=0;
    for(size_t p=0;p<f.size();++p){
        const auto& ph=f[p];
        for(int d=0;d<c[p];++d){
            size_t idx=si+off+d; if(idx>=dias.size()) return false;
            auto& D=dias[idx];
            if(!dentro(D.tmax,ph.minT,ph.maxT)||!dentro(D.tmin,ph.minT,ph.maxT))return false;
            pd+=std::max(0.0,D.tmax-cfg.TMAX_PEN_THR)*0.06; pn+=std::max(0.0,D.tmin-cfg.TMIN_PEN_THR)*0.10;
            if(!dentro(D.tmax,ph.optMinT,ph.optMaxT)||!dentro(D.tmin,ph.optMinT,ph.optMaxT)) ideal=false;
            if(ph.nome=="Maturação"){if(D.tmax>cfg.ESBRANQ_THR)esb=true; if(D.tmin>cfg.RED_THR)red=true;}
        }
        off+=c[p];
    }
    return true;
}
std::vector<ResultadoData> rodar_analise(const std::vector<Dia>& dias,const std::vector<Fase>& fases){
    size_t N=dias.size(); std::vector<ResultadoData> res(N);
    if(N==0||fases.empty())return res;
    long long tot=1; for(auto& ph:fases){ if(ph.durMin>ph.durMax)throw std::invalid_argument("durMin>durMax "+ph.nome);
        long long opts=(long long)ph.durMax-ph.durMin+1;
        if(tot>LLONG_MAX/opts)throw std::overflow_error("overflow"); tot*=opts;
        if(tot>AnalysisConfig::LIMITE_COMBINACOES) throw std::overflow_error("limite combos"); }
    int min_days=0; for(auto& ph:fases)min_days+=ph.durMin;

    std::vector<int> durMin(fases.size()); for(size_t i=0;i<fases.size();++i) durMin[i]=fases[i].durMin;

    #pragma omp parallel
    {
        std::vector<int> combo(fases.size());
        #pragma omp for schedule(guided)
        for(size_t i=0;i<N;++i){ if((int)N-(int)i<min_days) continue;
            memcpy(combo.data(),durMin.data(),combo.size()*sizeof(int));
            long long cv=0,ce=0,cr=0,co=0; double sy=0;
            do{ double pd,pn; bool esb,red,ideal;
                if(avaliar_path(dias,i,fases,combo,AnalysisConfig(),pd,pn,esb,red,ideal)){
                    cv++; sy+=std::max(0.0,1.0-(pd+pn)); ce+=esb; cr+=red; co+=ideal;
                }
            }while(proxima_combinacao(combo,fases));
            auto& R=res[i]; R.data_str=dias[i].data_str; R.total_caminhos=tot; R.caminhos_viaveis=cv;
            if(tot) R.prob_viabilidade=(double)cv/tot;
            if(cv){ R.rendimento_medio=sy/cv; R.prob_esbranquiamento=(double)ce/cv;
                    R.prob_reducao_moagem=(double)cr/cv; R.prob_optimo=(double)co/cv; }
        }
    }
    return res;
}
} // namespace viab