#ifndef H_rts_operator_ResultsPrinter
#define H_rts_operator_ResultsPrinter
//---------------------------------------------------------------------------
// RDF-3X
// (c) 2008 Thomas Neumann. Web site: http://www.mpi-inf.mpg.de/~neumann/rdf3x
//
// This work is licensed under the Creative Commons
// Attribution-Noncommercial-Share Alike 3.0 Unported License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
// or send a letter to Creative Commons, 171 Second Street, Suite 300,
// San Francisco, California, 94105, USA.
//---------------------------------------------------------------------------
#include "rts/operator/Operator.hpp"
#include <dblayer.hpp>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
//---------------------------------------------------------------------------
class DictionarySegment;
class Register;
class Runtime;
//---------------------------------------------------------------------------
/// A wrapper to avoid duplicating the strings in memory
namespace {
struct CacheEntry {
    /// The string boundaries
    const char* start, *stop;
    /// The type
    Type::ID type;
    /// The sub-type
    unsigned subType;

    /// Constructor
    CacheEntry() : start(0), stop(0) {}
    /// Print the raw value
    void printValue(std::ostream &out, bool escape) const;
    /// Print it
    void print(std::ostream &out, const std::map<uint64_t, CacheEntry>& stringCache, bool escape) const;
    void print(const std::map<uint64_t, CacheEntry>& stringCache, bool escape) const;
    //To string
    std::string tostring(const std::map<uint64_t, CacheEntry>& stringCache, bool escape) const;
};
}

/// Consumes its input and prints it. Produces a single empty tuple.
class ResultsPrinter : public Operator {
public:
    /// Duplicate handling
    enum DuplicateHandling { ReduceDuplicates, ExpandDuplicates, CountDuplicates, ShowDuplicates };
    /// Output modes
    enum OutputMode { DefaultOutput, Embedded };

private:
    /// The output registers
    std::vector<Register*> output;
    /// The input
    Operator* input;
    /// The runtime
    Runtime& runtime;
    /// The dictionary
    DBLayer& dictionary;
    /// The duplicate handling
    DuplicateHandling duplicateHandling;
    /// The output mode
    OutputMode outputMode;
    /// Maximum number of output tuples
    uint64_t limit;
    /// Skip the printing, resolve only?
    bool silent;
    /// Count the printed rows
    uint64_t nrows;
    //Produce JSON output
    boost::property_tree::ptree *jsonoutput;

    void formatJSON(const int ncolumns,
                std::vector<uint64_t> &results,
                std::map<uint64_t, CacheEntry> &stringCache,
                ResultsPrinter::DuplicateHandling duplicateHandling,
                boost::property_tree::ptree *output);

public:
    /// Constructor
    ResultsPrinter(Runtime& runtime, Operator* input, const std::vector<Register*>& output, DuplicateHandling duplicateHandling, uint64_t limit = ~0lu, bool silent = false);
    /// Destructor
    ~ResultsPrinter();

    /// Set the output mode
    void setOutputMode(OutputMode o) {
        outputMode = o;
    }
    /// Get the input
    Operator* getInput() const {
        return input;
    }

    void setSilent(bool flag) {
        silent = flag;
    }

    void setJSONOutput(boost::property_tree::ptree *jsonoutput) {
        this->jsonoutput = jsonoutput;
    }

    /// Produce the first tuple
    uint64_t first();
    /// Produce the next tuple
    uint64_t next();

    uint64_t getPrintedRows() {
        return nrows;
    }

    /// Print the operator tree. Debugging only.
    void print(PlanPrinter& out);
    /// Add a merge join hint
    void addMergeHint(Register* reg1, Register* reg2);
    /// Register parts of the tree that can be executed asynchronous
    void getAsyncInputCandidates(Scheduler& scheduler);
};
//---------------------------------------------------------------------------
#endif
