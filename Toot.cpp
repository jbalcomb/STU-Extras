
#include <iostream>

#include "Toot.hpp"
#include "TootLib/TootLib.hpp"
#include "ProjectToot.h"

int main(int argc, char * argv[])
{
    if (argc < 2) {
        // report version
        std::cout << argv[0] << " Version " << ProjectToot_VERSION_MAJOR << "."
                    << ProjectToot_VERSION_MINOR << std::endl;
        std::cout << "Usage: " << argv[0] << " number" << std::endl;
        return 1;
    }

    std::cout << NUMBER << std::endl;

    Hello_World();

    return 0;

}
