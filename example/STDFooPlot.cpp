#include <../aCppCookbook/aCCb/binIo.hpp>
#include <../aCppCookbook/aCCb/plot2d.hpp>
#include <../aCppCookbook/aCCb/vectorText.hpp>
#include <../aCppCookbook/aCCb/widget.hpp>

//#include <../aCppCookbook/aCCb/stringToNum.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using std::string, std::vector, std::array, std::cout, std::endl;

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

int main(int argc, char **argv) {
    Fl::visual(FL_RGB);
    myTestWin w;
    w.show();
    return Fl::run();
}