#pragma once
#include <istream>
#include <string>
#include <vector>
#include <stdexcept>
#include "../aCCb/logicalIndexing.hpp"
namespace li = aCCb::logicalIndexing;
using std::string;
using std::vector;

class tableFactory {
public:
	static aCCb::logicalIndexing::vecMap csvTable(std::istream &is, char sep = ',') {
		// sequential list of parsed fields
		vector<string> fields;
		// translate sep (argument) to SEP (constant token)
		const int SEP = EOF - 1;
		// check consistent column count in all rows
		size_t commonNCols = 0;
		// whether common column count has already been determined
		bool commonNColsSet = false;
		// columns in current row
		size_t nColsRow = 0;
		string tmp;

		// === 'quoted' state ===
		// - entered by double quotes as first character in field
		// - exited by double quote then field terminator
		// - in quoted state, two consecutive double quotes are a literal double quote
		//   - in this case, the first double quote transitions QUOTE_OPEN => QUOTE_CLOSED_UNTERMINATED
		//   - the second double quote transitions QUOTE_CLOSED_UNTERMINATED => QUOTE_OPEN and inserts the literal double quote
		enum quoteState_e {
			QUOTE_OFF, QUOTE_OPEN, QUOTE_CLOSED_UNTERMINATED
		};
		quoteState_e quoteState = QUOTE_OFF;

		// === main loop: ===
		// Iterate over input characters
		bool running = true;
		while (running) {
			int cInt = is.get();

			switch (quoteState) {
			case QUOTE_OFF:
			case QUOTE_CLOSED_UNTERMINATED:

				// === EOF handling ===
				if (cInt == EOF) {
					// break outer loop
					running = false;
					if ((nColsRow == 0) && (tmp.size() == 0)) {
						// EOF on empty row: break outer loop immediately
						break;
					} else {
						// EOF on non-empty row: handle as newline, then break outer loop
						cInt = '\n';
					}
				} else if (cInt == sep) {
					cInt = SEP;
				}

				switch (cInt) {
				case '"':
					if (quoteState == QUOTE_OFF) {
						// opens quoted string
						quoteState = QUOTE_OPEN;
					} else /*if (quoteState == QUOTE_CLOSED_UNTERMINATED) */{
						// two consecutive '"' insert a literal '"'
						tmp.push_back(cInt);
						quoteState = QUOTE_OPEN;
					}
					break;
				case '\r':
					// suppress windows \r from \r\n
					break;
				case SEP: // end-of-field
					fields.push_back(tmp);
					tmp = "";
					++nColsRow;
					quoteState = QUOTE_OFF;
					break;
				case '\n': // end-of-line
					fields.push_back(tmp);
					quoteState = QUOTE_OFF;
					tmp = "";

					if (!commonNColsSet) {
						commonNCols = nColsRow; // first row determines common nFields
						commonNColsSet = true;
					} else {
						if (nColsRow != commonNCols)
							throw std::runtime_error("inconsistent number of fields");
					}
					nColsRow = 0;
					break;
				default:
					// any data
					if (quoteState == QUOTE_CLOSED_UNTERMINATED)
						throw std::runtime_error("unexpected data after double quote in quoted string");
					tmp.push_back(cInt);
				}
				break; // quoteState == QUOTE_OFF or QUOTE_CLOSED_UNTERMINATED
			case QUOTE_OPEN:
				switch (cInt) {
				case EOF:
					throw std::runtime_error("EOF in quoted string");
				case '"':
					quoteState = QUOTE_CLOSED_UNTERMINATED;
					break;
				default:
					// anything except quote character is literal data
					tmp.push_back(cInt);
				} // switch (cIn)
				break; // quoteState == QUOTE_OPEN
			} // switch quoteState
			  // switch (quoted)
		} // while not EOF

		for (auto x : fields){
			std::cout << "'" << x << "'\n" << std::flush;
		}

		vecMapLoader self;
		return self;
	}
protected:
	class vecMapLoader: public li::vecMap {
	};
};
