#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <functional>
 
 
#include <filesystem>
#include <iostream>

class test{
public:
    //test(){};
    std::vector<int> get_v(){
        return v;
    }
private:
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

int main()
{
    test t;
    std::cout<<t.get_v()[1]<<std::endl;
    t.get_v()[1]=6;
    std::cout<<t.get_v()[1]<<std::endl;
    for (auto &t:t.get_v()){
        if (t==3){
            t=12;
        }
    }
    std::cout<<t.get_v()[2]<<std::endl;
    return 0;
}
