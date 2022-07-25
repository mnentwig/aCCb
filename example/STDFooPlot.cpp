#include <cassert>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "../aCppCookbook/aCCb/binIo.hpp"
#include "../aCppCookbook/aCCb/cmdLineParsing.hpp"
#include "../aCppCookbook/aCCb/plot2d.hpp"
#include "../aCppCookbook/aCCb/stringToNum.hpp"
#include "../aCppCookbook/aCCb/stringUtil.hpp"
#include "../aCppCookbook/aCCb/vectorText.hpp"
#include "../aCppCookbook/aCCb/widget.hpp"
#include "plot2d/syncFile.hpp"
using std::string, std::vector, std::array, std::cout, std::endl, std::runtime_error, std::map, std::pair, std::cerr;

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

class myTestWin {
   public:
    myTestWin(const string &syncfname, const string &persistfname, int windowX, int windowY, int windowW, int windowH) : syncfile(syncfname), persistfile(persistfname) {
        int areaW, areaH;
        if ((windowX > 0) && (windowY > 0) && (windowW > 0) && (windowH > 0)) {
            window = new Fl_Double_Window(windowX, windowY, windowW, windowH);
            areaW = windowW;
            areaH = windowH;
        } else if ((windowW > 0) && (windowH > 0)) {
            window = new Fl_Double_Window(windowX, windowY);
            areaW = windowW;
            areaH = windowH;
        } else {
            int dim = std::min(Fl::w(), Fl::h());
            window = new Fl_Double_Window(dim / 2, dim / 2);
            areaW = dim / 2;
            areaH = dim / 2;
        }

        window->color(FL_BLACK);
        tb = new aCCb::plot2d(0, 0, areaW, areaH);
        tb->setTitleUpdateWindow(window);
        // this->menu = new myMenu(800, 0, 400, 200);
        // menu->box(Fl_Boxtype::FL_BORDER_FRAME);
        // menu->color(FL_GREEN);
        window->resizable(tb);
        window->callback(cb_closeWrapper, (void *)this);
        window->end();
        Fl::add_timeout(0.1, timer_cb, (void *)this);
    }

    void cb_close() {
        if (persistfile != "") {
            std::ofstream s(persistfile);
            s << "-windowX " << window->x()
              << " -windowY " << window->y()
              << " -windowW " << window->w()
              << " -windowH " << window->h();
        }
        window->hide();
    }

    void shutdown() {
        Fl::remove_timeout(timer_cb);
        tb->shutdown();
    }

    static void cb_closeWrapper(Fl_Widget *w, void *userdata) {
        myTestWin *this_ = (myTestWin *)userdata;
        this_->cb_close();
    }

    static void timer_cb(void *userdata) {
        myTestWin *this_ = (myTestWin *)userdata;
        this_->tb->timer_cb();
        if (this_->syncfile.isModified()) {
            this_->window->hide();  // all windows are hidden => Fl::run() returns
        } else
            Fl::repeat_timeout(0.1, timer_cb, userdata);
    }

    void show() {
        window->show();
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

// ==============================================================================
// command line parser: '-trace' structure
// ==============================================================================
class trace : public aCCb::argObj {
   public:
    trace() : argObj("-trace") {}
    bool acceptArg_stateUnset(const string &a) {
        if (std::find(switchArgs.cbegin(), switchArgs.cend(), a) != switchArgs.cend()) {
            // implement switches here
        } else if (std::find(stateArgs.cbegin(), stateArgs.cend(), a) != stateArgs.cend()) {
            state = a;
        } else {
            return argObj::acceptArg_stateUnset(a);
        };
        return true;
    }

    bool acceptArg_stateSet(const string &a) {
        if (state == "-dataX")
            dataX = std::filesystem::canonical(a).string();
        else if (state == "-dataY")
            dataY = std::filesystem::canonical(a).string();
        else if (state == "-marker")
            marker = a;
        else if (state == "-horLineY") {
            float v;
            if (!aCCb::str2num(a, v)) throw aoException(state + ": failed to parse number ('" + a + "')");
            horLineY.push_back(v);
        } else if (state == "-vertLineX") {
            float v;
            if (!aCCb::str2num(a, v)) throw aoException(state + ": failed to parse number ('" + a + "')");
            vertLineX.push_back(v);
        } else if (state == "-annot")
            annotate = a;
        else
            throw runtime_error("state implementation missing: " + state);
        state = "";
        return true;
    }

    string dataX;
    string dataY;
    string marker;
    vector<float> horLineY;
    vector<float> vertLineX;
    string annotate;

   protected:
    const vector<string> stateArgs{"-dataX", "-dataY", "-marker", "-horLineY", "-vertLineX", "-annot"};
    const vector<string> switchArgs;
};

// ==============================================================================
// command line parser: root level
// ==============================================================================
class loader : public aCCb::argObj {
   public:
    loader() : argObj("cmdline root") {}
    bool acceptArg_stateUnset(const string &a) {
        if (std::find(switchArgs.cbegin(), switchArgs.cend(), a) != switchArgs.cend()) {
            if (a == "-trace") {
                traces.push_back(trace());  // note: container may not invalidate iterators on insertion e.g. DO NOT use vector
                stack.push_back(&traces.back());
            } else if (a == "-help") {
                showUsage = true;
            } else
                throw new runtime_error(token + "unsupported switch '" + a + "'");
        } else if (std::find(stateArgs.cbegin(), stateArgs.cend(), a) != stateArgs.cend()) {
            state = a;
        } else
            return argObj::acceptArg_stateUnset(a);
        return true;
    }

    bool acceptArg_stateSet(const string &a) {
        if (state == "-title") {
            title = a;
        } else if (state == "-xlabel") {
            xlabel = a;
        } else if (state == "-ylabel") {
            ylabel = a;
        } else if (state == "-xLimLow") {
            if (!aCCb::str2num(a, xLimLow)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-xLimHigh") {
            if (!aCCb::str2num(a, xLimHigh)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-yLimLow") {
            if (!aCCb::str2num(a, yLimLow)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-yLimHigh") {
            if (!aCCb::str2num(a, yLimHigh)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-windowX") {
            if (!aCCb::str2num(a, windowX)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-windowY") {
            if (!aCCb::str2num(a, windowY)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-windowW") {
            if (!aCCb::str2num(a, windowW)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-windowH") {
            if (!aCCb::str2num(a, windowH)) throw aoException(state + ": failed to parse number ('" + a + "')");
        } else if (state == "-sync") {
            syncfile = a;
        } else if (state == "-persist") {
            persistfile = a;
        } else
            return argObj::acceptArg_stateSet(a);
        state = "";
        return true;
    }

    const vector<string> stateArgs{"-title", "-xlabel", "-ylabel", "-xLimLow", "-xLimHigh", "-yLimLow", "-yLimHigh", "-sync", "-persist", "-windowX", "-windowY", "-windowW", "-windowH"};
    const vector<string> switchArgs{"-trace", "-help"};
    string title;
    string xlabel;
    string ylabel;
    double xLimLow = std::numeric_limits<double>::quiet_NaN();
    double xLimHigh = std::numeric_limits<double>::quiet_NaN();
    double yLimLow = std::numeric_limits<double>::quiet_NaN();
    double yLimHigh = std::numeric_limits<double>::quiet_NaN();
    int windowX = -1;
    int windowY = -1;
    int windowW = -1;
    int windowH = -1;
    string syncfile;
    string persistfile;
    std::deque<trace> traces;
    bool showUsage = false;
};

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

class markerMan_cl {
   public:
    markerMan_cl() {
        // =======================================
        // set up markers
        // =======================================
        const vector<pair<char, uint32_t>> colors{
            {'k', 0xFF222222},
            {'r', 0xFF0000FF},
            {'g', 0xFF00FF00},
            {'b', 0xFFFF0000},
            {'c', 0xFFFFFF00},
            {'m', 0xFF00FFFF},
            {'y', 0xFFFF00FF},
            {'a', 0xFF888888},
            {'o', 0xFF008cFF},  // orange
            {'w', 0xFFFFFFFF}};

        const string dot = ".";
        const string plus = "+";
        const string cross = "x";
        for (auto x : colors) {
            char colCode = x.first;
            uint32_t rgba = x.second;
            string sequence;
            string key;

            sequence = "X";
            key = colCode + dot + "1";
            markers[key] = new marker_cl(sequence, rgba);
            markers[colCode + dot] = markers[key];

            sequence =
                "XXX"
                "XXX"
                "XXX";
            key = colCode + dot + "2";
            markers[key] = new marker_cl(sequence, rgba);

            sequence =
                " XXX "
                "XXXXX"
                "XXXXX"
                "XXXXX"
                " XXX ";
            key = colCode + dot + "3";
            markers[key] = new marker_cl(sequence, rgba);

            sequence =
                " X "
                "XXX"
                " X ";
            key = colCode + plus + "1";
            markers[key] = new marker_cl(sequence, rgba);
            markers[colCode + plus] = markers[key];

            sequence =
                "  X  "
                "  X  "
                "XXXXX"
                "  X  "
                "  X  ";
            key = colCode + plus + "2";
            markers[key] = new marker_cl(sequence, rgba);

            sequence =
                "X X"
                " X "
                "X X";
            key = colCode + cross + "1";
            markers[key] = new marker_cl(sequence, rgba);
            markers[colCode + cross] = markers[key];

            sequence =
                "X   X"
                " X X "
                "  X  "
                " X X "
                "X   X";
            key = colCode + cross + "2";
            markers[key] = new marker_cl(sequence, rgba);

        }  // for colors
    }

    const marker_cl *getMarker(const string &desc) const {
        cout << "marker search " << desc << endl;
        auto it = markers.find(desc);
        if (it == markers.end())
            return NULL;
        cout << "marker found " << desc << endl;
        return it->second;
    }

    ~markerMan_cl() {
        // markers contains aliases e.g. "w." for "w.1"
        // collect unique markers
        std::set<marker_cl *> uniqueMarkers;
        for (auto v : markers)
            uniqueMarkers.insert(v.second);
        // ... and clean up
        for (auto v : uniqueMarkers)
            delete v;
    }

   protected:
    map<string, marker_cl *> markers;
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
    loader l;

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

    //* GUI drawing code */
    myTestWin w(l.syncfile, l.persistfile, l.windowX, l.windowY, l.windowW, l.windowH);

    //* stores all trace data */
    traceDataMan_cl traceDataMan;

    //* provides all markers */
    markerMan_cl markerMan;

    try {  // above starts background thread. Need to shut down on exception

        for (auto t : l.traces) {
            const marker_cl *m = markerMan.getMarker(t.marker);
            if (!m)
                throw aCCb::argObjException("invalid marker description '" + t.marker + "'. Valid example: g.1");
            traceDataMan.loadData(t.dataX);
            traceDataMan.loadData(t.dataY);
            traceDataMan.loadAnnotations(t.annotate);
            w.tb->addTrace(traceDataMan.getData(t.dataX), traceDataMan.getData(t.dataY), traceDataMan.getAnnotations(t.annotate), m, t.vertLineX, t.horLineY);
        }

        // === autoscale ===
        if (std::isnan(l.xLimLow) | std::isnan(l.xLimHigh) | std::isnan(l.yLimLow) | std::isnan(l.yLimHigh))
            w.tb->autoscale();

        // === set fixed range ===
        if (!std::isnan(l.xLimLow))
            w.tb->x0 = l.xLimLow;
        if (!std::isnan(l.xLimHigh))
            w.tb->x1 = l.xLimHigh;
        if (!std::isnan(l.yLimLow))
            w.tb->y0 = l.yLimLow;
        if (!std::isnan(l.yLimHigh))
            w.tb->y1 = l.yLimHigh;

        w.tb->setTitle(l.title);
        w.tb->setXlabel(l.xlabel);
        w.tb->setYlabel(l.ylabel);
    } catch (std::exception &e) {
        w.shutdown();
        throw;
    }
    w.show();
    Fl::run();
    w.shutdown();
    return 0;
}

int main(int argc, const char **argv) {
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