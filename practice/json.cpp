#include <iostream>
#include <nlohmann/json.hpp>

/*
using json=nlohmann::json;
int main()
{
    json j;
    j["type"]="login";
    j["username"]="lele";
    j["password"]="123456";
    std::cout<<j.dump(4)<<std::endl;//这句的意思是将json对象j转换成字符串并输出到控制台。dump()函数会将json对象序列化为一个字符串，默认情况下会以紧凑的格式输出，即没有多余的空格和换行符。如果需要更美观的格式，可以传递一个参数来指定缩进的空格数，例如j.dump(4)会使用4个空格进行缩进。

    return 0;
}
*/
using json=nlohmann::json;
int main()
{
    json j;
    j["type"]="login";
    j["username"]="lele";
    j["password"]=123456;
    std::cout<<j.dump(4)<<std::endl;
    return 0;
}