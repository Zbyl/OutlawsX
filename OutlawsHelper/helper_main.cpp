

#include "Lab.h"
#include "Lvt.h"
#include "Inf.h"

#include <iostream>
#include <boost/exception/all.hpp>

std::string labFilePath;

lab_fuse::Lab lab;

int main(int argc, char *argv[])
{
    try {
        //lab = lab_fuse::loadLabFile(labFilePath);

        inf::loadAtx(R"(F:\VSProjects\LabFuse\OutlawsX\OutlawsLib\antlr\AtxInput.txt)");
        inf::loadInf(R"(F:\GOG Games\Outlaws\hideoutz.INF)");

        auto level = lvt::loadLvt(R"(F:\VSProjects\LabFuse\trash\HIDEOUT.LVT)");
        auto texInfos = lvt::loadTexInfos(R"(F:\VSProjects\LabFuse\UnityProj0\UnityProj0\Assets\Textures\pack.json)");

        std::vector<lvt::Vector3> verts;
        std::vector<lvt::Uv> uvs;
        std::vector<int> tris;

        lvt::computeLevel(level, texInfos, verts, uvs, tris);
        //lvt::computeSector(level, texInfos, 402, verts, uvs, tris);
    }
    catch (...) {
        std::cerr << boost::current_exception_diagnostic_information() << std::endl;
    }

    return 0;
}
