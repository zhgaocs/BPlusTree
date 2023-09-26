#include <fstream>
#include <iostream>
#include <string>
#include "bptree.h"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        BPlusTree<int, 3> bpt;

        std::ifstream in(argv[1]);

        if (!in.is_open())
        {
            std::cerr << "unable to open" << argv[1] << '\n';
            std::exit(EXIT_FAILURE);
        }

        std::string line;

        while (std::getline(in, line))
            bpt.insert(std::stoi(line));

        in.close();

        std::cout << bpt << std::endl;

        std::cin.get();
    }
    else
    {
        std::cerr << "arguments of main too little\n";
        std::exit(EXIT_FAILURE);
    }
}
