#include <iostream>
#include "progopt.hpp"
#include "buildfiles.hpp"


class Progopt {
    Progopt() = default;
    Progopt(Progopt const &) = delete;
    Progopt &operator=(Progopt const &) = delete;
public:
    std::vector<std::string> files;
    std::string where = "";


    static Progopt &instance() {
        static Progopt obj;
        return obj;
    }
};


static const char helpMsg[] = R"HO(textToBin

Usage:

  textToBin -w <path> [-h] <string>

Options:

  -w <path>
    (required) directory where to generate files

  -h, /?, --help
    Display usage information and exit.

  <string>
    (required)  directories.
)HO";

void help() {
    std::cout << helpMsg;
}


int main(int argc, char const **argv) {
    auto &progOpt = Progopt::instance();

    ParsedOptions options = {
        {{"-w", "--where"},      true,  [&](std::string &&arg) { progOpt.where = arg; }},
        {{"-h", "/?", "--help"}, false, [&](std::string &&) { throw ProgoptError(); }}
    };

    try {
        parseProgramOptions(argc, argv, options,
                            [&files = progOpt.files](std::string &&arg) { files.emplace_back(move(arg)); });

        if (progOpt.where == "" || progOpt.files.empty()) {
            throw std::invalid_argument("");
        }

        std::cout << progOpt.where << std::endl;
        buildFiles(progOpt.where, progOpt.files);

    } catch (ProgoptError &) {
        help();
        return 1;
    } catch (std::invalid_argument &) {
        std::cout << "Invalid argument" << std::endl;
        help();
        return 1;
    }

    return 0;
}
