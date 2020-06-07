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
//  DataSet
//  Main
//
// FUNCTIONS:
//  None.
//
// DATA FILES:
//    plain text files as indicated in command line

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class InputParser {
    public:
        InputParser(int &argc, char **argv) {
            for (int i=1; i < argc; ++i)
                tokens.push_back(std::string(argv[i]));
        }

        const std::string& getCmdOption(const std::string &option) const {
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(tokens.begin(), tokens.end(), option);
            if (itr != tokens.end() && ++itr != tokens.end()) {
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }

        bool cmdOptionExists(const std::string &option) const {
            return std::find(tokens.begin(), tokens.end(), option)
                != tokens.end();
        }

    private:
        std::vector<std::string>tokens;
};

class DataSet {
    public:
        DataSet(std::string fname, std::string out_id,
                char sub_char, int line_len) {
            ifn = fname;
            ofn = fname;
            oid = out_id + ".";
            sc = sub_char;
            ll = line_len;

            read_file();
            make_ofn();
            if (raw.size() > 0) {
                parse_raw();
                get_results();
            } else {
                std::cerr << "No data extracted from " << ifn << std::endl;
            }
        }

        void write_to_file() {
            if (data.size() > 0) {
                std::ofstream ofs;
                ofs.open(ofn);
                if (ofs.is_open()) {
                    for (const auto &dsi: data) {
                        ofs << dsi.name << std::endl;
                        int l = ll == 0 ? dsi.seq.size() : ll;
                        for (std::size_t b = 0, len = dsi.seq.size(); b < len; b += l) {
                            ofs << dsi.seq.substr(b, l) << std::endl;
                        }
                    }
                    ofs.close();
                } else {
                    std::cerr << ofn << " was not opened." << std::endl;
                    return_code = 1;
                }
            }
            std::cout << data.size() << " data set items written to " << ofn
                << std::endl;
        }

    private:
        struct DataSetItem {
            std::string name;
            std::string seq;
        };

        int ll;                             // line length
        int return_code;                    // Well, what else would it be?
        char sc;                            // substitution char
        std::string oid;                    // makes output file name different
        std::string ifn;                    // input file name
        std::string ofn;                    // output file name
        std::vector<DataSetItem>data;     // where the action is
        std::vector<std::string>raw;       // raw data from input file

        void read_file() {
            if (!ifn.empty()) {
                std::ifstream file(ifn);
                std::string str;
                while (std::getline(file, str)) {
                    raw.emplace_back(str);
                }
            }
        }

        void get_results() {
            DataSetItem dsi;
            dsi = data[0];
            std::string seq_0 = dsi.seq;
            for (int n = 1; n < data.size(); n++) {
                dsi = data[n];
                for (int i = 0; i < dsi.seq.size(); i++) {
                    if (!isalpha(seq_0[i])) {
                        dsi.seq[i] = sc;
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
            size_t pos = ofn.rfind('.');
            pos = (pos == std::string::npos) ? ofn.size(): pos + 1;
            ofn.insert(pos, oid);
        }

        void parse_raw() {
            DataSetItem dsi;
            dsi.name = "";
            dsi.seq = "";
            std::string line = "";
            for (auto e = raw.begin(); e != raw.end(); e++) {
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

        void augment_data(DataSetItem &dsi) {
            cleanup_seq(dsi.seq);
            if (!dsi.name.empty() && !dsi.seq.empty()) {
                data.emplace_back(dsi);
            }
        }
};

class Main {
    public:
        Main(int &argc, char **argv) {
            return_code = 0;
            argument_count = argc;
            arguments = argv;
            sc = '+';
            oid = "OUT";
            ll = 60;

            manage_cmdline();

            int i = 1;
            while (i < argument_count) {
                if (arguments[i][0] == '-') {
                    i++;
                } else {
                    std::string fn = argv[i];
                    DataSet tds(fn, oid, sc, ll);
                    d_sets.emplace_back(tds);
                }
                i++;
            }

            for (auto d_ptr = d_sets.begin(); d_ptr != d_sets.end(); d_ptr++) {
                DataSet tds = *d_ptr;
                tds.write_to_file();
            }
        }


        int get_return_code() {
            return return_code;
        }


    private:
        char sc;
        int return_code;
        int argument_count;
        int ll;
        std::string line_length;
        std::string oid;
        std::vector<DataSet>d_sets;
        char **arguments;

        bool isNumber(const std::string& s) {
            return !s.empty() && std::find_if(s.begin(), s.end(), [](char c)
                    {return !std::isdigit(c);}) == s.end();
        }

        void manage_cmdline() {
            InputParser input(argument_count, arguments);

            if (input.cmdOptionExists("-h")) {
                std::cout << "Usage: translate -s + -l 60 -o OUT "
                    << "<at least one file name>\n"
                    << "\ts:\tSubstitution char, default = '+'\n"
                    << "\t\tOnly first character will be used.\n"
                    << "\tl:\tMax output length, default = 60\n"
                    << "\to:\tOutput identifier, default = \"OUT\"\n"
                    << "\t\tOne or more file names.\n"
                    << "\t\tThese files must be simple text files.\n";
                exit(0);
            }

            if (input.cmdOptionExists("-l")) {
                std::string line_length = input.getCmdOption("-l");
                if (isNumber(line_length)) {
                    ll = std::stoi(line_length);
                }
            }

            if (input.cmdOptionExists("-o")) {
                std::string oout_id = input.getCmdOption("-o");
                if (oout_id.size() > 0) {
                    oid = oout_id;
                }
            }

            if (input.cmdOptionExists("-s")) {
                std::string substitutionChar = input.getCmdOption("-s");
                if (substitutionChar.size() > 0) {
                    sc = substitutionChar[0];
                }
            }
        }
};

int main(int argc, char **argv) {
    Main just_do_it(argc, argv);
    return just_do_it.get_return_code();
}
