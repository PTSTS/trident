#ifndef H_cts_infra_QueryGraph
#define H_cts_infra_QueryGraph
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
#include <vector>
#include <string>
#include <set>
#include <memory>
#include <trident/sparql/aggrhandler.h>
//---------------------------------------------------------------------------
/// A query graph representing a SPARQL query
class QueryGraph {
    public:
        /// Possible duplicate handling modes
        enum DuplicateHandling { AllDuplicates, CountDuplicates, ReducedDuplicates, NoDuplicates, ShowDuplicates };

        /// A node in the graph
        struct Node {
            /// The values
            uint64_t subject, predicate, object;
            /// Which of the three values are constants?
            bool constSubject, constPredicate, constObject;

            void *additionalData;

            /// Is there an implicit join edge to another node?
            bool canJoin(const Node& other) const;
        };
        // A special node with a list of values (VALUES in SPARQL 1.1)
        struct ValuesNode {
            std::vector<unsigned> variables;
            std::vector<uint64_t> values;
        };

        /// The potential join edges
        struct Edge {
            /// The endpoints
            uint64_t from, to;
            /// Common variables
            std::vector<unsigned> common;

            /// Constructor
            Edge(uint64_t from, uint64_t to, const std::vector<unsigned>& common);
            /// Destructor
            ~Edge();
        };
        /// A value filter
        struct SubQuery;
        struct Filter {
            /// Possible types
            enum Type {
                Or, And, Equal, NotEqual, Less, LessOrEqual, Greater, GreaterOrEqual, Plus, Minus, Mul, Div,
                Not, UnaryPlus, UnaryMinus, Literal, Variable, IRI, Null, Function, ArgumentList,
                Builtin_str, Builtin_lang, Builtin_langmatches, Builtin_datatype, Builtin_bound, Builtin_sameterm,
                Builtin_isiri, Builtin_isblank, Builtin_isliteral, Builtin_regex, Builtin_replace, Builtin_in,
                Builtin_notin, Builtin_contains,
                Builtin_xsddecimal, Builtin_notexists
            };

            /// The type
            Type type;
            /// Input arguments
            Filter* arg1, *arg2, *arg3, *arg4;
            /// The id (if possible)
            unsigned id;
            /// The raw value (for constants)
            std::string value;
            uint64_t valueid; //id of constants
            std::shared_ptr<QueryGraph> subquery;
            std::shared_ptr<QueryGraph::SubQuery> subpattern;

            /// Constructor
            Filter();
            /// Copy-Constructor
            Filter(const Filter& other);
            /// Destructor
            ~Filter();

            /// Assignment
            Filter& operator=(const Filter& other);

            /// Could be applied?
            bool isApplicable(const std::set<unsigned>& variables) const;

            std::set<std::pair<uint64_t, bool> > allIdVarsAndLiterals() const;
        };
        /// A table function
        struct TableFunction {
            std::shared_ptr<Filter> associatedFilter;
            /// An argument
            struct Argument {
                /// The variable id if any
                unsigned id;
                /// The string value
                std::string value;
                uint64_t idvalue;
            };
            /// The function name
            std::string name;
            /// Input
            std::vector<Argument> input;
            /// Output
            std::vector<unsigned> output;

            TableFunction() : associatedFilter(NULL) {}
        };
        /// Description of a subquery
        struct SubQuery {
            /// The nodes
            std::vector<Node> nodes;
            /// The edges
            std::vector<Edge> edges;
            /// The filter conditions
            std::vector<Filter> filters;
            /// Optional subqueries
            std::vector<SubQuery> optional;
            /// Union subqueries
            std::vector<std::vector<SubQuery> > unions;
            /// The table functions
            std::vector<TableFunction> tableFunctions;
            /// Possible query
            std::vector<std::shared_ptr<QueryGraph>> subqueries;
            /// Possible Minuses
            std::vector<std::shared_ptr<QueryGraph>> minuses;
            /// All VALUES patterns
            std::vector<ValuesNode> valueNodes;
        };
        /// Order by entry
        struct Order {
            /// The variable
            unsigned id;
            /// Descending
            bool descending;
        };
    private:
        /// The query itself
        SubQuery query;
        /// The projection
        std::vector<unsigned> projection;
        /// The duplicate handling
        DuplicateHandling duplicateHandling;
        /// The order by clause
        std::vector<Order> order;
        /// Maximum result size
        unsigned limit;
        /// Is the query known to produce an empty result?
        bool knownEmptyResult;
        /// The table functions (that occur outside the where clause)
        std::vector<TableFunction> assignments;

        //Variables used to create groups
        AggregateHandler hdl;
        std::vector<unsigned> groupby;
        std::vector<Filter> having; //Criteria to filter out some groups

        QueryGraph(const QueryGraph&);
        void operator=(const QueryGraph&);

    public:
        /// Constructor
        QueryGraph(unsigned varcount);
        /// Destructor
        ~QueryGraph();

        /// Clear the graph
        void clear();
        /// Construct the edges
        void constructEdges();

        /// Set the duplicate handling mode
        void setDuplicateHandling(DuplicateHandling d) {
            duplicateHandling = d;
        }
        /// Get the duplicate handling mode
        DuplicateHandling getDuplicateHandling() const {
            return duplicateHandling;
        }
        /// Set the result limit
        void setLimit(unsigned l) {
            limit = l;
        }
        /// Get the result limit
        unsigned getLimit() const {
            return limit;
        }
        /// Known empty result
        void markAsKnownEmpty() {
            knownEmptyResult = true;
        }
        /// Known empty result?
        bool knownEmpty() const {
            return knownEmptyResult;
        }

        std::vector<TableFunction> &getGlobalAssignments() {
            return assignments;
        }

        /// Get the query
        SubQuery& getQuery() {
            return query;
        }
        /// Get the query
        const SubQuery& getQuery() const {
            return query;
        }

        //Add a groupby variable
        void addGroupBy(unsigned id) {
            groupby.push_back(id);
        }

        //Add a having variable
        void addHaving(Filter &f) {
            having.push_back(f);
        }

        /// Add an entry to the output projection
        void addProjection(unsigned id) {
            projection.push_back(id);
        }
        /// Iterator over the projection
        typedef std::vector<unsigned>::const_iterator projection_iterator;
        /// Iterator over the projection
        projection_iterator projectionBegin() const {
            return projection.begin();
        }
        /// Iterator over the projection
        projection_iterator projectionEnd() const {
            return projection.end();
        }

        /// Add an entry to the sort order
        void addOrder(const Order& o) {
            order.push_back(o);
        }
        /// Iterator over the sort order
        typedef std::vector<Order>::const_iterator order_iterator;
        /// Iterator over the sort order
        order_iterator orderBegin() const {
            return order.begin();
        }
        /// Iterator over the sort order
        order_iterator orderEnd() const {
            return order.end();
        }

        unsigned getVarCount() const {
            return hdl.getVarCount();
        }

        AggregateHandler &getAggredateHandler() {
            return hdl;
        }

        const std::vector<Filter> &getHavings() const {
            return having;
        }

        const AggregateHandler &c_getAggredateHandler() const {
            return hdl;
        }

        const std::vector<unsigned> &getGroupBy() const {
            return groupby;
        }
};
//---------------------------------------------------------------------------
#endif
