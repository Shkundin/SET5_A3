#pragma once
#include <cstdint>
#include <string>
#include <random>

class HashFuncGen {
    std::uint64_t a,b,base;
    static std::uint64_t splitmix64(std::uint64_t x){
        x+=0x9e3779b97f4a7c15ULL;
        x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
        x=(x^(x>>27))*0x94d049bb133111ebULL;
        return x^(x>>31);
    }
public:
    explicit HashFuncGen(std::uint64_t seed){
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<std::uint64_t> d;
        a = d(rng)|1ULL;
        b = d(rng);
        base = (d(rng)|1ULL)%((1ULL<<63)-1);
        if(base<1315423911ULL) base+=1315423911ULL;
    }
    std::uint32_t operator()(const std::string& x) const {
        std::uint64_t h=0;
        for(unsigned char c: x) h = h*base + (std::uint64_t)c + 1ULL;
        h = a*h + b;
        h = splitmix64(h);
        return (std::uint32_t)h;
    }
};
