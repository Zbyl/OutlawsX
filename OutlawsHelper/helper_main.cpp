

#include "Lab.h"
#include "Lvt.h"

#include <iostream>
#include <boost/exception/all.hpp>

std::string labFilePath;

lab_fuse::Lab lab;

int main(int argc, char *argv[])
{
    try {
        //lab = lab_fuse::loadLabFile(labFilePath);

        lab_fuse::loadLvt(R"(F:\VSProjects\LabFuse\trash\CANYON.LVT)");
    }
    catch (...) {
        std::cerr << boost::current_exception_diagnostic_information() << std::endl;
    }

    return 0;
}
