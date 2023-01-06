
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>


int main(){
    
    std::stringstream stream;
    stream<<"salam\n";
    std::cout<<stream.str();
    stream.str(std::string());
    stream<<"salam\n";
    std::cout<<stream.str();
    return 0;
}

