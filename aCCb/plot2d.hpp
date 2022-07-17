#pragma once
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "vectorText.hpp"
#include "widget.hpp"
namespace aCCb {
using std::vector, std::string, std::array, std::unordered_set;
class plot2d : public Fl_Box {
    int axisMarginLeft;
    int axisMarginBottom;
    void draw() {
        this->Fl_Box::draw();
        float fontsize = 13;
        axisMarginLeft = fontsize;
        axisMarginBottom = fontsize;

        vector<double> xAxisDeltas = axisTics::getTicDelta(x0, x1);
        double xAxisDeltaMajor = xAxisDeltas[0];
        double xAxisDeltaMinor = xAxisDeltas[1];

        vector<double> xAxisTicsMajor = axisTics::getTicVals(x0, x1, xAxisDeltaMajor);
        vector<double> xAxisTicsMinor = axisTics::getTicVals(x0, x1, xAxisDeltaMinor);

        // === draw axes === todo use projection
        fl_line_style(FL_SOLID);
        fl_color(FL_GREEN);
        fl_begin_line();
        fl_vertex(axisMarginLeft, 0);
        fl_vertex(axisMarginLeft, h() - axisMarginBottom);
        fl_vertex(w(), h() - axisMarginBottom);
        fl_end_line();

        // === draw x axis minor tics ===
        for (double ticX : xAxisTicsMinor) {  // draw minor tics before data
            fl_begin_line();
            fl_vertex(projX(ticX), h() - axisMarginBottom);
            fl_vertex(projX(ticX), h() - axisMarginBottom - 3);
            fl_end_line();
        }

        // === draw x axis major tics and numbers ===
        vector<string> xAxisTicsMajorStr = axisTics::formatTicVals(xAxisTicsMajor);
        for (int ix = 0; ix < xAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticX = xAxisTicsMajor[ix];
            fl_begin_line();
            fl_vertex(projX(ticX), h() - axisMarginBottom);
            fl_vertex(projX(ticX), h() - axisMarginBottom - 7);
            fl_end_line();
            string ticStr = xAxisTicsMajorStr[ix];
            vector<array<float, 4>> geom = aCCb::vectorFont::renderText(ticStr.c_str());
            geom = aCCb::vectorFont::centerX(geom);
            aCCbWidget::renderText(geom, fontsize, projX(ticX), h() - axisMarginBottom);
        }

        vector<double> yAxisDeltas = axisTics::getTicDelta(y0, y1);
        double yAxisDeltaMajor = yAxisDeltas[0];
        double yAxisDeltaMinor = yAxisDeltas[1];

        vector<double> yAxisTicsMajor = axisTics::getTicVals(y0, y1, yAxisDeltaMajor);
        vector<double> yAxisTicsMinor = axisTics::getTicVals(y0, y1, yAxisDeltaMinor);

        // === draw y axis minor tics ===
        for (double ticY : yAxisTicsMinor) {  // draw minor tics before data
            fl_begin_line();
            fl_vertex(axisMarginLeft, projY(ticY));
            fl_vertex(axisMarginLeft + 3, projY(ticY));
            fl_end_line();
        }

        // === draw x axis major tics and numbers ===
        vector<string> yAxisTicsMajorStr = axisTics::formatTicVals(yAxisTicsMajor);
        for (int ix = 0; ix < yAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticY = yAxisTicsMajor[ix];
            fl_begin_line();
            fl_vertex(axisMarginLeft, projY(ticY));
            fl_vertex(axisMarginLeft + 7, projY(ticY));
            fl_end_line();
            string ticStr = yAxisTicsMajorStr[ix];
            vector<array<float, 4>> geom = aCCb::vectorFont::renderText(ticStr.c_str());
            aCCb::vectorFont::rotate270(geom);
            geom = aCCb::vectorFont::centerY(geom);
            aCCbWidget::renderText(geom, fontsize, axisMarginLeft, projY(ticY));
        }

        // must reset on exit
        fl_line_style(FL_SOLID);
        fl_color(FL_BLACK);
    }

    double projX(double x) const {
        double xNorm = (x - x0) / (x1 - x0);
        double drawingAreaWidth = w() - axisMarginLeft;
        return axisMarginLeft + xNorm * drawingAreaWidth;
    }

    double projY(double y) const {
        double yNorm = (y - y0) / (y1 - y0);
        double drawingAreaWidth = h() - axisMarginLeft;
        return axisMarginLeft + yNorm * drawingAreaWidth;
    }

   public:
    plot2d(int x, int y, int w, int h, const char *l = 0)
        : Fl_Box(x, y, w, h, l) {}
    ~plot2d() {}

   protected:
    double x0 = -1;
    double x1 = 2;
    double y0 = 1.23;
    double y1 = 1.24;
    class axisTics {
       public:
        // Determines axis grid. Returns vector with minorTic, majorTic, nextHigherTic (as fallback if majorTic turns out too small)
        static vector<double> getTicDelta(double startVal, double endVal) {
            double range = std::abs(endVal - startVal);
            double x = 1;
            while (x < range)
                x *= 10;
            vector<double> r;
            double nTarget = 4;  // minimum number
            while (r.size() < 3) {
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

        static vector<double> getTicVals(double startVal, double endVal, double ticDelta) {
            assert(ticDelta != 0);
            if (startVal < endVal)
                ticDelta = std::abs(ticDelta);
            else
                ticDelta = -std::abs(ticDelta);
            double gridVal = startVal;
            gridVal /= ticDelta;
            gridVal = std::floor(gridVal);
            gridVal *= ticDelta;
            gridVal -= ticDelta;  // take one step back as floor may round in the wrong direction, depending on sign
            vector<double> r;
            while (gridVal <= endVal + ticDelta / 2.0) {
                if (isInRange(startVal, endVal, gridVal)) r.push_back(gridVal);
                gridVal += ticDelta;
            }
            return r;
        }

        static vector<string> formatTicVals(const vector<double> ticVals) {
            vector<string> ticValsStr;
            for (int precision = 0; precision < 12; ++precision) {
                ticValsStr = formatTicVals(ticVals, 6);
                unordered_set checkDuplicates(ticValsStr.begin(), ticValsStr.end());
                if (checkDuplicates.size() == ticValsStr.size())
                    break;  // labels are unique
            }
            return ticValsStr;
        }

       protected:
        static vector<string> formatTicVals(const vector<double> vals, int precision) {
            std::stringstream ss;
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
};
}  // namespace aCCb