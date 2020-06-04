#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

namespace fs = std::filesystem;

typedef std::array<std::string, 2> dataSet;
typedef std::vector<dataSet> bunch;

class
InputParser {
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }

        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }

        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
        }

    private:
        std::vector <std::string> tokens;
};

std::string
getName(std::string l) {
    std::size_t start = l.find('[') + 1;
    std::size_t len = l.find(']') - start;
    return ">" + l.substr(start, len);
}

std::string
cleanupSeq(std::string l) {
    std::string s = "";
    for(auto i = l.begin(); i != l.end(); i++) {
        if(*i != '\n') s += *i;
    }
    return s;
}

void
writeResults(bunch &data, const int ll, std::string ofn) {
    long l;
    std::string ts;
    std::ofstream out;
    out.open(ofn);
    if(out.is_open()) {
        dataSet tempArray;
        for(auto i = data.begin(); i != data.end(); i++) {
            tempArray = *i;
            out << tempArray[0] << std::endl;
            ts = tempArray[1];
            for(long b = 0; b < ts.size(); b += ll) {
                ts = tempArray[1];
                l = b + ll <= ts.size() ? ll : ts.size() - b;
                out << ts.substr(b, l) << std::endl;
            }
        }
    } else {
        std::cerr << ofn << " was not opened." << std::endl;
    }
    out.close();
}

std::string
makeOFN(std::string fn, std::string id) {
    size_t pos = fn.rfind('.');
    if(pos == std::string::npos) pos = id.size();
    return fn.insert(pos + 1, id);
}

void
makeResults(bunch &data, char &sc) {
    for(long n = 1; n < data.size(); n++) {
        for(long i = 0; i < data[n][1].size(); i++) {
            if(!isalpha(data[0][1][i])) {
                data[n][1][i] = sc;
            }
        }
    }
}

bunch
parseRawData(std::vector<std::string> rawData) {
    bunch pd;
    dataSet tempArray;
    tempArray[0] = "";
    tempArray[1] = "";
    std::string line = "";
    for(auto e = rawData.begin(); e != rawData.end(); e++) {
        line = *e;
        if(line[0] == '>') {
            if(tempArray[0][0] == '>') pd.emplace_back(tempArray);
            tempArray[0] = getName(line);
            tempArray[1] = "";
        } else {
            tempArray[1] += cleanupSeq(line);
        }
    }
    if(tempArray[0][0] == '>') pd.emplace_back(tempArray);
    return pd;
}

void
doFile(const std::string fn ,const int ll, std::string id,
        char sc) {
    std::vector <std::string> contents;
    std::string line = "";
    std::ifstream fin;
    fin.open(fn);
    while (fin) {
        getline(fin, line);
        contents.emplace_back(line);
    }
    fin.close();
    id += ".";

    bunch parsedRawData = parseRawData(contents);
    makeResults(parsedRawData, sc);
    writeResults(parsedRawData, ll, makeOFN(fn, id));
}

int
main(int argc, char **argv){
    InputParser input(argc, argv);
    if(input.cmdOptionExists("-h")){
        std::cout << "Usage: translate -s+ -l60 -oOUT <file name(s)>\n"
            << "\ts:\tSubstitution char, default = '+'\n"
            << "\t\tOnly first character will be used.\n"
            << "\tl:\tMax output length, default = 60\n"
            << "\to:\tOutput identifier, default = \"OUT\"\n"
            << "\t\tOne or more file names.\n"
            << "\t\tThese files must be simple text files.\n";
    }

    int lineLen = 60;
    const std::string &lineLength = input.getCmdOption("-l");
    if(lineLength.size() > 0) {
        lineLen = std::stoi(lineLength);
    }

    std::string outID = "OUT";
    const std::string &ooutID = input.getCmdOption("-o");
    if(ooutID.size() > 0) {
        outID = ooutID;
    }

    char sc = '+';
    const std::string &substitutionChar = input.getCmdOption("-s");
    if(substitutionChar.size() > 0) {
        sc = substitutionChar[0];
    }

    std::vector <std::string> fns;
    int i = 1;
    while(i < argc) {
        if(argv[i][0] == '-') {
            i++;
        } else {
            fns.emplace_back(argv[i]);
        }
        i++;
    }

    for (auto i =fns.begin(); i!=fns.end(); ++i) {
        doFile(*i, lineLen, outID, sc);
    }

    return 0;
}
