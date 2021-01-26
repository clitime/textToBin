#include "progopt.hpp"
#include <iostream>


using std::string;


void parseProgramOptions(int argc, const char **argv, const ParsedOptions &options,
                         std::function<void(std::string &&)> found) {
    auto findOpt = [&options](const string &arg) {
        return find_if(begin(options), end(options), [arg](const Option &o) {
            return find(o.names.begin(), o.names.end(), arg) != o.names.end();
        });
    };

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        auto it = findOpt(arg);
        if (it == end(options)) {
            found(move(arg));
        } else if (it->hasArg) {
            ++i;
            if (i == argc) {
                std::cout << "No argument specified for " << arg << std::endl;
                throw ProgoptError();
            }

            string val = argv[i];
            auto itNext = findOpt(val);
            if (itNext != end(options)) {
                std::cout << "No argument specified for " << arg << std::endl;
                throw ProgoptError();
            }
            it->found(move(val));
        } else {
            it->found(string());
        }
    }
}
