

#include "Lab.h"
#include "LevelRuntime.h"
#include "Inf.h"
#include "Atx.h"

#include <iostream>
#include <boost/exception/all.hpp>

std::string labFilePath;

lab_fuse::Lab lab;

int main(int argc, char *argv[])
{
    try {
        //lab = lab_fuse::loadLabFile(labFilePath);

        auto instructions = outlaws::loadAtx(R"(F:\VSProjects\LabFuse\OutlawsX\OutlawsLib\antlr\AtxInput.txt)");
        outlaws::AtxItem item(0, outlaws::AtxWallKind::MID, 0, instructions);
        bool trigger = false;
        outlaws::LvtLevel dummyLevel;
        while (true) {
            item.update(dummyLevel, 0.1f);
            if (trigger)
                item.trigger(dummyLevel, 0);
        }

        outlaws::loadInf(R"(F:\GOG Games\Outlaws\hideoutz.INF)");

        auto level = outlaws::loadLvt(R"(F:\VSProjects\LabFuse\trash\HIDEOUT.LVT)");
        auto texInfos = outlaws::loadTexInfos(R"(F:\VSProjects\LabFuse\UnityProj0\UnityProj0\Assets\Textures\pack.json)");

        std::vector<outlaws::Vector3> verts;
        std::vector<outlaws::Uv> uvs;
        std::vector<int> tris;

        outlaws::computeLevel(level, texInfos, verts, uvs, tris);
        //outlaws::computeSector(level, texInfos, 402, verts, uvs, tris);
    }
    catch (...) {
        std::cerr << boost::current_exception_diagnostic_information() << std::endl;
    }

    return 0;
}
