

#include "Lab.h"
#include "RuntimeLevel.h"
#include "Inf.h"
#include "Atx.h"

#include <iostream>
#include <boost/exception/all.hpp>

std::string labFilePath;

lab_fuse::Lab lab;

int main(int argc, char *argv[])
{
    using namespace outlaws;
    try {
        //lab = lab_fuse::loadLabFile(labFilePath);

#if 0
        auto instructions = loadAtx(R"(F:\VSProjects\OutlawsXDir\OutlawsX\OutlawsLib\antlr\AtxInput.txt)");
        AtxItem item(0, AtxWallKind::MID, 0, instructions);
        bool trigger = false;
        LvtLevel dummyLevel;
        while (true) {
            item.update(dummyLevel, 0.1f);
            if (trigger)
                item.trigger(dummyLevel, 0);
        }

        loadInf(R"(F:\GOG Games\Outlaws\hideoutz.INF)");
#endif

        auto lvt = loadLvt(R"(F:\VSProjects\OutlawsXDir\trash\HIDEOUT.LVT)");
        auto texInfos = loadTexInfos(R"(F:\VSProjects\OutlawsXDir\UnityProj0\UnityProj0\Assets\Textures\pack.json)");

        RuntimeLevel level(std::move(lvt), texInfos);
        level.computeMeshes();
    }
    catch (...) {
        std::cerr << boost::current_exception_diagnostic_information() << std::endl;
        return 1;
    }

    return 0;
}
