#include <cassert>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
namespace aCCb {
using std::string_view;
using std::vector;
class csvRawReader {
   public:
    csvRawReader(const std::string& filename, char sep) : reader(filename), sep(sep), tokenStart(0) {}
    vector<std::string_view> getRow() {
        // previous row becomes invalid, discard base buffer and overwrite
        reader.consume(tokenStart);
        size_t posInBuffer = 0;
        tokenStart = 0;

        vector<std::string_view> r;
        string_view buf = reader.getBuffer();
        while (true) {
            if (posInBuffer == buf.size()) {
                // row exceeds buffer capacity or EOF
                if (!reader.extend()) {
                    break;  // EOF
                }
                buf = reader.getBuffer();
            }
            char c = buf[posInBuffer];
            if (c == sep) {
                r.push_back(string_view(&buf[tokenStart], posInBuffer - tokenStart));
                tokenStart = posInBuffer + 1;
            }
            if (c == '\n') {
                if ((posInBuffer > tokenStart) && (buf[posInBuffer - 1] == '\r'))
                    r.push_back(string_view(&buf[tokenStart], posInBuffer - tokenStart - 1));  // skip '\r' before '\n'
                else
                    r.push_back(string_view(&buf[tokenStart], posInBuffer - tokenStart));
                tokenStart = posInBuffer + 1;
                break;
            }
            ++posInBuffer;
        }
        return r;
    }

   protected:
    class bufReader {
       public:
        bufReader(const std::string& filename) : buf(65536), readPos(0), nValid(0), nTrunc(32768) {
            is = std::ifstream(filename, std::ios::binary | std::ios::ate);
            if (!is.is_open()) throw std::runtime_error("failed to open file (r)");
            nAvailable = is.tellg();
            is.seekg(0, std::ios::beg);
            fillBuffer();
        }

        /* Removes nTaken leading chars from getBuffer()'s next output. Invalidates earlier getBuffer() result. */
        void consume(size_t nTaken) {
            assert(nTaken <= nValid);
            readPos += nTaken;
            assert(readPos < buf.size());
            nValid -= nTaken;

            // decide whether to shift the buffer
            if (readPos > nTrunc) {
                if (readPos + nAvailable > buf.size()) {
                    // available data would exceed the buffer end
                    memmove(/*dest*/ &buf[0], /*src*/ &buf[readPos], /*count*/ nValid);
                    readPos = 0;
                }
                // a significant chunk was freed. Fill with data.
                fillBuffer();
            }

            if (nValid == 0)
                extend();
        }

        /** returns currently available data of indefinite length. Use extend() to request more */
        std::string_view getBuffer() const {
            return std::string_view(buf.data() + readPos, nValid);
        }

        //* increases the length of getBuffer()'s result. False if EOF. Repeated calls are allowed. */
        bool extend() {
            if (nAvailable == 0)
                return false;
            if ((readPos == 0) && (nValid == buf.size())) {
                // need more space. Double buffer size
                buf.insert(buf.end(), /*count*/ buf.size(), /*value*/ 0);
            }
            fillBuffer();
            return true;
        }

        void fillBuffer() {
            size_t nFree = buf.size() - readPos - nValid;
            size_t nRead = std::min(nAvailable, nFree);
            is.read(buf.data() + readPos + nValid, nRead);
            nAvailable -= nRead;
            nValid += nRead;
        }

       protected:
        std::ifstream is;
        std::vector<char> buf;
        size_t readPos;
        size_t nValid;
        size_t nAvailable;
        // shift the buffer, if more than nTrunc bytes of old data remain */
        const size_t nTrunc;
    };  // class bufReader

    size_t tokenStart;
    bufReader reader;
    const char sep;
};  // class csvRawReader
class csvRawReaderRef {
   protected:
    std::ifstream is;

   public:
    csvRawReaderRef(const std::string& filename, char sep) : sep(sep) {
        is = std::ifstream(filename, std::ios::binary);
        if (!is.is_open()) throw std::runtime_error("failed to open file (r)");
    }
    vector<std::string> getRow() {
        vector<std::string> r;

        std::string buf;
        if (!std::getline(is, buf))
            return r;
        size_t posInBuffer = 0;
        size_t tokenStart = 0;

        while (true) {
            char c = posInBuffer == buf.size() ? '\n' : buf[posInBuffer];
            if (c == sep) {
                r.push_back(std::string(&buf[tokenStart], posInBuffer - tokenStart));
                tokenStart = posInBuffer + 1;
            }
            if (c == '\n') {
                if ((posInBuffer > tokenStart) && (buf[posInBuffer - 1] == '\r'))
                    r.push_back(std::string(&buf[tokenStart], posInBuffer - tokenStart - 1));  // skip '\r' before '\n'
                else
                    r.push_back(std::string(&buf[tokenStart], posInBuffer - tokenStart));
                tokenStart = posInBuffer + 1;
                break;
            }
            ++posInBuffer;
        }
        return r;
    }

   protected:
    const char sep;
};  // class csvRawReader
}  // namespace aCCb