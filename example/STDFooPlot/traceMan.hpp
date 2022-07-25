#pragma once
#include <filesystem>
#include <string>
#include "../aCCb/binIo.hpp"
using std::string;
class traceDataMan_cl {
   public:
    traceDataMan_cl() {}
    void loadData(const string &filename) {
        if (filename == "")
            return;
        // todo: make filename canonical
        if (dataByFilename.find(filename) != dataByFilename.end())
            return;
        string ext = std::filesystem::path(filename).extension().string();
        if (aCCb::caseInsensitiveStringCompare(".float", ext))
            dataByFilename[filename] = aCCb::binaryIo::file2vec<float>(filename);
        else if (aCCb::caseInsensitiveStringCompare(".txt", ext))
            dataByFilename[filename] = aCCb::binaryIo::file2vec_asc<float>(filename);
        else
            throw aCCb::argObjException("unsupported data file extension (" + filename + ")");
    }

    void loadAnnotations(const string &filename) {
        if (filename == "")
            return;

        vector<string> r;
        std::ifstream is(filename);
        if (!is.is_open()) throw runtime_error("failed to open file (r): '" + filename + "')");
        string line;
        while (std::getline(is, line))
            r.push_back(line);
        annotationsByFilename[filename] = r;
    }

    const vector<string> *getAnnotations(const string &filename) const {
        if (filename == "")
            return NULL;
        auto it = annotationsByFilename.find(filename);
        if (it == annotationsByFilename.end())
            throw runtime_error("datafile should have been loaded but does not exist");
        return &(it->second);
    }

    const vector<float> *getData(const string &filename) {
        if (filename == "")
            return NULL;
        auto it = dataByFilename.find(filename);
        if (it == dataByFilename.end())
            throw runtime_error("datafile should have been loaded but does not exist");
        return &(it->second);
    }

   protected:
    map<string, vector<float>> dataByFilename;
    map<string, vector<string>> annotationsByFilename;
};