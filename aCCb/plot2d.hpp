#pragma once
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <array>
#include <cassert>
#include <cmath>  // isinf
#include <iostream>
#include <limits>  // inf
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "vectorText.hpp"
#include "widget.hpp"
namespace aCCb {
using std::vector, std::string, std::array, std::unordered_set, std::cout, std::endl;
class plot2d : public Fl_Box {
   public:
    plot2d(int x, int y, int w, int h, const char* l = 0)
        : Fl_Box(x, y, w, h, l), data(NULL) {}
    ~plot2d() {}

    void draw() {
        this->Fl_Box::draw();
        axisMarginLeft = fontsize;
        axisMarginBottom = fontsize;

        vector<double> xAxisDeltas = axisTics::getTicDelta(x0, x1);
        double xAxisDeltaMajor = xAxisDeltas[0];
        double xAxisDeltaMinor = xAxisDeltas[1];

        vector<double> xAxisTicsMajor = axisTics::getTicVals(x0, x1, xAxisDeltaMajor);
        vector<double> xAxisTicsMinor = axisTics::getTicVals(x0, x1, xAxisDeltaMinor);

        // bottom left
        double screenX0 = x() + axisMarginLeft;
        double screenY0 = y() + h() - axisMarginBottom;
        // top right
        double screenX1 = x() + w();
        double screenY1 = y();

        fl_line_style(FL_SOLID);
        fl_color(FL_GREEN);

        proj p(x0, y0, x1, y1, screenX0, screenY0, screenX1, screenY1);

        // === draw axes ===
        aCCbWidget::line(
            p.projX(x0), p.projY(y1),   // top left
            p.projX(x0), p.projY(y0),   // bottom left
            p.projX(x1), p.projY(y0));  // bottom right

        // === draw x axis minor tics ===
        for (double ticX : xAxisTicsMinor)  // draw minor tics before data
            aCCbWidget::line(p.projX(ticX), p.projY(y0), p.projX(ticX), p.projY(y0) - minorTicLength);

        // === draw x axis major tics and numbers ===
        vector<string> xAxisTicsMajorStr = axisTics::formatTicVals(xAxisTicsMajor);
        for (int ix = 0; ix < xAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticX = xAxisTicsMajor[ix];
            aCCbWidget::line(p.projX(ticX), p.projY(y0), p.projX(ticX), p.projY(y0) - majorTicLength);
            string ticStr = xAxisTicsMajorStr[ix];
            vector<array<float, 4>> geom = aCCb::vectorFont::renderText(ticStr.c_str());
            geom = aCCb::vectorFont::centerX(geom);
            aCCbWidget::renderText(geom, fontsize, p.projX(ticX), p.projY(y0));
        }

        vector<double> yAxisDeltas = axisTics::getTicDelta(y0, y1);
        double yAxisDeltaMajor = yAxisDeltas[0];
        double yAxisDeltaMinor = yAxisDeltas[1];

        vector<double> yAxisTicsMajor = axisTics::getTicVals(y0, y1, yAxisDeltaMajor);
        vector<double> yAxisTicsMinor = axisTics::getTicVals(y0, y1, yAxisDeltaMinor);

        // === draw y axis minor tics ===
        for (double ticY : yAxisTicsMinor)  // draw minor tics before data
            aCCbWidget::line(p.projX(x0), p.projY(ticY), p.projX(x0) + minorTicLength, p.projY(ticY));

        // === draw x axis major tics and numbers ===
        vector<string> yAxisTicsMajorStr = axisTics::formatTicVals(yAxisTicsMajor);
        for (int ix = 0; ix < yAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticY = yAxisTicsMajor[ix];
            aCCbWidget::line(p.projX(x0), p.projY(ticY), p.projX(x0) + majorTicLength, p.projY(ticY));
            string ticStr = yAxisTicsMajorStr[ix];
            vector<array<float, 4>> geom = aCCb::vectorFont::renderText(ticStr.c_str());
            aCCb::vectorFont::rotate270(geom);
            geom = aCCb::vectorFont::centerY(geom);
            aCCbWidget::renderText(geom, fontsize, p.projX(x0), p.projY(ticY));
        }

        // === plot ===
        if (data != NULL) {
            int width = screenX1 - screenX0;
            int height = screenY0 - screenY1;

            vector<int> pixels(width * height);  // int is slightly faster than char
            vector<uint32_t> rgba(pixels.size());
            const proj pPixmap(x0, y0, x1, y1, /*screenX0*/ 0, /*screenY0*/ height, /*screenX1*/ width, /*screenY1*/ 0);

            size_t ixMax = data->size();
            for (size_t ix = 0; ix < ixMax; ++ix) {
                float plotX = ix + 1;
                float plotY = (*data)[ix];
                int pixX = pPixmap.projX(plotX);
                int pixY = pPixmap.projY(plotY);
                if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                    pixels[pixY * width + pixX] = 1;
            }

            // === convert to RGBA image ===
            ixMax = pixels.size();
            for (int ix = 0; ix < ixMax; ++ix)
                if (pixels[ix])
                    // AABBGGRR
                    rgba[ix] = 0xFF00FF00;

            // === render the RGBA image repeatedly, according to the stencil ===
            fl_push_clip(screenX0, screenY1, width, height);
            Fl_RGB_Image im((const uchar*)&rgba[0], width, height, 4);
            const char* tmp = marker;
            int delta = markerSize1d / 2;
            const char* m = this->marker;
            for (int dy = -delta; dy <= delta; ++dy)
                for (int dx = -delta; dx <= delta; ++dx)
                    if (*(m++) != ' ')  // any non-space character in the stencil creates a shifted replica
                        im.draw(screenX0 + dx, screenY1 + dy);
            fl_pop_clip();
        }
    }

    void setData(const std::vector<float>& data, const char* marker) {
        this->data = &data;
        this->marker = marker;
        int charCount = string(marker).size();
        this->markerSize1d = std::sqrt(charCount);
        assert(this->markerSize1d * this->markerSize1d == charCount);  // marker must be square
        x0 = 1;
        x1 = data.size();
        const float inf = std::numeric_limits<float>::infinity();
        float yMin = inf;
        float yMax = -inf;
        for (float v : data) {
            if (~std::isinf(v) && ~std::isnan(v)) {
                yMin = std::min(yMin, v);
                yMax = std::max(yMax, v);
            }
        }
        y0 = yMin;
        y1 = yMax;
        autorange_y0 = yMin;
        autorange_y1 = yMax;
    }

   protected:
    class proj {
        float dataX0, dataY0, dataX1, dataY1;
        int screenX0, screenY0, screenX1, screenY1;
        float mXData2screen;
        float bXData2screen;
        float mYData2screen;
        float bYData2screen;
        // transformation:
        // screen = (data-data1)/(data2-data1)*(screen2-screen1)+screen1;
        // screen = data * (screen2-screen1)/(data2-data1) + screen1 - data1 * (screen2-screen1)/(data2-data1)
       public:
        proj(float dataX0, float dataY0, float dataX1, float dataY1, int screenX0, int screenY0, int screenX1, int screenY1) : dataX0(dataX0), dataY0(dataY0), dataX1(dataX1), dataY1(dataY1), screenX0(screenX0), screenY0(screenY0), screenX1(screenX1), screenY1(screenY1), mXData2screen((screenX1 - screenX0) / (dataX1 - dataX0)), bXData2screen(screenX0 - dataX0 * (screenX1 - screenX0) / (dataX1 - dataX0)), mYData2screen((screenY1 - screenY0) / (dataY1 - dataY0)), bYData2screen(screenY0 - dataY0 * (screenY1 - screenY0) / (dataY1 - dataY0)) {}
        inline int projX(float x) const {
            return x * mXData2screen + bXData2screen + 0.5f;
        }
        inline int projY(float y) const {
            return y * mYData2screen + bYData2screen + 0.5f;
        }
    };

    const char* marker = "X";
    int markerSize1d = 1;
    float fontsize = 13;
    int axisMarginLeft;
    int axisMarginBottom;
    const int minorTicLength = 3;
    const int majorTicLength = 7;

    double x0 = -1;
    double x1 = 2;
    double y0 = 1.23;
    double y1 = 1.24;
    double autorange_y0 = 0;
    double autorange_y1 = 1;
    const std::vector<float>* data;
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
            gridVal -= 2 * ticDelta;  // take one step back as floor may round in the wrong direction, depending on sign
            vector<double> r;
            while (gridVal <= endVal + ticDelta / 2.0) {
                if (isInRange(startVal - 0.01 * ticDelta, endVal + 0.01 * ticDelta, gridVal))
                    r.push_back(gridVal);
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