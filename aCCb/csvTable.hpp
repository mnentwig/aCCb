#pragma once
#include <istream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include "../aCCb/stringToNum.hpp"
#include "../aCCb/logicalIndexing.hpp"
#include "../aCCb/containerUtils.hpp"
namespace li = aCCb::logicalIndexing;
using std::string;
using std::vector;
using std::runtime_error;
class tableFactory {
protected:
	class vecMapCsvLoader;
public:
	class importSpec {
		friend vecMapCsvLoader;
	public:
		importSpec() :
				colType(), colName(), inputHandler() {
		}
		typedef li::vecMap::colType_e colType_e;
		char sep = ',';
		// enum autoColName_e {OFF, BASE0, BASE1, EXCEL};
		colType_e colTypeDefault = colType_e::UNDEF;
		const colType_e colType_UNDEF = colType_e::UNDEF;
		const colType_e colType_FLOAT = colType_e::FLOAT;
		const colType_e colType_DOUBLE = colType_e::DOUBLE;
		const colType_e colType_BOOL = colType_e::BOOL;
		const colType_e colType_STRING = colType_e::STRING;
		const colType_e colType_UINT8 = colType_e::UINT8;
		const colType_e colType_INT8 = colType_e::INT8;
		const colType_e colType_UINT16 = colType_e::UINT16;
		const colType_e colType_INT16 = colType_e::INT16;
		const colType_e colType_UINT32 = colType_e::UINT32;
		const colType_e colType_INT32 = colType_e::INT32;
		const colType_e colType_UINT64 = colType_e::UINT64;
		const colType_e colType_INT64 = colType_e::INT64;
		void registerColumn(size_t posBase0, string name, colType_e colType) {
			// === error checking ===
			if (name == string())
				throw runtime_error("name may not be empty");
			if ((this->colName.size() > posBase0) && (this->colName[posBase0] != string()))
				throw runtime_error("column position already registered");
			if (aCCb::containerUtils::contains(this->colName, name))
				throw runtime_error("column name already registered");

			// === create new entry ===
			aCCb::containerUtils::putAt(this->colName, name, posBase0);
			this->colType[name] = colType;
		}
	protected:
		std::unordered_map<string, colType_e> colType;
		vector<string> colName;
		vector<void (*)(const string&)> inputHandler;
		template<typename T> static void put(vector<T> vec, size_t index, T elem) {
			while (vec.size() <= index)
				vec.resize(index + 1);
			vec[index] = elem;
		}
	}; // class importSpec

	static void csvTable(std::istream &is, importSpec &spec, li::vecMap& result) {
		vector<string> fields;
		size_t nCols;
		csvTable(is, spec, /*out*/fields, /*out*/nCols);
		result = vecMapCsvLoader(fields, nCols, spec);
	}
protected:
	static void csvTable(std::istream &is, importSpec &spec, /*out*/vector<string> &fields, /*out*/size_t &nColumns) {
		// translate sep (argument) to SEP (constant token)
		const int sep = spec.sep;
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

				if (cInt == EOF) {
					// === EOF handling ===
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
					// === field separator detection ===
					cInt = SEP;					// constant for "case SEP:" below
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
				case SEP:					// end-of-field
					fields.push_back(tmp);
					tmp = "";
					++nColsRow;
					quoteState = QUOTE_OFF;
					break;
				case '\n':					// end-of-line
					fields.push_back(tmp);
					++nColsRow;
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

		for (auto x : fields) {
			std::cout << "'" << x << "'\n" << std::flush;
		}
		nColumns = commonNCols;
	}

	// ===================================================================
	// vecMapCsvLoader
	// ===================================================================
	/** helper class to load a vecMap from a CSV source */
	class vecMapCsvLoader: public li::vecMap {
	protected:
		class importJob;
		typedef void (*importFun_t)(importJob&, const string&);

		class importJob {
		public:
			importJob() :
					handlerFun(NULL), ixCol(0), colType(UNDEF), dataVec(NULL) {
			}
			importFun_t handlerFun;
			size_t ixCol;
			colType_e colType;
			void *dataVec;
		};

		template<typename T> static void handler_num(importJob &j, const string &data) {
			T val;
			if (!aCCb::str2num(data, val))
				throw runtime_error("conversion failed");
			((vector<T>*) j.dataVec)->push_back(val);
		}

		static void handler_string(importJob &j, const string &data) {
			((vector<string>*) j.dataVec)->push_back(data);
		}

		const colType_e colType_UNDEF = colType_e::UNDEF;
		const colType_e colType_FLOAT = colType_e::FLOAT;
		const colType_e colType_DOUBLE = colType_e::DOUBLE;
		const colType_e colType_BOOL = colType_e::BOOL;
		const colType_e colType_STRING = colType_e::STRING;
		const colType_e colType_UINT8 = colType_e::UINT8;
		const colType_e colType_INT8 = colType_e::INT8;
		const colType_e colType_UINT16 = colType_e::UINT16;
		const colType_e colType_INT16 = colType_e::INT16;
		const colType_e colType_UINT32 = colType_e::UINT32;
		const colType_e colType_INT32 = colType_e::INT32;
		const colType_e colType_UINT64 = colType_e::UINT64;
		const colType_e colType_INT64 = colType_e::INT64;

	public:
		vecMapCsvLoader(vector<string> fields, size_t nCols, importSpec &spec) {

			// === set up parser function per column ===
			vector<importJob> importJobsByCol;

			for (size_t ixCol = 0; ixCol < nCols; ++ixCol) {
				string &colName = spec.colName[ixCol];

				// === skip unused columns ===
				if (colName == "")
					continue;
				colType_e colType = spec.colType[colName];

				// === dispatch field type ===
				// - determine the vector, where data goes
				// - select the correct conversion function to put it there
				importJob job;
				// note: The [] map operator on a non-existing element inserts the default element
				switch (colType) {
				case colType_e::UNDEF:
					continue;

				case colType_e::FLOAT: {
					job.dataVec = (void*) &(this->data_float[colName]);
					job.handlerFun = &handler_num<float>;
					break;
				}
				case colType_e::STRING: {
					job.dataVec = (void*) &(this->data_string[colName]);
					job.handlerFun = &handler_string;
					break;
				}
				case colType_e::DOUBLE: {
					job.dataVec = (void*) &(this->data_double[colName]);
					job.handlerFun = &handler_num<double>;
					break;
				}
				case colType_e::BOOL: {
					job.dataVec = (void*) &(this->data_double[colName]);
					job.handlerFun = &handler_num<bool>;
					break;
				}
				case colType_e::UINT8: {
					job.dataVec = (void*) &(this->data_uint8[colName]);
					job.handlerFun = &handler_num<uint8_t>;
					break;
				}
				case colType_e::INT8: {
					job.dataVec = (void*) &(this->data_int8[colName]);
					job.handlerFun = &handler_num<int8_t>;
					break;
				}
				case colType_e::UINT16: {
					job.dataVec = (void*) &(this->data_uint16[colName]);
					job.handlerFun = &handler_num<uint16_t>;
					break;
				}
				case colType_e::INT16: {
					job.dataVec = (void*) &(this->data_int16[colName]);
					job.handlerFun = &handler_num<int16_t>;
					break;
				}
				case colType_e::UINT32: {
					job.dataVec = (void*) &(this->data_uint32[colName]);
					job.handlerFun = &handler_num<uint32_t>;
					break;
				}
				case colType_e::INT32: {
					job.dataVec = (void*) &(this->data_int32[colName]);
					job.handlerFun = &handler_num<int32_t>;
					break;
				}
				case colType_e::UINT64: {
					job.dataVec = (void*) &(this->data_uint64[colName]);
					job.handlerFun = &handler_num<uint64_t>;
					break;
				}
				case colType_e::INT64: {
					job.dataVec = (void*) &(this->data_int64[colName]);
					job.handlerFun = &handler_num<int64_t>;
					break;
				}
				default:
					throw std::runtime_error("?unsupported colType?");
				}

				job.colType = colType;
				aCCb::containerUtils::putAtVal(importJobsByCol, job, ixCol);
			} // for columns

			// === parse all fields ===
			auto itField = fields.cbegin();
			auto itEnd = fields.cend();
			size_t ixCol = 0;
			while (itField != itEnd) {
				if (ixCol < importJobsByCol.size())
					importOneField(importJobsByCol[ixCol], *itField);

				++ixCol;
				if (ixCol == nCols)
					ixCol = 0;
				++itField;
			}
			if (ixCol != 0)
				throw std::runtime_error("?excess field?");
		}

		static void importOneField(importJob &j, const string &token) {
			if (j.handlerFun == NULL)
				return;
			j.handlerFun(j, token);
		}
	};
	// class vecMapLoader
};
// class tableFactory
