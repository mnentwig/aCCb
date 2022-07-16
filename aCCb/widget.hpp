#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
class aCCbWidget : public Fl_Box {
   public:
    enum curMove_e {
        /** don't move the cursor */
        NOP,
        /**advance x after drawing*/
        DX,
        /**advance y after drawing*/
        DY,
        /** return to original x and advance x after drawing*/
        X0,
        /** advance y for label height*/
        DYL
    };

    aCCbWidget(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
        if (parent())
            this->color(parent()->color());
    }

    class cursor {
       public:
        cursor() : cursor(0, 0, 20, 20) {}
        cursor(int x, int y, int w, int h) : x(x), y(y), w(w), h(h), x0(x) {}
        int x, y, w, h, x0;
        void set(int x, int y) {
            this->x = x;
            this->x0 = x;
            this->y = y;
        }
        void moveXY(int dx, int dy) {
            this->x += dx;
            this->y += dy;
        }
        void nextRow() {
            this->x = x0;
            this->y += this->h;
        }
        void move(curMove_e mv, int dx, int dy) {
            if (mv == NOP)
                moveXY(dx, dy);
            else
                move(mv);
        }
        void move(curMove_e m) {
            switch (m) {
                case NOP:
                    break;
                case DX:
                    this->moveXY(w, 0);
                    break;
                case DY:
                    this->moveXY(0, h);
                    break;
                case X0:
                    this->nextRow();
                    break;
                case DYL:
                    this->moveXY(0, /*label height*/ 15);
                    break;
            }
        }
    };

   protected:
    Fl_Color colorLabel = FL_GREEN;
    virtual void callbackCheckbutton(Fl_Check_Button *w) {}
    virtual void callbackButton(Fl_Button *w) {}
    virtual void callbackInput(Fl_Input *w) {}

    cursor cur;

    //* Helper class: Generates a FLTK widget by casting to the respective type */
    class widgetGenOnCast {
       public:
        /** creates widget of return type, advancing the cursor as specified by mv */
        widgetGenOnCast(aCCbWidget *parent, const char *label, curMove_e mv) : parent(parent), label(label), mv(mv), dx(0), dy(0) {}
        /** creates widget of return type, advancing the cursor by dx, dy */
        widgetGenOnCast(aCCbWidget *parent, const char *label, int dx, int dy) : parent(parent), label(label), mv(NOP), dx(dx), dy(dy) {}

        operator Fl_Input *() {
            cursor &c = parent->cur;
            Fl_Input *r = new Fl_Input(c.x, c.y, c.w, c.h, label);
            r->labelcolor(parent->labelcolor());
            r->align(FL_ALIGN_TOP_LEFT);
            c.move(mv, dx, dy);
            return r;
        }

        operator Fl_Check_Button *() {
            cursor &c = parent->cur;
            Fl_Check_Button *r = new Fl_Check_Button(c.x, c.y, /*height as width*/ c.h, c.h, label);
            r->align(FL_ALIGN_RIGHT);
            r->callback((Fl_Callback *)cbHandlerCb, (void *)parent);
            r->labelcolor(parent->labelcolor());
            c.move(mv, dx, dy);
            return r;
        }

        operator Fl_Button *() {
            cursor &c = parent->cur;
            Fl_Button *r = new Fl_Button(c.x, c.y, c.w, c.h, label);
            r->align(FL_ALIGN_INSIDE);
            r->callback((Fl_Callback *)cbHandlerB, (void *)parent);
            r->labelcolor(parent->labelcolor());
            r->color(parent->color());
            c.move(mv, dx, dy);
            return r;
        }

       protected:
        aCCbWidget *parent;
        const char *label;
        curMove_e mv;
        int dx;
        int dy;
    };

    /** New widget of return type, advancing the cursor as specified by mv.  Note: Delayed creation on cast */
    widgetGenOnCast create(const char *label, curMove_e mv) {
        return widgetGenOnCast(this, label, mv);
    }
    /** New widget of return type, advancing the cursor by dx, dy. Note: Delayed creation on cast */
    widgetGenOnCast create(const char *label, int dx, int dy) {
        return widgetGenOnCast(this, label, dx, dy);
    }

    /** Invokes the user-defined, widget-specific callback function */
    static void cbHandlerB(Fl_Widget *w, void *p) {
        ((aCCbWidget *)p)->callbackButton((Fl_Button *)w);
    }

    /** Invokes the user-defined, widget-specific callback function */
    static void cbHandlerCb(Fl_Widget *w, void *p) {
        ((aCCbWidget *)p)->callbackCheckbutton((Fl_Check_Button *)w);
    }

    /** Invokes the user-defined, widget-specific callback function */
    static void cbHandlerInp(Fl_Widget *w, void *p) {
        ((aCCbWidget *)p)->callbackInput((Fl_Input *)w);
    }
};
