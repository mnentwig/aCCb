#include <filesystem>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "../aCCb/binIo.hpp"
#include "../aCCb/cmdLineParsing.hpp"
#include "../aCCb/plot2d.hpp"
#include "../aCCb/plot2d/syncFile.hpp"
#include "../aCCb/stringToNum.hpp"
#include "../aCCb/stringUtil.hpp"
#include "../aCCb/vectorText.hpp"
#include "../aCCb/widget.hpp"
#include "STDFooPlot/cmdLineProcessor.hpp"
#include "STDFooPlot/markerMan.hpp"
#include "STDFooPlot/traceMan.hpp"
using std::string, std::vector, std::array, std::cout, std::endl, std::runtime_error, std::map, std::pair, std::cerr;

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#if 0
class myMenu : public aCCbWidget {
   public:
    myMenu(int x, int y, int w, int h) : aCCbWidget(x, y, w, h) {
        this->labelcolor(FL_GREEN);  // propagates to widgets
        cur = cursor(x, y, /*column width*/ 100, /*row height*/ 20);
        this->cb1 = create("log", 50, 0);
        this->cb2 = create("log", 50, 0);
        this->cb3 = create("log", X0);
        cur.move(DYL);  // space for label on top
        this->i1 = create("my text", DX);
        this->i2 = create("my text", X0);
        cur.move(DYL);  // space for label on top
        this->fi = create("my float", X0);
        // Fl_Button *b =
        (Fl_Button *)create("Click me!", DY);
    }
    void callbackButton(Fl_Button *) {
    }
    void callbackCheckbutton(Fl_Check_Button *) {
    }
    void callbackInput(Fl_Input *) {
        std::cout << "inp" << std::endl;
    }

   protected:
    Fl_Button *bPickFile;
    Fl_Check_Button *cb1;
    Fl_Check_Button *cb2;
    Fl_Check_Button *cb3;
    Fl_Input *i1;
    Fl_Input *i2;
    Fl_Float_Input *fi;
};
#endif
class myTestWin {
   public:
    myTestWin(fooplotCmdLineArgRoot &l, allDrawJobs_cl &adr) : syncfile(l.syncfile), persistfile(l.persistfile) {
        // === main window ===
        int areaW, areaH;
        if ((l.windowX > 0) && (l.windowY > 0) && (l.windowW > 0) && (l.windowH > 0)) {
            window = new Fl_Double_Window(l.windowX, l.windowY, l.windowW, l.windowH);
            areaW = l.windowW;
            areaH = l.windowH;
        } else if ((l.windowW > 0) && (l.windowH > 0)) {
            window = new Fl_Double_Window(l.windowX, l.windowY);
            areaW = l.windowW;
            areaH = l.windowH;
        } else {
            int dim = std::min(Fl::w(), Fl::h());
            window = new Fl_Double_Window(dim / 2, dim / 2);
            areaW = dim / 2;
            areaH = dim / 2;
        }
        window->callback(cb_closeWrapper, (void *)this);
        window->color(FL_BLACK);

        // === plot area ===
        tb = new aCCb::plot2d(0, 0, areaW, areaH, adr);
        if (!std::isnan(l.xLimLow))
            tb->x0 = l.xLimLow;
        if (!std::isnan(l.xLimHigh))
            tb->x1 = l.xLimHigh;
        if (!std::isnan(l.yLimLow))
            tb->y0 = l.yLimLow;
        if (!std::isnan(l.yLimHigh))
            tb->y1 = l.yLimHigh;

        tb->setTitle(l.title);
        tb->setXlabel(l.xlabel);
        tb->setYlabel(l.ylabel);
        if (std::isnan(l.xLimLow) | std::isnan(l.xLimHigh) | std::isnan(l.yLimLow) | std::isnan(l.yLimHigh))
            tb->autoscale();
        tb->setTitleUpdateWindow(window);

        // this->menu = new myMenu(800, 0, 400, 200);
        // menu->box(Fl_Boxtype::FL_BORDER_FRAME);
        // menu->color(FL_GREEN);
        window->resizable(tb);
        window->end();
        Fl::add_timeout(0.1, cb_timerWrapper, (void *)this);
    }

    void show() {
        window->show();
    }

    void cb_close() {
        window->hide();
    }

    void shutdown() {
        // need to stop background processes, before removal of the window triggers destructors
        Fl::remove_timeout(cb_timerWrapper);
        tb->shutdown();
    }

    static void cb_closeWrapper(Fl_Widget *w, void *userdata) {
        myTestWin *this_ = (myTestWin *)userdata;
        this_->cb_close();
    }

    void cb_timer() {
        tb->cb_timer();
        if (syncfile.isModified())
            cb_close();  // all windows are hidden => Fl::run() returns
        else
            Fl::repeat_timeout(0.02, cb_timerWrapper, (void *)this);

        if (persistfile != "") {
            std::ofstream s(persistfile);
            s << "-windowX " << window->x()
              << " -windowY " << window->y()
              << " -windowW " << std::max(window->w(), 300)
              << " -windowH " << std::max(window->h(), 300);
        }
    }

    static void cb_timerWrapper(void *userdata) {
        assert(userdata);
        ((myTestWin *)userdata)->cb_timer();
    }

    ~myTestWin() {
        delete this->window;  // deletes children recursively
    }
    aCCb::plot2d *tb;

   protected:
    aCCbWidget *menu;
    Fl_Double_Window *window;
    syncFile_t syncfile;
    string persistfile;
};

//* returns file contents split by whitespace */
vector<string> readPersistFileTokens(const string &filename) {
    std::ifstream is(filename, std::ios::binary);
    if (!is)
        return vector<string>();  // persistfile does not exist the first time, will be created

    // === file to string ===
    std::ostringstream all;
    all << is.rdbuf();
    string content = all.str();

    // === split at whitespace ===
    std::regex r("\\s+");  // cannot use a temporary expression (one-liner)
    return {std::sregex_token_iterator(content.begin(), content.end(), r, -1), /*equiv. to end()*/ std::sregex_token_iterator()};
}

void usage() {
    cerr << "usage:" << endl;
    cerr << "-trace" << endl;
    cerr << "   -dataX (filename)" << endl;
    cerr << "   -dataY (filename)" << endl;
    cerr << "   -vertLineY (number) repeated use is allowed" << endl;
    cerr << "   -horLineX (number) repeated use is allowed" << endl;
    cerr << "   -marker (e.g. w.1 see [1])" << endl;
    cerr << "   -annot (filename)" << endl;
    cerr << "-xlabel (text)" << endl;
    cerr << "-ylabel (text)" << endl;
    cerr << "-title (text)" << endl;
    cerr << "-sync (filename)" << endl;
    cerr << "-persist (filename)" << endl;
    cerr << endl;
    cerr << "[1] colors in place of 'w': krgbcmyaow" << endl;
    cerr << "    shapes in place of '.1': .1 .2 .3 +1 +2 x1 x2 ('1' can be omitted')" << endl;
}

static myTestWin *windowForSigIntHandler;
int main2(int argc, const char **argv) {
#if 0
    const char *tmp[] = {"execname",
                         "-trace", "-dataY", "out2.float", "-marker", "g.3",
                         "-trace", "-dataY", "y.txt", "-dataX", "x.txt", "-marker", "wx1", /*"-vertLineY", "-1", "-vertLineY", "1",*/ "-annot", "x.txt",
                         "-trace", "-vertLineX", "-3", "-vertLineX", "3", "-horLineY", "-3", "-horLineY", "3", "-marker", "o.1",
                         "-title", "this is the title!", "-xlabel", "the xlabel", "-ylabel", "and the ylabel", "-xLimLow", "-200000", "-sync", "b.txt",
                         // "-windowX", "10", "-windowY", "20", "-windowW", "1800", "-windowH", "1000",
                         "-persist", "c.txt"};
#else
    const char *tmp[] = {"execname",
                         "-trace", "-marker", "g.3", "-dataX", "x.txt", "-dataY", "y.txt", "-sync", "b.txt", "-persist", "c.txt"};
#endif
    if (argc < 2) {
        cout << "*** debug cmd line args ***" << endl;
        argv = tmp;
        argc = sizeof(tmp) / sizeof(tmp[0]);
    }
    Fl::visual(FL_RGB);

    //* collects command line arguments */
    fooplotCmdLineArgRoot l;

    // === parse command line args ===
    for (int ixArg = 1; ixArg < argc; ++ixArg) {
        string a = argv[ixArg];
        cout << "parsing " << a << endl;
        if (!l.acceptArg(a))
            throw aCCb::argObjException("unexpected argument '" + a + "'");
    }

    if (l.showUsage) {
        usage();  // note: -usage, even at the end, is handled before any args processing that could throw an exception
        exit(/*EXIT_SUCCESS*/ 0);
    }

    // === read and apply "persistent" settings e.g. window position ===
    // those are applied after all command line args have been handled
    // (use model: delete persist file to reset)
    if (l.persistfile != "") {
        vector<string> tokens = readPersistFileTokens(l.persistfile);
        for (string v : tokens)
            if (!l.acceptArg(v))
                throw aCCb::argObjException("persist file error (" + l.persistfile + ") : unexpected token '" + v + "'");
    }

    l.close();

    // === variables below must remain in scope until shutdown ===

    //* stores all trace data */
    traceDataMan_cl traceDataMan;

    //* provides all markers */
    markerMan_cl markerMan;

    allDrawJobs_cl allDrawJobs;

    for (auto t : l.traces) {
        const marker_cl *marker = markerMan.getMarker(t.marker);
        if (!marker)
            throw aCCb::argObjException("invalid marker description '" + t.marker + "'. Valid example: g.1");
        traceDataMan.loadData(t.dataX);
        traceDataMan.loadData(t.dataY);
        traceDataMan.loadAnnotations(t.annotate);
        drawJob j(traceDataMan.getData(t.dataX), traceDataMan.getData(t.dataY), traceDataMan.getAnnotations(t.annotate), marker, t.vertLineX, t.horLineY);
        allDrawJobs.addTrace(j);
    }

    // === start up window ===
    // background thread running
    myTestWin w(l, allDrawJobs);
    windowForSigIntHandler = &w;
    w.show();

    // === main loop ===
    Fl::run();

    // === shutdown ===
    // background thread reaped
    w.shutdown();
    return 0;
}

// Ctrl-C callback
void sigIntHandler(int /*signal*/) {
    std::cout << "sig INT detected, shutting down" << endl;
    if (windowForSigIntHandler)
        windowForSigIntHandler->cb_close();  // same as regular close
    exit(1);
}

int main(int argc, const char **argv) {
    windowForSigIntHandler = NULL;
    signal(SIGINT, sigIntHandler);

    try {
        main2(argc, argv);
    } catch (aCCb::argObjException &e) {
        cerr << "error: " << e.what() << "\n";
        cerr << "use -help for usage information" << endl;
        exit(/*EXIT_FAILURE*/ -1);
    } catch (std::exception &e) {
        cerr << "unhandled exception: " << e.what() << endl;
        exit(/*EXIT_FAILURE*/ -1);
    }
    return 0;
}