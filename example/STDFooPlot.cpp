#include <../aCppCookbook/aCCb/binIo.hpp>
#include <../aCppCookbook/aCCb/plot2d.hpp>
#include <../aCppCookbook/aCCb/vectorText.hpp>
#include <../aCppCookbook/aCCb/widget.hpp>

//#include <../aCppCookbook/aCCb/stringToNum.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using std::string, std::vector, std::array, std::cout, std::endl, std::runtime_error, std::unordered_map;

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
        Fl_Button *b = create("Click me!", DY);
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
        this->data = aCCb::binaryIo::file2vec<float>("out2.float");
        const char *marker =
            "X   X"
            " X X "
            "  X  "
            " X X "
            "X   X";
        const char *marker2 = "X";
        const char *marker3 =
            "X X"
            " X "
            "X X";
        tb->setData(this->data, marker3);
    }
    void show() {
        this->window->show();
    }
    ~myTestWin() {
        delete this->window;  // deletes children recursively
    }

   protected:
    aCCbWidget *menu;
    Fl_Double_Window *window;
    aCCb::plot2d *tb;
    vector<float> data;
};

// ==============================================================================
// command line parser: base class for argument
// ==============================================================================
class argObj {
   public:
    argObj(string token) : token(token), state(""), stack(), closed(false) {}

    virtual bool acceptArg_stateSet(const string &arg) {
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
        if (state != "") throw runtime_error(token + " expecting argument for '" + state + "'");
        closed = true;
        for (auto it = stack.begin(); it != stack.end(); ++it)
            (*it)->close();
        stack.clear();
    }

    bool acceptArg(const string &arg) {
        if (closed)
            return false;
        if (state == "")
            return acceptArg_stateUnset(arg);
        else
            return acceptArg_stateSet(arg);
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
            dataX = a;
        else if (state == "-dataY")
            dataY = a;
        else if (state == "-mask")
            mask = a;
        else if (state == "-marker")
            marker = a;
        else
            throw runtime_error("?? state implementation missing: " + state + " ??");
        state = "";
        return true;
    }

   protected:
    const vector<string> stateArgs{"-dataX", "-dataY", "-mask", "-marker"};
    const vector<string> switchArgs;

    string dataX;
    string dataY;
    string mask;
    string marker;
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
        } else
            return argObj::acceptArg_stateSet(a);
        state = "";
        return true;
    }

   protected:
    const vector<string> stateArgs{"-title", "-xlabel", "-ylabel", "-xlines"};
    const vector<string> switchArgs{"-trace", "-debug"};
    string title;
    string xlabel;
    string ylabel;
    vector<trace> traces;
    vector<float> xlines;
    vector<float> ylines;
};

int main(int argc, const char **argv) {
    loader l;
    for (int ixArg = 1; ixArg < argc; ++ixArg) {
        string a = argv[ixArg];
        if (!l.acceptArg(a))
            throw runtime_error("unexpected argument '" + a + "'");
    }
    l.close();
    exit(0);
    Fl::visual(FL_RGB);
    myTestWin w;
    w.show();
    return Fl::run();
}