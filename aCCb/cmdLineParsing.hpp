#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>
namespace aCCb {
using std::string;
// ==============================================================================
// command line parser: base class for argument
// ==============================================================================

// exception during command line parsing on incorrect input
class argObjException : public std::exception {
   public:
    argObjException(const string &msg) : msg(msg) {}
    const char *what() const noexcept {
        return msg.c_str();
    }

    // protected:
    const string msg;
};

class argObj {
   public:
    argObj(string token) : token(token), state(""), stack(), closed(false) {}

    // exception during command line parsing on incorrect input
    argObjException aoException(const string &msg) {
        throw argObjException("(" + token + "): " + msg);
    }

    virtual bool acceptArg_stateSet(const string & /*arg*/) {
        throw std::runtime_error("?? " + token + ":state implementation is missing for '" + state + "' state ??");
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
        if (arg == "-end") {
            assert(stack.size() == 0);  // any child has been offered the arg, rejected it and was therefore closed
            closed = true;              // note, our removal from the parent's stack is delayed
            return true;
        }

        return false;
    }
    void close() {
        std::cout << token << " closing" << std::endl;
        if (!closed) {
            if (state != "") throw aoException(state + ": expecting argument");
            closed = true;
            while (stack.size() > 0) {
                std::cout << token << " closing child" << std::endl;
                stack.back()->close();
                std::cout << token << " closed child" << std::endl;
                stack.pop_back();
            }
        }
        std::cout << token << " closed " << std::endl;
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
    // friendly name for messages
    string token;
    // purpose of the next expected argument (if any)
    string state;
    // objects defined earlier on the command line that may still use arguments not understood by the current object
    std::vector<argObj *> stack;
    bool closed;
};
}  // namespace aCCb