#pragma once
#include <string>
#include <vector>
#include <random>
#include <cstdint>
#include <algorithm>

class RandomStreamGen {
    std::vector<std::string> s;
    std::mt19937_64 rng;
    double repeat_p;
    static constexpr const char* alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
    std::string gen_one() {
        std::uniform_int_distribution<int> len_d(1, 30);
        std::uniform_int_distribution<int> ch_d(0, 62);
        int len = len_d(rng);
        std::string x;
        x.reserve(len);
        for(int i=0;i<len;i++) x.push_back(alphabet[ch_d(rng)]);
        return x;
    }
public:
    RandomStreamGen(std::size_t n, std::uint64_t seed, double repeat_prob=0.2): rng(seed), repeat_p(repeat_prob) {
        s.reserve(n);
        std::uniform_real_distribution<double> p(0.0, 1.0);
        for(std::size_t i=0;i<n;i++){
            if(!s.empty() && p(rng)<repeat_p){
                std::uniform_int_distribution<std::size_t> pick(0, s.size()-1);
                s.push_back(s[pick(rng)]);
            }else{
                s.push_back(gen_one());
            }
        }
    }
    const std::vector<std::string>& stream() const { return s; }
    std::vector<std::size_t> split_steps(int steps) const {
        std::vector<std::size_t> idx;
        idx.reserve(steps);
        for(int k=1;k<=steps;k++){
            double f = double(k)/double(steps);
            idx.push_back(std::size_t(std::llround(f*double(s.size()))));
        }
        idx.back() = s.size();
        for(std::size_t& v: idx) if(v> s.size()) v = s.size();
        std::sort(idx.begin(), idx.end());
        idx.erase(std::unique(idx.begin(), idx.end()), idx.end());
        return idx;
    }
};
