#include <iostream>
#include "../include/linked_hash_map.hpp"

using namespace ppstd;

int main()
{
    linked_hash_map<int, double>::iterator free_it;

    linked_hash_map<int, double> map;
    for (int i = 1; i <= 10; i++)
    {
        map.insert({ i, i * 10.00 });
    }

    map.insert({
        {11, 110},
        {12, 120},
        {13, 130}
    });

    map.emplace(14, 140.14);
    map.emplace_hint(map.cbegin(), 15, 150.15);
    map.emplace(std::pair<int, double>{16, 160.16});

    auto k = map.insert({ 2, 1111 });
    std::cout << k.first->first << " " << k.first->second << "\n";

    for (auto it = map.begin(); it != map.end(); it++)
    {
        //std::cout << it << "\n";
        std::cout << it->first << " " << (*it).second << "\n";
    }

    map.erase(14);
    for (auto it = map.begin(); it != map.end(); it++)
    {
        //std::cout << it << "\n";
        std::cout << it->first << " " << (*it).second << "\n";
    }

    auto seven_it = map.find(7);
    std::cout << seven_it->first << " " << (*seven_it).second << "\n";
    seven_it->second = 7000.0;
    std::cout << seven_it->first << " " << (*seven_it).second << "\n";

    std::pair<const int, double> what = *seven_it;
    what.second = 70000.0;
    std::cout << seven_it->first << " " << seven_it->second << "\n";
    std::cout << what.first << " " << what.second << "\n";

    auto rang = map.equal_range(5);
    std::cout << rang.first->first << " " << rang.first->second << "\n";
    std::cout << rang.second->first << " " << rang.second->second << "\n";

    std::cout << map.at(9) << "\n";
    map.at(9) = 90000.0;
    std::cout << map.at(9) << "\n";
    std::cout << map.find(9)->second << "\n";

    std::cout << map[10] << "\n";
    map[10] = 100000.0;
    std::cout << map[10] << "\n";
    std::cout << map.find(10)->second << "\n";

    map[20];
    std::cout << map[20] << "\n";
    map[20] = 200000.0;
    std::cout << map[20] << "\n";

    linked_hash_map<int, double> map1;
    map1.insert({ 1, 10 });
    linked_hash_map<int, double> map2;
    if (map1 != map2)
    {
        std::cout << "uneq as expected\n";
    }
    map2.insert({ 1, 10 });
    if (map1 == map2)
    {
        std::cout << "eq as expected\n";
    }
    map1.insert({ 2,20 });
    map2.insert({ 2, 200 });
    if (map1 != map2)
    {
        std::cout << "uneq as expected\n";
    }
    swap(map1, map2);
    for (auto map1_iter = map1.begin(), map2_iter = map2.begin(); 
        map1_iter != map1.end() && map2_iter != map2.end();
        ++map1_iter, ++map2_iter)
    {
        std::cout << "map1 " << map1_iter->first << " " << map1_iter->second << "\n";
        std::cout << "map2 " << map2_iter->first << " " << map2_iter->second << "\n";
    }

    linked_hash_map<double, int> ohmap = { {20.1, 2}, {50.1, 5} };
    for (auto it = ohmap.begin(); it != ohmap.end(); ++it)
    {
        std::cout << it->first << " " << (*it).second << "\n";
    }

    linked_hash_map<double, int> ohcopymap(ohmap);
    for (auto it = ohcopymap.begin(); it != ohcopymap.end(); ++it)
    {
        std::cout << it->first << " " << (*it).second << "\n";
    }

    linked_hash_map<double, int> ohbkmap = 20;
    std::cout << ohbkmap.bucket_count() << "\n";

    std::cout << "hello world\n";
    return 0;
}
