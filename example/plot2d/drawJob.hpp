#pragma once
#include <FL/Fl_Image.H>

#include <cassert>
#include <cmath>  // ceil
#include <future>
#include <string>
#include <vector>

#include "plot2d/marker.hpp"
#include "plot2d/proj.hpp"
using std::vector, std::string;
typedef uint8_t stencil_t;
class drawJob {
   public:
    drawJob(const vector<float>* dataX, const vector<float>* dataY, const marker_cl* marker) : marker(marker), dataX(dataX), dataY(dataY) {}

    void drawData2stencil(const proj<float> p, vector<stencil_t>& stencil) {
        size_t nData = dataY->size();
        int width = p.getScreenWidth();
        int height = p.getScreenHeight();

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

            if (dataX) {
                assert(dataX->size() == dataY->size());
                auto job = [this, &stencil, width, height, &p, taskIxStart, taskIxEnd]() {
                    for (size_t ix = taskIxStart; ix < taskIxEnd; ++ix) {
                        float plotX = (*(this->dataX))[ix];
                        float plotY = (*(this->dataY))[ix];
                        int pixX = p.projX(plotX);
                        int pixY = p.projY(plotY);
                        if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                            stencil[pixY * width + pixX] = 1;
                    }
                };
                jobs.push_back(std::async(job));
            } else {
                auto job = [this, &stencil, width, height, &p, taskIxStart, taskIxEnd]() {
                    for (size_t ix = taskIxStart; ix < taskIxEnd; ++ix) {
                        const float plotX = (float)(ix + 1);
                        const float plotY = (*(this->dataY))[ix];
                        const int pixX = p.projX(plotX);
                        const int pixY = p.projY(plotY);
                        if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                            stencil[pixY * width + pixX] = 1;
                    }
                };
                jobs.push_back(std::async(job));
            }
        }  // for ixTask
        for (auto it = jobs.begin(); it != jobs.end(); ++it)
            (*it).get();
    }

    static vector<stencil_t> convolveStencil(const vector<stencil_t>& stencil, int width, int height, const marker_cl* marker) {
        vector<stencil_t> r(stencil.size());
        int count = 0;
        for (int dx = -marker->dxMinus; dx <= marker->dxPlus; ++dx) {
            for (int dy = -marker->dxMinus; dy <= marker->dxPlus; ++dy) {
                if (marker->seq[count++]) {
                    int ixSrc = 0;
                    int ixDest = 0;
                    int absDx = std::abs(dx);
                    int absDy = std::abs(dy);
                    if (dx < 0)
                        ixSrc += absDx;
                    if (dx > 0)
                        ixDest += absDx;
                    if (dy < 0)
                        ixSrc += absDy * width;
                    if (dy > 0)
                        ixDest += absDy * width;
                    for (int ixRow = height - absDy; ixRow != 0; --ixRow) {
                        for (int ixCol = width - absDx; ixCol != 0; --ixCol) {
                            assert(ixSrc >= 0);
                            assert(ixDest >= 0);
                            assert(ixSrc < (int)r.size());
                            assert(ixDest < (int)r.size());
                            r[ixDest++] |= stencil[ixSrc++];
                        }  // for column
                        ixDest += absDx;
                        ixSrc += absDx;
                    }  // for row
                }      // if marker pixel enabled
            }          // for marker column
        }              // for marker row
        return r;
    }

    static void drawStencil2rgba(const vector<stencil_t>& stencil, int width, int height, const marker_cl* marker, vector<uint32_t>& rgba) {
        assert((int)stencil.size() == width * height);
        assert((int)rgba.size() == width * height);
        uint32_t markerRgba = marker->rgba;

        // === convert to RGBA image ===
        size_t ixMax = stencil.size();
        for (size_t ix = 0; ix < ixMax; ++ix)
            rgba[ix] = stencil[ix] ? markerRgba : 0;
    }

    //* render the RGBA image repeatedly, as defined by the marker sequence */
    static void
    drawRgba2screen(const vector<uint32_t>& rgba, int screenX, int screenY, int screenWidth, int screenHeight, const marker_cl* marker) {
        assert(screenWidth * screenHeight == (int)rgba.size());
        Fl_RGB_Image im((const uchar*)&rgba[0], screenWidth, screenHeight, 4);
        im.draw(screenX, screenY);
    }

    /** given limits are extended to include data */
    void updateAutoscale(double& x0, double& x1, double& y0, double& y1) const {
        const float inf = std::numeric_limits<float>::infinity();
        float x0f = inf;
        float x1f = -inf;
        float y0f = inf;
        float y1f = -inf;
        for (float y : *dataY) {
            if (!std::isinf(y) && !std::isnan(y)) {
                y0f = std::min(y0f, y);
                y1f = std::max(y1f, y);
            }
        }
        if (dataX == NULL) {
            x0f = std::min(x0f, 1.0f);
            x1f = std::max(x1f, (float)(dataY->size() + 1));
        } else {
            for (float x : *dataX) {
                if (!std::isinf(x) && !std::isnan(x)) {
                    x0f = std::min(x0f, x);
                    x1f = std::max(x1f, x);
                }
            }
        }
        x0 = (double)x0f;
        y0 = (double)y0f;
        x1 = (double)x1f;
        y1 = (double)y1f;
    }

    const marker_cl* marker;

   protected:
    const vector<float>* dataX;
    const vector<float>* dataY;
};  // class drawJob

class allDrawJobs_cl {
   public:
    allDrawJobs_cl() : drawJobs() {}
    void draw(const proj<double>& p) {
        int screenWidth = p.getScreenWidth();
        int screenHeight = p.getScreenHeight();
        int screenX = p.getScreenX0();
        int screenY = p.getScreenY1();

        /* Projection to stencil at x=0 Y=0 */
        const proj<float> pStencil(p.getDataX0(), p.getDataY1(), p.getDataX1(), p.getDataY0(), /*stencil X0*/ 0, /*stencil Y0*/ 0, /*stencil X1*/ screenWidth, /*stencil Y1*/ screenHeight);

        // ... combine subsequent traces with same marker into a  common stencil
        vector<stencil_t> stencil(screenWidth * screenHeight);

        // ... then render the stencil using the marker into rgba
        vector<uint32_t> rgba(screenWidth * screenHeight);

        const marker_cl* currentMarker = NULL;
        for (auto it = drawJobs.begin(); it != drawJobs.end(); ++it) {
            drawJob& j = *it;
            bool stencilHoldsIncompatibleData = (currentMarker != NULL) && (currentMarker->id != j.marker->id);
            if (stencilHoldsIncompatibleData) {
                // Draw stencil...
                vector<stencil_t> sConv = drawJob::convolveStencil(stencil, screenWidth, screenHeight, currentMarker);
                drawJob::drawStencil2rgba(sConv, screenWidth, screenHeight, currentMarker, /*out*/ rgba);
                drawJob::drawRgba2screen(rgba, screenX, screenY, screenWidth, screenHeight, currentMarker);
                // ... and clear
                std::fill(stencil.begin(), stencil.end(), 0);
            }  // if incompatible with stencil contents
            j.drawData2stencil(pStencil, /*out*/ stencil);
            currentMarker = j.marker;
        }
        // render final stencil
        if (currentMarker != NULL) {
            vector<stencil_t> sConv = drawJob::convolveStencil(stencil, screenWidth, screenHeight, currentMarker);
            drawJob::drawStencil2rgba(sConv, screenWidth, screenHeight, currentMarker, /*out*/ rgba);
            drawJob::drawRgba2screen(rgba, screenX, screenY, screenWidth, screenHeight, currentMarker);
        }
    }

    void addTrace(drawJob j) {
        this->drawJobs.push_back(j);
    }

    void updateAutoscale(double& x0, double& y0, double& x1, double& y1) const {
        for (auto j : drawJobs)
            j.updateAutoscale(x0, x1, y0, y1);
    }

   protected:
    vector<drawJob>
        drawJobs;
};