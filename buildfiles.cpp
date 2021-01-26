#include "buildfiles.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <functional>

namespace fs = std::filesystem;


class FsData {
public:
    FsData(std::string path) {
        os.open(path, std::ios::binary);
        if (!os.is_open()) {
            std::cerr << "Error: Couldn't open " << path << std::endl;
        }
    }

    ~FsData() {
        if (os.is_open())
            os.close();
    }

    void write(std::string msg) {
        os << msg;
    }
private:
    std::ofstream os;

    FsData(FsData const &) = delete;
    FsData & operator=(FsData const &) = delete;
};


static void addList(std::unique_ptr<FsData> &fs, std::vector<std::string> &fileList) {
    std::string prev_file;
    std::string prev_point;


    for (auto name = fileList.cbegin(); name != fileList.cend() - 1; ++name) {
        if (prev_point.empty()) prev_point = "NULL";
        else prev_point = "&file" + prev_file;

        prev_file = *name;

        fs->write("static const struct fsdata_file file_" + *name + " = {");
        fs->write(prev_point + ", data_" + *name + ", ");
        fs->write("data_" + *name + " + " + std::to_string(name->size() + 2) + ", ");
        fs->write("sizeof(data_" + *name + ") - " + std::to_string(name->size() + 2) + "};\n\n");
    }

    prev_point = "&file" + prev_file;
    std::string lastName = fileList.back();
    fs->write("const struct fsdata_file file_" + lastName + " = {");
    fs->write(prev_point + ", data_" + lastName + ", ");
    fs->write("data_" + lastName + " + " + std::to_string(lastName.size() + 2) + ", ");
    fs->write("sizeof(data_" + lastName + ") - " + std::to_string(lastName.size() + 2) + "};\n\n");
    fs->write("#define FS_NUMFILES " + std::to_string(fileList.size()) + "\n");
}


static std::string addFile(std::unique_ptr<FsData> &fs, fs::path path) {
    std::string raw_name = "";
    if (fs::is_symlink(path)) return raw_name;

    if (fs::is_regular_file(path)) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Couldn't open " << path << std::endl;
        }

        std::string file_name = "/" + path.string();

        std::transform(file_name.begin(), file_name.end(), file_name.begin(), [](char ch) {
           return ch == '\\' ? '/' : ch;
        });

        raw_name = file_name.substr(1);

        std::transform(raw_name.begin(), raw_name.end(), raw_name.begin(), [](char ch) {
            if (ch == '/' || ch == '.') return '_';
            return ch;
        });

        fs->write("static const unsigned char data_" + raw_name + "[] = {\n");
        fs->write("\t/* " + file_name + " */\n\t");

        auto writeByCh = [&fs](std::string str) {
            for (auto ch : str) {
                char buf[10];
                std::sprintf(buf, "0x%X", (static_cast<uint8_t>(ch) & 0xff));
                fs->write(buf);
                fs->write(", ");
            }
        };

        writeByCh(file_name);
        fs->write("0,\n\t");

        std::ifstream inf("." + file_name, std::ios::binary);
        if (!inf.is_open()) {
            std::cerr << "Error: Couldn't open " << path << std::endl;
            return "";
        }

        char ch;
        uint8_t count = 0;
        while(inf.get(ch)) {
            char buf[10];
            std::sprintf(buf, "0x%X, ", (static_cast<uint8_t>(ch) & 0xff));
            fs->write(buf);
            if (++count == 10) {
                count = 0;
                fs->write("\n\t");
            }
        }

        inf.close();

        fs->write("0};\n\n");
    }
    return raw_name;
}


static std::unique_ptr<FsData> prepareFsdatac(fs::path path) {
    std::unique_ptr<FsData> fsdata {new FsData(path.string() + "/fsdata.c")};

    fsdata->write("#include \"fsdata.h\"\n\n");
    fsdata->write("#include <stddef.h>\n\n");

    return fsdata;
}


static void makeFsdatah(fs::path path, std::string &name) {
    auto tmp = new FsData(path.string() + "/fsdata.h");
    std::unique_ptr<FsData> fsdata_h{tmp};

    fsdata_h->write("#ifndef __FSDATA_H__\n");
    fsdata_h->write("#define __FSDATA_H__\n");
    fsdata_h->write("\n");
    fsdata_h->write("#include \"fs.h\"\n");
    fsdata_h->write("\n");
    fsdata_h->write("extern const struct fsdata_file file_" + name + ";\n");
    fsdata_h->write("\n");
    fsdata_h->write("#define FS_ROOT file_" + name + "\n");
    fsdata_h->write("\n");
    fsdata_h->write("struct fsdata_file {\n");
    fsdata_h->write("    const struct fsdata_file *next;\n");
    fsdata_h->write("    const unsigned char *name;\n");
    fsdata_h->write("    const unsigned char *data;\n");
    fsdata_h->write("    const int len;\n");
    fsdata_h->write("};\n");
    fsdata_h->write("\n");
    fsdata_h->write("struct fsdata_file_noconst {\n");
    fsdata_h->write("    struct fsdata_file *next;\n");
    fsdata_h->write("    char *name;\n");
    fsdata_h->write("    char *data;\n");
    fsdata_h->write("    int len;\n");
    fsdata_h->write("};\n");
    fsdata_h->write("\n");
    fsdata_h->write("#endif /* __FSDATA_H__ */\n");
    fsdata_h->write("\n");
}


void buildFiles(std::string where, std::vector<std::string> files) {
    if (!fs::exists(where)) {
        std::cerr << "Directory not exists: " << where << std::endl;
        return;
    }
    auto dataPath = fs::canonical(where);

    if (!fs::is_directory(dataPath)) {
        std::cerr << "It's not directory: " << dataPath << std::endl;
        return;
    }

    auto fsdatac = prepareFsdatac(dataPath);

    std::vector<std::string> fileList;
    for (std::string const &file : files) {
        if (!fs::exists(file)) {
            std::cerr << "File not exists: " << file << std::endl;
            continue;
        }

        auto fsP = fs::canonical(file);

        auto current_direcoty = fs::canonical(fs::current_path());

        if (fs::is_directory(file)) {
            fs::current_path(file);

            for (auto &&p : fs::recursive_directory_iterator(fs::current_path())) {
                std::string rfile = addFile(fsdatac, fs::relative(fs::canonical(p.path())));
                if (rfile != "") {
                    fileList.push_back(rfile);
                }
            }
        } else {
            std::string rfile = addFile(fsdatac, file);
            if (rfile != "") {
                fileList.push_back(rfile);
            }
        }
        fs::current_path(current_direcoty);
    }
    addList(fsdatac, fileList);

    makeFsdatah(dataPath, fileList.back());
}
