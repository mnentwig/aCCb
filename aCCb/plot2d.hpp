#pragma once
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <array>
#include <cassert>
#include <cmath>  // isinf
#include <future>
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
    class evtMan_cl {
       public:
        evtMan_cl(plot2d* parent) : parent(parent){};
        double mouseDown1DataX;
        double mouseDown1DataY;
        double mouseDown3DataX;
        double mouseDown3DataY;
        plot2d* parent;
        double drawRectx0;
        double drawRecty0;
        double drawRectx1;
        double drawRecty1;
        class mouseState_cl {
           public:
            mouseState_cl() : state(0) {}
            void update(int event) {
                // === track mouse buttons ===
                int stateUpdate = Fl::event_buttons();
                stateDeltaOn = stateUpdate & ~state;
                stateDeltaOff = state & ~stateUpdate;
                state = stateUpdate;

                switch (event) {
                    case FL_PUSH:
                    case FL_RELEASE:
                    case FL_DRAG:
                    case FL_MOUSEWHEEL:
                    case FL_MOVE:
                        int mx = Fl::event_x();
                        int my = Fl::event_y();
                        if ((mx != mouseX) || (my != mouseY)) {
                            mouseX = mx;
                            mouseY = my;
                            mouseMoved = true;
                        } else
                            mouseMoved = false;
                }  // switch

                if (event == FL_MOUSEWHEEL)
                    mouseWheel = Fl::event_dy();
                else
                    mouseWheel = 0;
            }
            bool getDeltaOn(int mask) {
                return (stateDeltaOn & mask) != 0;
            }
            bool getDeltaOff(int mask) {
                return (stateDeltaOff & mask) != 0;
            }
            bool getState(int mask) {
                return (state & mask) != 0;
            }
            bool getMouseMove() {
                return mouseMoved;
            }
            int getMouseWheel() {
                return mouseWheel;
            }
            int getMouseX() {
                return mouseX;
            }
            int getMouseY() {
                return mouseY;
            }

           protected:
            int state;
            int stateDeltaOn;
            int stateDeltaOff;
            int mouseX;
            int mouseY;
            bool mouseMoved;
            int mouseWheel;
        } mouseState;
        int handle(int event) {
            proj<double> p = parent->projDataToScreen<double>();
            mouseState.update(event);
            int mouseX = mouseState.getMouseX();
            int mouseY = mouseState.getMouseY();
            double dataX = p.unprojX(mouseX);
            double dataY = p.unprojY(mouseY);
            if (mouseState.getDeltaOn(FL_BUTTON1)) {
                mouseDown1DataX = dataX;
                mouseDown1DataY = dataY;
            }
            if (mouseState.getDeltaOn(FL_BUTTON3)) {
                mouseDown3DataX = dataX;
                mouseDown3DataY = dataY;
                drawRectx0 = dataX;
                drawRecty0 = dataY;
                drawRectx1 = dataX;
                drawRecty1 = dataY;
            }
            if (mouseState.getDeltaOff(FL_BUTTON3))
                if ((std::fabs(mouseDown3DataX - dataX) > 1e-18) || (std::fabs(mouseDown3DataY - dataY) > 1e-18))
                    parent->setViewArea(/*x0*/ std::min(mouseDown3DataX, dataX), /*y0*/ std::min(mouseDown3DataY, dataY), /*x1*/ std::max(mouseDown3DataX, dataX), /*y1*/ std::max(mouseDown3DataY, dataY));

            if (mouseState.getMouseMove()) {
                if (mouseState.getState(FL_BUTTON1)) {
                    double dx = dataX - mouseDown1DataX;
                    double dy = dataY - mouseDown1DataY;
                    double x0, y0, x1, y1;
                    parent->getViewArea(x0, y0, x1, y1);
                    x0 -= dx;
                    x1 -= dx;
                    y0 -= dy;
                    y1 -= dy;
                    dataX -= dx;
                    dataY -= dy;
                    mouseDown1DataX = dataX;
                    mouseDown1DataY = dataY;
                    parent->setViewArea(x0, y0, x1, y1);
                }
                if (mouseState.getState(FL_BUTTON3)) {
                    drawRectx1 = dataX;
                    drawRecty1 = dataY;
                    parent->cursorRedraw();
                }
            }

            int d = mouseState.getMouseWheel();
            if (d != 0) {
                double scale = 1.2;
                if (d < 0)
                    scale = 1 / scale;
                double x0, y0, x1, y1;
                parent->getViewArea(x0, y0, x1, y1);
                double deltaX0 = x0 - dataX;
                double deltaX1 = x1 - dataX;
                double deltaY0 = y0 - dataY;
                double deltaY1 = y1 - dataY;
                deltaX0 *= scale;
                deltaY0 *= scale;
                deltaX1 *= scale;
                deltaY1 *= scale;
                x0 = deltaX0 + dataX;
                y0 = deltaY0 + dataY;
                x1 = deltaX1 + dataX;
                y1 = deltaY1 + dataY;
                parent->setViewArea(x0, y0, x1, y1);
            }
            return 1;
        }
        bool drawRect(double& x0, double& y0, double& x1, double& y1) {
            if (mouseState.getState(FL_BUTTON3)) {
                x0 = drawRectx0;
                y0 = drawRecty0;
                x1 = drawRectx1;
                y1 = drawRecty1;
                return true;
            } else
                return false;
        }
    } evtMan;

   protected:
    //* fltk event handler */
    int handle(int event) {
        return evtMan.handle(event);
    }

    /** gets the visible plot area */
    void getViewArea(double& x0, double& y0, double& x1, double& y1) const {
        x0 = this->x0;
        y0 = this->y0;
        x1 = this->x1;
        y1 = this->y1;
    }

    //* sets the visible area, triggers redraw */
    void setViewArea(double x0, double y0, double x1, double y1) {
        this->x0 = x0;
        this->y0 = y0;
        this->x1 = x1;
        this->y1 = y1;
        needFullRedraw = true;
        redraw();
    }
    void cursorRedraw() {
        redraw();
    }

    template <typename T>
    class proj;

    template <typename T>
    proj<T> projDataToScreen() {
        // bottom left
        int screenX0 = x() + axisMarginLeft;
        int screenY0 = y() + h() - axisMarginBottom;
        // top right
        int screenX1 = x() + w();
        int screenY1 = y();
        return proj<T>(x0, y0, x1, y1, screenX0, screenY0, screenX1, screenY1);
    }

   public:
    plot2d(int x, int y, int w, int h, const char* l = 0)
        : Fl_Box(x, y, w, h, l), evtMan(this), data(NULL) {}
    ~plot2d() {}

    void drawAxes() {
        proj<double> p = projDataToScreen<double>();
        vector<double> xAxisDeltas = axisTics::getTicDelta(x0, x1);
        double xAxisDeltaMajor = xAxisDeltas[0];
        double xAxisDeltaMinor = xAxisDeltas[1];

        vector<double> xAxisTicsMajor = axisTics::getTicVals(x0, x1, xAxisDeltaMajor);
        vector<double> xAxisTicsMinor = axisTics::getTicVals(x0, x1, xAxisDeltaMinor);

        // === draw axes ===
        fl_push_clip(x(), y(), w(), h());
        aCCbWidget::line(
            p.projX(x0), p.projY(y1),   // top left
            p.projX(x0), p.projY(y0),   // bottom left
            p.projX(x1), p.projY(y0));  // bottom right

        // === draw x axis minor tics ===
        for (double ticX : xAxisTicsMinor)  // draw minor tics before data
            aCCbWidget::line(p.projX(ticX), p.projY(y0), p.projX(ticX), p.projY(y0) - minorTicLength);

        // === draw x axis major tics and numbers ===
        vector<string> xAxisTicsMajorStr = axisTics::formatTicVals(xAxisTicsMajor);
        for (size_t ix = 0; ix < xAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticX = xAxisTicsMajor[ix];

            fl_color(FL_DARK_GREEN);
            aCCbWidget::line(p.projX(ticX), p.projY(y0), p.projX(ticX), p.projY(y1));

            fl_color(FL_GREEN);
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
        for (size_t ix = 0; ix < yAxisTicsMajor.size(); ++ix) {  // todo draw major tics after data
            double ticY = yAxisTicsMajor[ix];

            fl_color(FL_DARK_GREEN);
            aCCbWidget::line(p.projX(x0), p.projY(ticY), p.projX(x1), p.projY(ticY));
            fl_color(FL_GREEN);

            aCCbWidget::line(p.projX(x0), p.projY(ticY), p.projX(x0) + majorTicLength, p.projY(ticY));
            string ticStr = yAxisTicsMajorStr[ix];
            vector<array<float, 4>> geom = aCCb::vectorFont::renderText(ticStr.c_str());
            aCCb::vectorFont::rotate270(geom);
            geom = aCCb::vectorFont::centerY(geom);
            aCCbWidget::renderText(geom, fontsize, p.projX(x0), p.projY(ticY));
        }
        fl_pop_clip();
    }

    //* captures image for fast redraw where permitted e.g. when drawing zoom box */
    class cachedImage_cl {
       public:
        cachedImage_cl() : data(NULL), lastW(0), lastH(0) {}
        void capture(int x, int y, int w, int h) {
            if ((w != lastW) || (h != lastH)) {
                delete[] data;  // NULL is OK
                data = NULL;
            }
            lastW = w;
            lastH = h;
            data = fl_read_image(data, x, y, w, h);
        }
        bool restore(int x, int y, int w, int h) {
            if ((w != lastW) || (h != lastH))
                return false;
            Fl_RGB_Image im((const uchar*)&data[0], w, h, 3);
            im.draw(x, y);
            return true;
        }
        ~cachedImage_cl() {
            delete[] data;
        }
        uchar* data;
        int lastW;
        int lastH;
    } cachedImage;

    void draw() {
        this->Fl_Box::draw();

        axisMarginLeft = fontsize;  // TBD move
        axisMarginBottom = fontsize;
        proj<float> p = projDataToScreen<float>();
        const int screenX = p.getScreenX0();
        const int screenY = p.getScreenY1();
        const int width = p.getScreenWidth();
        const int height = p.getScreenHeight();

        // === try to reload the cached bitmap ===
        if (!this->needFullRedraw)
            if (cachedImage.restore(screenX, screenY, width, height))
                goto skipDataDrawing;

        // === background ===
        fl_rectf(x(), y(), w(), h(), FL_BLACK);

        fl_line_style(FL_SOLID);
        fl_color(FL_GREEN);

        this->drawAxes();

        // === plot ===
        if (data != NULL) {
            size_t nData = data->size();

            size_t n = width * height;
            if (pixels.size() != n)
                pixels = vector<int>(n);
            else
                std::fill(pixels.begin(), pixels.end(), 0);
            if (rgba.size() != n)
                rgba = vector<uint32_t>(n);
            else
                std::fill(rgba.begin(), rgba.end(), 0);

            // === render into a bitmap starting (0, 0) ===
            const proj<float> pPixmap(x0, y0, x1, y1, /*screenX0*/ 0, /*screenY0*/ height, /*screenX1*/ width, /*screenY1*/ 0);

            // === mark pixels that show a data point ===
            //  O{nData} but can be parallelized. For a 10M dataset, 1000 repetitions: 29 secs 1 thread) > 6 s (8 threads)
            int nTasks = 1;
            if (nData < 10000)
                nTasks = 1;
            else if (nData < 100000)
                nTasks = 4;
            else
                nTasks = 8;

            vector<std::future<void>> jobs;
            size_t chunk = std::ceil((float)nData / nTasks);
            for (int ixTask = 0; ixTask < nTasks; ++ixTask) {
                size_t taskIxStart = ixTask * chunk;
                size_t taskIxEnd = std::min(taskIxStart + chunk, nData);

                auto job = [this, width, height, pPixmap, taskIxStart, taskIxEnd]() {
                    for (size_t ix = taskIxStart; ix < taskIxEnd; ++ix) {
                        float plotX = (float)(ix + 1);
                        float plotY = (*data)[ix];
                        int pixX = pPixmap.projX(plotX);
                        int pixY = pPixmap.projY(plotY);
                        if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                            pixels[pixY * width + pixX] = 1;
                    }
                };
                jobs.push_back(std::async(job));
            }  // for ixTask
            for (auto it = jobs.begin(); it != jobs.end(); ++it)
                (*it).get();

            // === convert to RGBA image ===
            size_t ixMax = pixels.size();
            for (size_t ix = 0; ix < ixMax; ++ix)
                if (pixels[ix])
                    // AABBGGRR
                    rgba[ix] = 0xFF00FF00;

            // === render the RGBA image repeatedly, according to the stencil ===
            fl_push_clip(screenX, screenY, width, height);
            Fl_RGB_Image im((const uchar*)&rgba[0], width, height, 4);
            const char* tmp = marker;
            int delta = markerSize1d / 2;
            const char* m = this->marker;
            for (int dy = -delta; dy <= delta; ++dy)
                for (int dx = -delta; dx <= delta; ++dx)
                    if (*(m++) != ' ')  // any non-space character in the stencil creates a shifted replica
                        im.draw(p.getScreenX0() + dx, p.getScreenY1() + dy);
            fl_pop_clip();
            cachedImage.capture(screenX, screenY, width, height);
        }
        needFullRedraw = false;
    skipDataDrawing:
        // === draw zoom rectangle ===
        fl_color(FL_WHITE);
        proj<double> pd = projDataToScreen<double>();
        double xr0, yr0, xr1, yr1;
        if (evtMan.drawRect(xr0, yr0, xr1, yr1)) {
            int xs0 = pd.projX(xr0);
            int ys0 = pd.projY(yr0);
            int xs1 = pd.projX(xr1);
            int ys1 = pd.projY(yr1);
            fl_rect(std::min(xs0, xs1), std::min(ys0, ys1), std::abs(xs1 - xs0), std::abs(ys1 - ys0));
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
            if (!std::isinf(v) && !std::isnan(v)) {
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
    //* transformation from data to screen */
    template <typename T>
    class proj {
        T dataX0, dataY0, dataX1, dataY1;
        int screenX0, screenY0, screenX1, screenY1;
        T mXData2screen;
        T bXData2screen;
        T mYData2screen;
        T bYData2screen;
        // transformation:
        // screen = (data-data1)/(data2-data1)*(screen2-screen1)+screen1;
        // screen = data * (screen2-screen1)/(data2-data1) + screen1 - data1 * (screen2-screen1)/(data2-data1)
       public:
        proj(T dataX0, T dataY0, T dataX1, T dataY1, int screenX0, int screenY0, int screenX1, int screenY1) : dataX0(dataX0), dataY0(dataY0), dataX1(dataX1), dataY1(dataY1), screenX0(screenX0), screenY0(screenY0), screenX1(screenX1), screenY1(screenY1), mXData2screen((screenX1 - screenX0) / (dataX1 - dataX0)), bXData2screen(screenX0 - dataX0 * (screenX1 - screenX0) / (dataX1 - dataX0)), mYData2screen((screenY1 - screenY0) / (dataY1 - dataY0)), bYData2screen(screenY0 - dataY0 * (screenY1 - screenY0) / (dataY1 - dataY0)) {}
        //** projects data to screen */
        inline int projX(T x) const {
            return x * mXData2screen + bXData2screen + 0.5f;
        }
        //** projects data to screen */
        inline int projY(T y) const {
            return y * mYData2screen + bYData2screen + 0.5f;
        }

        //* projects screen to data */
        inline T unprojX(int xMouse) const {
            xMouse = std::min(xMouse, std::max(screenX0, screenX1));
            xMouse = std::max(xMouse, std::min(screenX0, screenX1));
            return (xMouse - bXData2screen) / mXData2screen;
        }
        //* projects screen to data */
        inline T unprojY(int yMouse) const {
            yMouse = std::min(yMouse, std::max(screenY0, screenY1));
            yMouse = std::max(yMouse, std::min(screenY0, screenY1));
            return (yMouse - bYData2screen) / mYData2screen;
        }
        inline int getScreenWidth() {
            return std::abs(screenX1 - screenX0);
        }
        inline int getScreenHeight() {
            return std::abs(screenY1 - screenY0);
        }
        inline int getScreenX0() {
            return screenX0;
        }
        inline int getScreenY1() {
            return screenY1;
        }
    };

    vector<int> pixels;  // note, int is here slightly faster than char
    vector<uint32_t> rgba;

    const char* marker = "X";
    int markerSize1d = 1;
    float fontsize = 14;
    int axisMarginLeft;
    int axisMarginBottom;
    const int minorTicLength = 3;
    const int majorTicLength = 7;

    double x0 = -1;
    double x1 = 2;
    double y0 = 1.23;
    double y1 = 1.24;
    bool needFullRedraw = true;
    double autorange_y0 = 0;
    double autorange_y1 = 1;
    const std::vector<float>* data;

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
                // note: don't make the interval wider, otherwise an axis looks like a tic when it's not.
                if (isInRange(startVal - 0.001 * ticDelta, endVal + 0.001 * ticDelta, gridVal))
                    r.push_back(gridVal);
                gridVal += ticDelta;
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
};
}  // namespace aCCb