#ifndef H_cts_plangen_Plan
#define H_cts_plangen_Plan
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
#include <cts/infra/QueryGraph.hpp>
#include <infra/util/Pool.hpp>
//---------------------------------------------------------------------------
/// A plan fragment
struct Plan
{
    /// Possible operators
    enum Op { IndexScan, AggregatedIndexScan, FullyAggregatedIndexScan,
        NestedLoopJoin, MergeJoin, HashJoin, HashGroupify, Filter, Union,
        MergeUnion, TableFunction, Singleton, Subselect, Minus, ValuesScan,
        CartProd };
    /// The cardinalits type
    typedef double card_t;
    /// The cost type
    typedef double cost_t;

    /// The root operator
    Op op;
    /// Operator argument
    unsigned opArg;
    /// Its input
    Plan* left,*right;
    /// The resulting cardinality
    card_t cardinality;
    /// The total costs
    cost_t costs;
    /// The ordering
    unsigned ordering;
    // is optional
    bool optional;
    /// QueryGraph associated (used for minus)
    std::shared_ptr<QueryGraph> subquery;

    /// The next plan in problem chaining
    Plan* next;

    /// Print the plan
    void print(unsigned indent) const;

    void init() {
        optional = false;
        subquery = NULL;
    }

    Plan() {
        init();
    }
};

struct FilterArgs {
    const QueryGraph::Filter *filter;
    Plan *plan;
};

//---------------------------------------------------------------------------
/// A container for plans. Encapsulates the memory management
class PlanContainer
{
    private:
        /// The pool
        StructPool<Plan> pool;
        StructPool<FilterArgs> poolFilterArgs;

    public:
        /// Constructor
        PlanContainer();
        /// Destructor
        ~PlanContainer();

        /// Alloca a new plan
        Plan* alloc() { Plan *plan = pool.alloc(); plan->init(); return plan; }
        FilterArgs* allocFilterArgs() { FilterArgs *ag = poolFilterArgs.alloc(); return ag; }
        /// Release an allocate plan
        void free(Plan* p);
        void freeFilterArgs(FilterArgs* p);
        /// Release all plans
        void clear();
};
//---------------------------------------------------------------------------
#endif
