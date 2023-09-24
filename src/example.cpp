#include <iostream>
#include "bptree.h"

int main()
{
    BPlusTree<int, 3> bpt;

    bpt.insert(66);
    bpt.insert(36);
    bpt.insert(17);
    bpt.insert(3);
    bpt.insert(20);
    bpt.insert(12);

    std::cout << bpt << std::endl;

    bpt.remove(17);

    bpt.find(36) ? std::cout << "find " << 36 << std::endl : std::cout << "no " << 36 << std::endl;

    bpt.clear();

    std::cin.get();
}
