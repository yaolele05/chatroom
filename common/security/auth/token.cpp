#include "token.h"
#include <iomanip>
#include <random>
#include <sstream>
std::string Token::generate()
{
    static thread_local std::mt19937_64 engine(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
    uint64_t high=dist(engine);
    uint64_t low=dist(engine);

    std::ostringstream oss;
    oss<<std::hex<<std::setw(16)<<std::setfill('0')<<high
    <<std::setw(16)<<std::setfill('0')<<low;
    return oss.str();
}