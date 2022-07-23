#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "../aCCb/csvRawReader.hpp"
using std::vector, std::string_view, std::string;
int main(void) {
    // testcase:
    // m = int64(floor(randn(11, 10)*10000))
    // dlmwrite('sampleCsv.csv', m)
    //    const std::string fn("sampleCsv.csv");
    //   aCCb::csvRawReader r(fn, ',');
    // m = int64(floor(randn(11, 10)*10000))
    // save('test.txt', m)
    // use unix2dos on input data, then compare results with diff
    const std::string fn("test.txt");
    aCCb::csvRawReader r(fn, ' ');
    while (true) {
        vector<string_view> row = r.getRow();
        if (row.size() == 0)
            break;
        for (size_t ix = 0; ix < row.size(); ++ix) {
            if (ix > 0)
                std::cout << " ";
            string v(row[ix].cbegin(), row[ix].cend());
            std::cout << v;
        }
        std::cout << std::endl;
    }
}