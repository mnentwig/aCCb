// =======================================================================
// === code snippet for simple command line parsing (non-hierarchical) ===
// ========================================================================
#include <cassert>
#include <iostream>
#include <regex>
#include <string>
#include <type_traits>
#include <vector>
#include "../aCCb/stringToNum.hpp"
using std::cerr, std::cout, std::endl, std::regex, std::string, std::vector;
class cmdLineArgs {
   public:
    cmdLineArgs(int argc, char** argv) : execName(argc > 0 ? argv[0] : "?? missing executable name ??") {
        vector<string> unparsed;

        // === handle named arguments ===
        for (int ix = 1; ix < argc; ++ix) {
            string arg(argv[ix]);
            if (tryArgNum(arg, "-exampleInt", this->exampleInt)) {
            } else if (tryArgNum(arg, "-exampleUInt", this->exampleUInt)) {
            } else if (tryArgNum(arg, "-exampleFloat", this->exampleFloat)) {
            } else if (tryArgNum(arg, "-exampleDouble", this->exampleDouble)) {
            } else if (tryArgString(arg, "-exampleString1", this->exampleString1)) {
            } else if (tryArgString(arg, "-exampleString2", this->exampleString2)) {
            } else {
                if (arg.find('-') == 0)
                    usage(arg + ": unknown switch");
                unparsed.push_back(arg);
            }
        }

        // === handle anonymous arguments ===
        for (auto a : unparsed) {
            if (this->exampleString1.empty())
                this->exampleString1 = a;
            else if (this->exampleString2.empty())
                this->exampleString2 = a;
            else
                usage(a + ": unexpected standalone argument");
        }

        // === assign default value where empty() flags non-initialized state ===
        if (this->exampleString1.empty())
            this->exampleString1 = this->exampleString1_default;
        if (this->exampleString2.empty())
            this->exampleString2 = this->exampleString2_default;
    }

   protected:
    bool
    tryArgString(const string& line, string argname, string& result) {
        if (!std::regex_match(line, this->m, regex(string(argname) + "=(.+)")))
            return false;
        assert(m.size() == 2);  // complete expression, capture
        result = m[1];
        return true;
    }

    template <typename T>
    bool tryArgNum(const string& line, string argname, T& result) {
        if (!std::regex_match(line, this->m, regex(argname + "=(.+)")))
            return false;
        assert(m.size() == 2);  // complete expression, capture
        if (!aCCb::str2num(this->m[1], /*out*/ result))
            usage(line + ": failed to convert '" + string(this->m[1]) + "'");
        return true;
    }

    string execName;
    std::smatch m;

   public:
    void usage(const string message) {
        cmdLineArgs defaults(0, NULL);  // dummy object to retrieve unmodified default values
        if (!message.empty())
            cerr << message << endl << endl;
        cerr << "usage:" << endl;
        cerr << "-exampleInt=xyz    accepts signed int (default: " << std::to_string(defaults.exampleInt) << endl;
        cerr << "-exampleUInt=xyz   accepts unsigned int (default: " << std::to_string(defaults.exampleUInt) << endl;
        cerr << "-exampleFloat      accepts float" << endl;
        cerr << "-exampleDouble     accepts double" << endl;
        cerr << "-exampleString1    accepts string. If unassigned, picks the first anonymous argument (default: '" << defaults.exampleString1 << "')" << endl;
        cerr << "-exampleString2    accepts string. If unassigned, picks the second anonymous argument (default: '" << defaults.exampleString2 << "')" << endl;
        exit(0);
    }

    // === user defined fields ===
    int exampleInt = -1;
    unsigned int exampleUInt = UINT_MAX;
    float exampleFloat = -1;
    double exampleDouble = -1;
    string exampleString1 = "";  // may take anonymous argument
    string exampleString1_default = "a default value";
    string exampleString2 = "";  // may take anonymous argument
    string exampleString2_default = "another default value";
};

int main(int argc, char** argv) {
    cmdLineArgs args(argc, argv);
    cout << "exampleInt:\t" << args.exampleInt << endl;
    cout << "exampleUInt:\t" << args.exampleUInt << endl;
    cout << "exampleFloat:\t" << args.exampleFloat << endl;
    cout << "exampleDouble:\t" << args.exampleDouble << endl;
    cout << "exampleString1:\t" << args.exampleString1 << endl;
    cout << "exampleString2:\t" << args.exampleString2 << endl;
    return 0;
}