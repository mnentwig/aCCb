#include "../aCppCookbook/aCCb/binIo.hpp"
#include "../aCppCookbook/aCCb/plot2d.hpp"
#include "../aCppCookbook/aCCb/stringUtil.hpp"
#include "../aCppCookbook/aCCb/vectorText.hpp"
#include "../aCppCookbook/aCCb/widget.hpp"

//#include <../aCppCookbook/aCCb/stringToNum.hpp>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
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
    myTestWin() {
        this->window = new Fl_Double_Window(1280, 800);
        this->window->color(FL_BLACK);
        this->tb = new aCCb::plot2d(0, 0, 800, 800);
        this->menu = new myMenu(800, 0, 400, 200);
        menu->box(Fl_Boxtype::FL_BORDER_FRAME);
        menu->color(FL_GREEN);
        window->resizable(this->tb);
        window->end();
    }

    void show() {
        this->window->show();
    }

    ~myTestWin() {
        delete this->window;  // deletes children recursively
    }
    aCCb::plot2d *tb;

   protected:
    aCCbWidget *menu;
    Fl_Double_Window *window;
};

// ==============================================================================
// command line parser: base class for argument
// ==============================================================================
class argObj {
   public:
    argObj(string token) : token(token), state(""), stack(), closed(false) {}

    virtual bool acceptArg_stateSet(const string & /*arg*/) {
        throw runtime_error("?? " + token + ":state implementation is missing for '" + state + "' state ??");
    }
    virtual bool acceptArg_stateUnset(const string &arg) {
        while (stack.size() > 0) {
            argObj *child = stack.back();
            // === feed the argument to the topmost element on the stack ===
            if (child->acceptArg(arg))
                return true;  // child accepted (and stays open)

            // === child is implicitly closed ===
            child->close();
            stack.pop_back();
        }

        // === handle standard close argument '-;' ===
        // note, removal from the parent's stack is delayed
        if (arg == "-end") {
            assert(stack.size() == 0);
            closed = true;
            return true;
        }

        return false;
    }
    void close() {
        if (closed) return;
        if (state != "") usage(token + ": expecting argument for '" + state + "'");
        closed = true;
        while (stack.size() > 0) {
            stack.back()->close();
            stack.pop_back();
        }
    }

    bool acceptArg(const string &arg) {
        if (closed)
            return false;
        if (state == "")
            return acceptArg_stateUnset(arg);
        else
            return acceptArg_stateSet(arg);
    }

    void usage(const string &msg) {
        exit(/*EXIT_FAILURE*/ -1);
        cerr << msg << endl;
        cerr << "usage:" << endl;
        cerr << "-trace" << endl;
        cerr << "   -dataX (filename)" << endl;
        cerr << "   -dataY (filename)" << endl;
        cerr << "   -vertLineY (number) repeated use is allowed" << endl;
        cerr << "   -horLineX (number) repeated use is allowed" << endl;
        cerr << "   -marker (e.g. w.1 see [1])" << endl;
        cerr << "-xlabel (text)" << endl;
        cerr << "-ylabel (text)" << endl;
        cerr << "-title (text)" << endl;
        cerr << endl;
        cerr << "[1] colors in place of 'w': krgbcmyaow" << endl;
        cerr << "    shapes in place of '.1': .1 .2 .3 +1 +2 x1 x2 ('1' can be omitted')" << endl;
        exit(/*EXIT_FAILURE*/ -1);
    }

   protected:
    //* friendly name for messages */
    string token;
    //* purpose of the next expected argument (if any) */
    string state;
    //* objects defined earlier on the command line that may still use arguments not understood by the current object */
    std::vector<argObj *> stack;
    bool closed;
};

// ==============================================================================
// command line parser: '-trace' structure
// ==============================================================================
class trace : public argObj {
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
            if (!aCCb::str2num(a, v)) usage("-horLineY: failed to parse number ('" + a + "')");
            horLineY.push_back(v);
        } else if (state == "-vertLineX") {
            float v;
            if (!aCCb::str2num(a, v)) usage("-vertLineX: failed to parse number ('" + a + "')");
            vertLineX.push_back(v);
        } else
            throw runtime_error("?? state implementation missing: " + state + " ??");
        state = "";
        return true;
    }

    string dataX;
    string dataY;
    string marker;
    vector<float> horLineY;
    vector<float> vertLineX;

   protected:
    const vector<string> stateArgs{"-dataX", "-dataY", "-marker", "-horLineY", "-vertLineX"};
    const vector<string> switchArgs;
};

// ==============================================================================
// command line parser: root level
// ==============================================================================
class loader : public argObj {
   public:
    loader() : argObj("") {}
    bool acceptArg_stateUnset(const string &a) {
        if (std::find(switchArgs.cbegin(), switchArgs.cend(), a) != switchArgs.cend()) {
            if (a == "-trace") {
                traces.push_back(trace());
                stack.push_back(&traces.back());
            } else
                throw new runtime_error(token + "?? unsupported switch '" + a + "'");
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
            if (!aCCb::str2num(a, xLimLow)) usage("-xLimLow: failed to parse number ('" + a + "')");
        } else if (state == "-xLimHigh") {
            if (!aCCb::str2num(a, xLimHigh)) usage("-xLimHigh: failed to parse number ('" + a + "')");
        } else if (state == "yxLimLow") {
            if (!aCCb::str2num(a, yLimLow)) usage("-yLimLow: failed to parse number ('" + a + "')");
        } else if (state == "yxLimHigh") {
            if (!aCCb::str2num(a, yLimHigh)) usage("-yLimHigh: failed to parse number ('" + a + "')");
        } else
            return argObj::acceptArg_stateSet(a);
        state = "";
        return true;
    }

    const vector<string> stateArgs{"-title", "-xlabel", "-ylabel", "-xLimLow", "-xLimHigh", "-yLimLow", "-yLimHigh"};
    const vector<string> switchArgs{"-trace"};
    string title;
    string xlabel;
    string ylabel;
    double xLimLow = std::numeric_limits<double>::quiet_NaN();
    double xLimHigh = std::numeric_limits<double>::quiet_NaN();
    double yLimLow = std::numeric_limits<double>::quiet_NaN();
    double yLimHigh = std::numeric_limits<double>::quiet_NaN();
    vector<trace> traces;
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
            throw runtime_error("unsupported data file extension");
    }

    vector<float> *getData(const string &filename) {
        if (filename == "")
            return NULL;
        auto it = dataByFilename.find(filename);
        if (it == dataByFilename.end())
            throw runtime_error("?? unknown datafile ??");
        return &(it->second);
    }

   protected:
    map<string, vector<float>> dataByFilename;
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
        auto it = markers.find(desc);
        if (it == markers.end())
            return NULL;
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

int main2(int argc, const char **argv) {
    const char *tmp[] = {"execname",
                         "-trace", "-dataY", "out2.float", "-marker", "g.3",
                         "-trace", "-dataY", "y.txt", "-dataX", "x.txt", "-marker", "wx1", /*"-vertLineY", "-1", "-vertLineY", "1",*/
                         "-trace", "-vertLineX", "-3", "-vertLineX", "3", "-horLineY", "-3", "-horLineY", "3", "-marker", "o.1",
                         "-title", "this is the title!", "-xlabel", "the xlabel", "-ylabel", "and the ylabel", "-xLimLow", "-200000"};
    argv = tmp;
    argc = sizeof(tmp) / sizeof(tmp[0]);

    Fl::visual(FL_RGB);

    //* collects command line arguments */
    loader l;

    // === parse command line args ===
    for (int ixArg = 1; ixArg < argc; ++ixArg) {
        string a = argv[ixArg];
        cout << "parsing " << a << endl;
        if (!l.acceptArg(a))
            l.usage("unexpected argument '" + a + "'");
    }
    l.close();
cout << "closed" << endl;
    //* GUI drawing code */
    myTestWin w;

    //* stores all trace data */
    traceDataMan_cl traceDataMan;

    //* provides all markers */
    markerMan_cl markerMan;
    for (auto t : l.traces) {
        traceDataMan.loadData(t.dataX);
        traceDataMan.loadData(t.dataY);
        const marker_cl *m = markerMan.getMarker(t.marker);
        if (m == NULL)
            l.usage("invalid marker description '" + t.marker + "'. Valid example: g.1");
        w.tb->addTrace(traceDataMan.getData(t.dataX), traceDataMan.getData(t.dataY), m, t.vertLineX, t.horLineY);
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
    w.show();
    Fl::run();
    return 0;
}
int main(int argc, const char **argv) {
    try {
        main2(argc, argv);
    } catch (std::exception &e) {
        cerr << "exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}