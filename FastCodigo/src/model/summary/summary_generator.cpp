#include "summary_generator.h"
#include <sstream>
#include <map>

namespace model::summary {
std::string gerar_csv_detalhado(const std::vector<viab::ResultadoData>& R){
    std::ostringstream o; o<<"Data,probabilidade_viabilidade,rendimento_medio,prob_esbranquiamento,prob_reducao_moagem,prob_optimo,total_caminhos,caminhos_viaveis\n";
    for(auto& r:R) o<<r.data_str<<","<<r.prob_viabilidade<<","<<r.rendimento_medio
        <<","<<r.prob_esbranquiamento<<","<<r.prob_reducao_moagem
        <<","<<r.prob_optimo<<","<<r.total_caminhos
        <<","<<r.caminhos_viaveis<<"\n";
    return o.str();
}

std::string gerar_csv_resumo_mensal(const std::vector<viab::ResultadoData>& R,const std::vector<viab::Dia>& D){
    std::ostringstream o; o<<"Mês,probabilidade_viabilidade_media,rendimento_medio,prob_esbranquiamento_media,prob_reducao_moagem_media,probabilidade_optimo_media\n";
    std::map<int,std::vector<const viab::ResultadoData*>> m;
    for(size_t i=0;i<R.size();++i) m[D[i].mes].push_back(&R[i]);
    for(auto& [mes,vec]:m){ double pv=0,rm=0,es=0,re=0,op=0;
        for(auto p:vec){pv+=p->prob_viabilidade;rm+=p->rendimento_medio;
            es+=p->prob_esbranquiamento;re+=p->prob_reducao_moagem;op+=p->prob_optimo;}
        int c=vec.size(); o<<mes<<","<<pv/c<<","<<rm/c<<","<<es/c<<","<<re/c<<","<<op/c<<"\n";
    }
    return o.str();
}
} // namespace model::summary
