#ifndef progopt_hpp_
#define progopt_hpp_

#include <string>
#include <vector>
#include <functional>


struct Option {
    std::vector<std::string> names;
    bool hasArg;
    std::function<void(std::string &&)> found;
};
typedef std::vector<Option> ParsedOptions;

struct ProgoptError {};


void parseProgramOptions(int argc, const char **argv, const ParsedOptions &options, std::function<void(std::string&&)> found);


#endif //progopt_hpp_
