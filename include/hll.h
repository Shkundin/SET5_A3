#ifdef _MSC_VER
#include <intrin.h>
#endif
#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>
static inline uint32_t clz32(uint32_t x) {
#ifdef _MSC_VER
    if (x == 0) return 32;
    unsigned long idx;
    _BitScanReverse(&idx, x);
    return 31u - static_cast<uint32_t>(idx);
#else
    return x ? static_cast<uint32_t>(clz32(x)) : 32u;
#endif
}

class HyperLogLog32 {
    int B;
    std::uint32_t m;
    std::vector<std::uint8_t> M;
    static double alpha(std::uint32_t m){
        if(m==16) return 0.673;
        if(m==32) return 0.697;
        if(m==64) return 0.709;
        return 0.7213/(1.0+1.079/double(m));
    }
public:
    explicit HyperLogLog32(int b): B(b), m(1u<<b), M(m,0) {}
    void add(std::uint32_t x){
        std::uint32_t idx = x>>(32-B);
        std::uint32_t w = x<<B;
        int rho = w? (clz32(w)+1) : (32-B+1);
        std::uint8_t v = (std::uint8_t)rho;
        if(v>M[idx]) M[idx]=v;
    }
    double estimate() const{
        double sum=0.0;
        std::uint32_t V=0;
        for(std::uint8_t v: M){
            if(v==0) V++;
            sum += std::ldexp(1.0, -int(v));
        }
        double em = alpha(m)*double(m)*double(m)/sum;
        if(em<=2.5*double(m) && V>0){
            em = double(m)*std::log(double(m)/double(V));
        }else{
            const double two32 = 4294967296.0;
            if(em>(two32/30.0)){
                em = -two32*std::log(1.0-em/two32);
            }
        }
        return em;
    }
    std::uint32_t registers() const { return m; }
    std::size_t bytes() const { return M.size()*sizeof(std::uint8_t); }
    void reset(){ std::fill(M.begin(), M.end(), 0); }
};

class HyperLogLog32Packed {
    int B;
    std::uint32_t m;
    std::vector<std::uint64_t> bits;
    static double alpha(std::uint32_t m){
        if(m==16) return 0.673;
        if(m==32) return 0.697;
        if(m==64) return 0.709;
        return 0.7213/(1.0+1.079/double(m));
    }
    static constexpr int W=6;
    std::uint8_t get(std::uint32_t i) const{
        std::uint64_t pos = std::uint64_t(i)*W;
        std::uint64_t w = pos>>6;
        int off = int(pos&63);
        std::uint64_t x = bits[w]>>off;
        if(off>64-W) x |= bits[w+1]<<(64-off);
        return std::uint8_t(x & ((1ULL<<W)-1));
    }
    void set(std::uint32_t i, std::uint8_t v){
        std::uint64_t pos = std::uint64_t(i)*W;
        std::uint64_t w = pos>>6;
        int off = int(pos&63);
        std::uint64_t mask = ((1ULL<<W)-1);
        bits[w] &= ~(mask<<off);
        bits[w] |= (std::uint64_t(v)&mask)<<off;
        if(off>64-W){
            int r = 64-off;
            bits[w+1] &= ~((mask>>r));
            bits[w+1] |= (std::uint64_t(v)&mask)>>r;
        }
    }
public:
    explicit HyperLogLog32Packed(int b): B(b), m(1u<<b) {
        std::uint64_t total = std::uint64_t(m)*W;
        std::size_t words = std::size_t((total+63)/64)+1;
        bits.assign(words,0);
    }
    void add(std::uint32_t x){
        std::uint32_t idx = x>>(32-B);
        std::uint32_t w = x<<B;
        int rho = w? (clz32(w)+1) : (32-B+1);
        std::uint8_t cur = get(idx);
        std::uint8_t v = (std::uint8_t)rho;
        if(v>cur) set(idx,v);
    }
    double estimate() const{
        double sum=0.0;
        std::uint32_t V=0;
        for(std::uint32_t i=0;i<m;i++){
            std::uint8_t v = get(i);
            if(v==0) V++;
            sum += std::ldexp(1.0, -int(v));
        }
        double em = alpha(m)*double(m)*double(m)/sum;
        if(em<=2.5*double(m) && V>0){
            em = double(m)*std::log(double(m)/double(V));
        }else{
            const double two32 = 4294967296.0;
            if(em>(two32/30.0)){
                em = -two32*std::log(1.0-em/two32);
            }
        }
        return em;
    }
    std::uint32_t registers() const { return m; }
    std::size_t bytes() const { return bits.size()*sizeof(std::uint64_t); }
};
