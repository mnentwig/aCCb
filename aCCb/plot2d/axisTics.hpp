#pragma once
#include <cassert>
#include <cmath>  // abs
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
using std::vector, std::unordered_set, std::string;
//* decides where to place the axis tics, formats the text labels */
class axisTics {
   public:
    //* Determines axis grid. Returns vector with minorTic, majorTic spacings
    static vector<double> getTicDelta(double startVal, double endVal) {
        double range = std::abs(endVal - startVal);
        double x = 1;
        while (x < range)
            x *= 10;
        vector<double> r;
        double nTarget = 4;  // minimum number
        while (r.size() < 2) {
            if (nTarget * x < range)
                r.push_back(x);
            if (nTarget * x * 0.5 < range)
                r.push_back(x * 0.5);
            else if (nTarget * x * 0.2 < range)
                r.push_back(x * 0.2);
            x /= 10;
            assert(x != 0);
        }
        return r;
    }

    //* given a spacing, calculate the absolute tic values */
    static vector<double> getTicVals(double startVal, double endVal, double ticDelta) {
        vector<double> r;
        if (ticDelta == 0) {
            r.push_back(0);
            return r;
        }

        assert(startVal < endVal);

        // quantize to grid
        int64_t quantStart = std::floor(startVal / ticDelta);
        int64_t quantEnd = std::ceil(endVal / ticDelta);
        assert((quantEnd - quantStart < 100) && "requesting excessive number of linear axis tics");

        for (int64_t quant = quantStart; quant <= quantEnd; ++quant) {
            double tic = ticDelta * quant;
            // note: don't make the interval wider, otherwise an axis looks like a tic when it's not.
            if ((tic >= startVal - 0.001 * ticDelta) && (tic <= endVal + 0.001 * ticDelta)) {
                // === add gridVal as a tic division ===
                if (!quant)
                    r.push_back(0);  // avoid roundoff error at origin
                else
                    r.push_back(tic);
            }
        }
        return r;
    }

    static vector<string> formatTicVals(const vector<double> ticVals) {
        vector<string> ticValsStr;
        for (int precision = -1; precision < 12; ++precision) {
            ticValsStr = formatTicVals(ticVals, precision);
            unordered_set checkDuplicates(ticValsStr.begin(), ticValsStr.end());
            if (checkDuplicates.size() == ticValsStr.size())
                break;  // labels are unique
        }
        return ticValsStr;
    }

   protected:
    static vector<string> formatTicVals(const vector<double> vals, int precision) {
        std::stringstream ss;
        if (precision < 0) {
            ss << std::fixed;
            ss.precision(0);
        } else
            ss.precision(precision);
        vector<string> r;
        for (double v : vals) {
            ss.str(std::string());  // clear
            ss << v;
            r.push_back(ss.str());
        }
        return r;
    }

    //* test whether val lies within lim1..lim2 range, regardless of order (endpoints inclusive) */
    static inline bool isInRange(double lim1, double lim2, double val) {
        return ((val >= lim1) && (val <= lim2)) || ((val >= lim2) && (val <= lim1));
    }
};
