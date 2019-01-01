

#include "Lab.h"

#include <iostream>
#include <boost/exception/all.hpp>

#define FUSE_USE_VERSION 30
#include <fuse/fuse.h>

std::string labFilePath;

lab_fuse::Lab lab;

static int lab_flush() {
    try {
        lab_fuse::saveLabFile(labFilePath, lab);
        return 0;
    } catch (...) {
        std::cerr << boost::current_exception_diagnostic_information(true) << std::endl;
        return -EFAULT;
    }
}

static int xmp_getattr(const char *path, struct FUSE_STAT *stbuf)
{
    memset(stbuf, 0, sizeof(struct FUSE_STAT));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } 
    
    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    const auto& labFile = position->second;

    stbuf->st_mode = S_IFREG | 0666;
    stbuf->st_nlink = 1;
    stbuf->st_size = labFile.data.size();
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, FUSE_OFF_T offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    if (filler(buf, ".", NULL, 0) != 0)
        return -ENOMEM;
    if (filler(buf, "..", NULL, 0))
        return -ENOMEM;

    for (const auto& fileName : lab.filesOrder) {
        if (filler(buf, fileName.c_str(), NULL, 0))
            return -ENOMEM;
    }

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    if (fi->flags & O_TRUNC) {
        auto& labFile = position->second;
        labFile.data.clear();
    }

    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, FUSE_OFF_T offset, struct fuse_file_info *fi)
{
    (void) fi;

    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    const auto& labFile = position->second;

    if (offset < 0) {
        return -EINVAL;
    }

    size_t len = labFile.data.size();
    size_t ofs = static_cast<size_t>(offset);
    if (ofs > len) {
        return 0;
    }

    if (ofs + size > len)
        size = len - ofs;

    memcpy(buf, labFile.data.data() + ofs, size);

    return static_cast<int>(size);
}

static int xmp_write(const char *path, const char *buf, size_t size, FUSE_OFF_T offset, struct fuse_file_info *fi)
{
    (void) fi;

    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    auto& labFile = position->second;

    if (offset < 0) {
        return -EINVAL;
    }

    size_t len = labFile.data.size();
    size_t ofs = static_cast<size_t>(offset);

    if (ofs + size > len) {
        try {
            labFile.data.resize(ofs + size);
        } catch (...) {
            std::cerr << boost::current_exception_diagnostic_information(true) << std::endl;
            return -ENOMEM;
        }
    }

    memcpy(labFile.data.data() + ofs, buf, size);

    return static_cast<int>(size);
}

static int xmp_rename(const char *from, const char *to)
{
    std::string oldName(from + 1);
    std::string newName(to + 1);

    if (lab.files.count(newName) > 0) {
        std::cerr << "File with name '" << newName << "' already exists." << std::endl;
        return -EINVAL; // Cannot use the same name as existing file.
    }

    auto position = lab.files.find(oldName);
    if (position == lab.files.end())
        return -ENOENT;

    auto namePosition = std::find(lab.filesOrder.begin(), lab.filesOrder.end(), oldName);
    if (namePosition == lab.filesOrder.end()) {
        std::cerr << "Internal structure inconsistent. File '" << oldName << "' not found in filesOrder." << std::endl;
        return -EFAULT;
    }

    // Rename file.
    lab_fuse::LabFile labFile = std::move(position->second);
    lab.files.erase(position);
    lab.files[newName] = std::move(labFile);
    *namePosition = newName;

    return lab_flush();
}

static int xmp_create(const char* path, mode_t, struct fuse_file_info *)
{
    std::string fileName(path + 1);
    if (lab.files.count(fileName) > 0) {
        std::cerr << "File with name '" << fileName << "' already exists." << std::endl;
        return -EINVAL;
    }

    // Create new file.
    lab.filesOrder.push_back(fileName);
    lab_fuse::LabFile labFile;
    labFile.typeAndExtension = std::string(4, '\0');
    lab.files[fileName] = labFile;

    return 0;
}

static int xmp_unlink(const char *path)
{
    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    auto namePosition = std::find(lab.filesOrder.begin(), lab.filesOrder.end(), fileName);
    if (namePosition == lab.filesOrder.end()) {
        std::cerr << "Internal structure inconsistent. File '" << fileName << "' not found in filesOrder." << std::endl;
        return -EFAULT;
    }

    // Remove the file.
    lab.files.erase(position);
    lab.filesOrder.erase(namePosition);

    return 0;
}

static int xmp_flush(const char *, struct fuse_file_info *) {
    return lab_flush();
}

static int xmp_fsync(const char *, int, struct fuse_file_info *) {
    return lab_flush();
}

static int xmp_truncate(const char * path, FUSE_OFF_T size) {
    std::string fileName(path + 1);

    auto position = lab.files.find(fileName);
    if (position == lab.files.end())
        return -ENOENT;

    auto& labFile = position->second;
    try {
        labFile.data.resize(size);
    } catch (...) {
        std::cerr << boost::current_exception_diagnostic_information(true) << std::endl;
        return -ENOMEM;
    }

    return 0;
}

static struct fuse_operations xmp_oper = { 0 };

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "Usage: lab-fuse <path-to-file.lab> <drive-letter> [fuse options]" << std::endl;
        return 1;
    }

    labFilePath = argv[1];

    try {
        lab = lab_fuse::loadLabFile(labFilePath);
    } catch (...) {
        std::cerr << boost::current_exception_diagnostic_information(true) << std::endl;
        return 1;
    }

    xmp_oper.getattr = &xmp_getattr;
    xmp_oper.readdir = &xmp_readdir;
    xmp_oper.open = &xmp_open;
    xmp_oper.read = &xmp_read;
    xmp_oper.write = &xmp_write;
    xmp_oper.rename = &xmp_rename;
    xmp_oper.create = &xmp_create;
    xmp_oper.unlink = &xmp_unlink;
    xmp_oper.flush = &xmp_flush;
    xmp_oper.fsync = &xmp_fsync;
    xmp_oper.truncate = &xmp_truncate;

    return fuse_main(argc - 1, argv + 1, &xmp_oper, NULL);
}
