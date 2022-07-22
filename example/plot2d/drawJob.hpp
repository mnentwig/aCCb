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
typedef uint8_t stencil_t; // bool: 32 ms; uint8: 4.5 ms; uint16: 6 ms uint32_t: 9 ms uint64_t: 16 ms
class drawJob {
   protected:
    // multithreaded job description.
    // passed by value - don't put anything large inside
    class job_t {
       public:
        job_t(size_t ixStart, size_t ixEnd, const vector<float>* pDataX, const vector<float>* pDataY, const proj<float> p, vector<stencil_t>* pStencil)
            : ixStart(ixStart),
              ixEnd(ixEnd),
              pDataX(pDataX),
              pDataY(pDataY),
              p(p),
              pStencil(pStencil) {}

        const size_t ixStart;
        const size_t ixEnd;
        const vector<float>* pDataX;
        const vector<float>* pDataY;
        const proj<float> p;
        vector<stencil_t>* pStencil;
    };

    static void drawDotsXY(const job_t job) {
        assert(job.pDataX->size() == job.pDataY->size());
        const int width = job.p.getScreenWidth();
        const int height = job.p.getScreenHeight();

        for (size_t ix = job.ixStart; ix < job.ixEnd; ++ix) {
            float plotX = (*(job.pDataX))[ix];
            float plotY = (*(job.pDataY))[ix];
            int pixX = job.p.projX(plotX);
            int pixY = job.p.projY(plotY);
            if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                (*job.pStencil)[pixY * width + pixX] = 1;
        }  // for ix
    }

    static void drawDotsY(const job_t job) {
        const int width = job.p.getScreenWidth();
        const int height = job.p.getScreenHeight();

        for (size_t ix = job.ixStart; ix < job.ixEnd; ++ix) {
            float plotX = ix + 1;
            float plotY = (*(job.pDataY))[ix];
            int pixX = job.p.projX(plotX);
            int pixY = job.p.projY(plotY);
            if ((pixX >= 0) && (pixX < width) && (pixY >= 0) && (pixY < height))
                (*job.pStencil)[pixY * width + pixX] = 1;
        }  // for ix
    }

   public:
    drawJob(const vector<float>* pDataX, const vector<float>* pDataY, const marker_cl* marker) : marker(marker), pDataX(pDataX), pDataY(pDataY) {}

    void drawData2stencil(const proj<float> p, vector<stencil_t>& stencil) {
        size_t nData = pDataY->size();

        // === mark pixels that show a data point ===
        const size_t chunk = 65536 * 16;
        vector<job_t> jobs;
        vector<std::future<void>> futs;
        for (size_t chunkIxStart = 0; chunkIxStart < nData; chunkIxStart += chunk) {
            size_t chunkIxEnd = std::min(chunkIxStart + chunk, nData);
            job_t job(chunkIxStart, chunkIxEnd, pDataX, pDataY, p, &stencil);
            jobs.push_back(job);
            if (pDataX)
                futs.push_back(std::async(drawDotsXY, jobs.back()));
            else
                futs.push_back(std::async(drawDotsY, jobs.back()));
        }
        for (std::future<void>& f : futs)
            f.get();
    }

    static vector<stencil_t> convolveStencil(const vector<stencil_t>& stencil, int width, int height, const marker_cl* marker) {
        vector<stencil_t> r(stencil);  // center pixel
//        return r;
        int count = 0;
        for (int dx = -marker->dxMinus; dx <= marker->dxPlus; ++dx) {
            for (int dy = -marker->dxMinus; dy <= marker->dxPlus; ++dy) {
                if ((dx == 0) && (dy == 0))
                    continue; // center pixel is set by return value constructor
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
                            r[ixDest] = r[ixDest] | stencil[ixSrc];
                            ++ixDest;
                            ++ixSrc;
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
    drawRgba2screen(const vector<uint32_t>& rgba, int screenX, int screenY, int screenWidth, int screenHeight) {
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
        for (float y : *pDataY) {
            if (!std::isinf(y) && !std::isnan(y)) {
                y0f = std::min(y0f, y);
                y1f = std::max(y1f, y);
            }
        }
        if (pDataX == NULL) {
            x0f = std::min(x0f, 1.0f);
            x1f = std::max(x1f, (float)(pDataY->size() + 1));
        } else {
            for (float x : *pDataX) {
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
    const vector<float>* pDataX;
    const vector<float>* pDataY;
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
                drawJob::drawRgba2screen(rgba, screenX, screenY, screenWidth, screenHeight);
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
            drawJob::drawRgba2screen(rgba, screenX, screenY, screenWidth, screenHeight);
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