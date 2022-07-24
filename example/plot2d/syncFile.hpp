#include <filesystem>
#include <string>
using std::string;
class syncFile_t {
   public:
    syncFile_t(const string& fname) : fname(fname) {
        lastReportedModifTime = std::filesystem::last_write_time(fname);
    }

    bool isModified() {
        std::filesystem::file_time_type m = std::filesystem::last_write_time(fname);
        bool r = (m != lastReportedModifTime);
        lastReportedModifTime = m;
        return r;
    }

   protected:
    string fname;
    std::filesystem::file_time_type lastReportedModifTime;
};