// FILE:        translate.cpp
// STANDARD:    C++17
// AUTHOR:      Phil Huffman
// Copyright 2020 Philip Huffman
//
// PURPOSE:
// This is a c++ program that reads a text file and writes a text file with
// a name derived from the name of the input file (e.g. d.txt -> d.OUT.txt).
// It reads the first data set, determines where the protein sequence is in the
// count. Then, in every subsequent data set, the program will only return data
// that is related to that count. This will replace data that does that does
// not precisely align with sequence positions from the first data set (whether
// they are a dash or a letter). The output file will have the same number of
// data sets as the input file. The proteins sequence of the first data set is
// unchanged. In the remaining data sets, each character of the sequence is
// unchanged if the corresponding character of the first sequence is a letter.
// Otherwise, it is replaced with the specified replacement character.
// If none is specified, the default '+' will be used.
//
// OVERALL METHOD:
// The general tasks are:
// 1. Parse command line building a vector of file names
// 2. Instantiate a DataSet object
// 3. Instantiate a DataSet object
// 4. Repeat from step 2 until all file names have been used.
//
// CLASSES:
//  InputParser
//
//  DataSet
//
// FUNCTIONS:
//  None.
//
// DATA FILES:
//    plain text files as indicated in command line

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class
InputParser {
    public:
        InputParser(int &argc, char **argv) {
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }

        const std::string& getCmdOption(const std::string &option) const {
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }

        bool cmdOptionExists(const std::string &option) const {
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
        }

    private:
        std::vector <std::string> tokens;
};

class
DataSet {
    public:
        DataSet(const std::string fname, const std::string out_id,
                char sub_char, int line_length) {
            this->ifn = fname;
            this->ofn = fname;
            this->oid = out_id + ".";
            this->sc = sub_char;
            this->ll = line_length;

            read_file();
            make_ofn();
            if (this->raw.size() > 0) {
                parse_raw();
                get_results();
            } else {
                std::cerr << "No data extracted from " << ifn << std::endl;
            }
        }

        void write_to_file() {
            if (this->data.size() > 0) {
                int l;              // length of output line
                int b;              // start of output line in sequence
                DataSet_item dsi;   // current dataset item
                std::ofstream out;
                out.open(this->ofn);
                if (out.is_open()) {
                    for (auto i = data.begin(); i != data.end(); i++) {
                        dsi = *i;
                        out << dsi.name << std::endl;
                        for (b = 0; b < dsi.seq.size(); b += this->ll) {
                            l = this->ll;
                            while (b + l > dsi.seq.size()) {
                                l--;
                            }
                            out << dsi.seq.substr(b, l) << std::endl;
                        }
                    }
                } else {
                    std::cerr << ofn << " was not opened." << std::endl;
                }
                out.close();
            }
            std::cout << this->data.size() << " data set items written to "
                << this->ofn << std::endl;
        }

    private:
        struct
            DataSet_item {
                std::string name;
                std::string seq;
            };

        int ll;                             // line length
        char sc;                            // substitution char
        std::string oid;                    // makes output file name different
        std::string ifn;                    // input file name
        std::string ofn;                    // output file name
        std::vector<DataSet_item> data;     // where the action is
        std::vector<std::string> raw;       // raw data from input file

        void read_file() {
            if (!this->ifn.empty()) {
                std::ifstream file(this->ifn);
                std::string str;
                while (std::getline(file, str)) {
                    this->raw.emplace_back(str);
                }
            }
        }

        void get_results() {
            DataSet_item dsi;
            dsi = this->data[0];
            std::string seq_0 = dsi.seq;
            for (int n = 1; n < data.size(); n++) {
                dsi = this->data[n];
                for (int i = 0; i < dsi.seq.size(); i++) {
                    if (!isalpha(seq_0[i])) {
                        dsi.seq[i] = this->sc;
                    }
                }
                data[n] = dsi;
            }
        }

        std::string get_name(std::string &s) {
            int start = s.find('[');
            int end = s.find(']');
            int len = end - start;
            return ">" + s.substr(start + 1, len - 1);
        }

        void cleanup_seq(std::string &s) {
            int i = 0;
            while (i < s.size()) {
                if (s[i] == '\n') {
                    s.erase(i, 1);
                } else {
                    i++;
                }
            }
        }

        void make_ofn() {
            size_t pos = this->ofn.rfind('.');
            pos = (pos == std::string::npos) ? this->ofn.size(): pos + 1;
            this->ofn.insert(pos, this->oid);
        }

        void parse_raw() {
            DataSet_item dsi;
            dsi.name = "";
            dsi.seq = "";
            std::string line = "";
            for (auto e = this->raw.begin(); e != this->raw.end(); e++) {
                line = *e;
                if (line[0] == '>') {
                    augment_data(dsi);
                    dsi.name = get_name(line);
                    dsi.seq = "";
                } else {
                    dsi.seq += line;
                }
            }
            augment_data(dsi);
        }

        void augment_data(DataSet_item &dsi) {
            cleanup_seq(dsi.seq);
            if (!dsi.name.empty() && !dsi.seq.empty()) {
                this->data.emplace_back(dsi);
            }
        }
};

int
main(int argc, char **argv) {
    InputParser input(argc, argv);
    if (input.cmdOptionExists("-h")) {
        std::cout << "Usage: translate -s + -l 60 -o OUT "
            << "<at least one file name>\n"
            << "\ts:\tSubstitution char, default = '+'\n"
            << "\t\tOnly first character will be used.\n"
            << "\tl:\tMax output length, default = 60\n"
            << "\to:\tOutput identifier, default = \"OUT\"\n"
            << "\t\tOne or more file names.\n"
            << "\t\tThese files must be simple text files.\n";
    }

    int lineLen = 60;
    const std::string &lineLength = input.getCmdOption("-l");
    if (lineLength.size() > 0) {
        lineLen = std::stoi(lineLength);
    }

    std::string out_id = "OUT";
    const std::string &oout_id = input.getCmdOption("-o");
    if (oout_id.size() > 0) {
        out_id = oout_id;
    }

    char sc = '+';
    const std::string &substitutionChar = input.getCmdOption("-s");
    if (substitutionChar.size() > 0) {
        sc = substitutionChar[0];
    }

    int i = 1;
    while (i < argc) {
        if (argv[i][0] == '-') {
            i++;
        } else {
            DataSet data(argv[i], out_id, sc, lineLen);
            data.write_to_file();
        }
        i++;
    }

    return 0;
}
