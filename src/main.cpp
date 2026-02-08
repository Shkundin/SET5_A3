#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <cstdint>
#include <string>
#include <iomanip>
#include "../include/random_stream_gen.h"
#include "../include/hash_func_gen.h"
#include "../include/hll.h"

static void write_csv(const std::string& path, const std::vector<std::vector<double>>& rows){
    std::ofstream out(path);
    out.setf(std::ios::fixed); out<<std::setprecision(6);
    for(const auto& r: rows){
        for(std::size_t i=0;i<r.size();i++){
            if(i) out<<',';
            out<<r[i];
        }
        out<<'\n';
    }
}

int main(){
    const int B=14;
    const int streams=30;
    const std::size_t n=200000;
    const int steps=20;
    std::vector<std::size_t> cuts;
    {
        RandomStreamGen tmp(n, 1, 0.25);
        cuts = tmp.split_steps(steps);
    }
    std::vector<std::vector<double>> per;
    per.reserve(streams*cuts.size());
    std::vector<std::vector<double>> ests(cuts.size()), pcks(cuts.size()), exs(cuts.size());
    for(int sid=0;sid<streams;sid++){
        RandomStreamGen gen(n, 1234567ULL + 101ULL*sid, 0.25);
        HashFuncGen hf(987654321ULL + 17ULL*sid);
        HyperLogLog32 hll(B);
        HyperLogLog32Packed hllp(B);
        std::unordered_set<std::string> exact;
        exact.reserve(n*2);
        std::size_t prev=0;
        for(std::size_t t=0;t<cuts.size();t++){
            std::size_t cur = cuts[t];
            for(std::size_t i=prev;i<cur;i++){
                const std::string& s = gen.stream()[i];
                exact.insert(s);
                std::uint32_t hx = hf(s);
                hll.add(hx);
                hllp.add(hx);
            }
            prev=cur;
            double e = double(exact.size());
            double h = hll.estimate();
            double hp = hllp.estimate();
            per.push_back({double(sid), double(t+1), double(cur), e, h, hp});
            ests[t].push_back(h);
            pcks[t].push_back(hp);
            exs[t].push_back(e);
        }
    }
    std::vector<std::vector<double>> stats;
    stats.reserve(cuts.size());
    for(std::size_t t=0;t<cuts.size();t++){
        auto mean_std = [](const std::vector<double>& v){
            double m=0.0;
            for(double x: v) m+=x;
            m/=double(v.size());
            double s=0.0;
            for(double x: v){ double d=x-m; s+=d*d; }
            s = std::sqrt(s/double(v.size()-1));
            return std::pair<double,double>(m,s);
        };
        auto [me,se] = mean_std(ests[t]);
        auto [mp,sp] = mean_std(pcks[t]);
        auto [mx,sx] = mean_std(exs[t]);
        stats.push_back({double(t+1), double(cuts[t]), mx, me, se, mp, sp});
    }
    std::vector<std::vector<double>> per_out;
    per_out.push_back({0,0,0,0,0,0});
    per_out.pop_back();
    {
        std::ofstream out("data/per_stream.csv");
        out<<"stream,step,processed,exact,est,est_packed\n";
        out.setf(std::ios::fixed); out<<std::setprecision(6);
        for(const auto& r: per){
            out<<std::uint64_t(r[0])<<','<<std::uint64_t(r[1])<<','<<std::uint64_t(r[2])<<','<<std::uint64_t(r[3])<<','<<r[4]<<','<<r[5]<<'\n';
        }
    }
    {
        std::ofstream out("data/stats.csv");
        out<<"step,processed,mean_exact,mean_est,std_est,mean_est_packed,std_est_packed\n";
        out.setf(std::ios::fixed); out<<std::setprecision(6);
        for(const auto& r: stats){
            out<<std::uint64_t(r[0])<<','<<std::uint64_t(r[1])<<','<<r[2]<<','<<r[3]<<','<<r[4]<<','<<r[5]<<','<<r[6]<<'\n';
        }
    }
    HyperLogLog32 hll(B);
    HyperLogLog32Packed hllp(B);
    std::ofstream out("data/memory.txt");
    out<<"B="<<B<<"\n";
    out<<"m="<<hll.registers()<<"\n";
    out<<"bytes_std="<<hll.bytes()<<"\n";
    out<<"bytes_packed="<<hllp.bytes()<<"\n";
    return 0;
}
