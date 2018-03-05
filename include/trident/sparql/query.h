/*
 * Copyright 2017 Jacopo Urbani
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
**/


#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include <trident/sparql/filter.h>
#include <trident/kb/dictmgmt.h>
//#include <cts/infra/QueryGraph.hpp>
//#include <cts/parser/SPARQLParser.hpp>

#include <vector>
#include <cstring>

#define PARSE_OK 0
#define PARSE_ERROR_SYNTAX 1
#define PARSE_URI_NOT_FOUND 2

class KB;

using namespace std;

class Pattern {
private:
    int64_t s, p, o;
    int idxToUse, n_vars;
    std::vector<std::string> vars;
    std::vector<int> pos_vars;
public:
    Pattern() {
        idxToUse = n_vars = 0;
        s = p = o = -1;
    }

    Pattern(const Pattern &pattern) {
        vars = pattern.vars;
        pos_vars = pattern.pos_vars;
        s = pattern.s;
        p = pattern.p;
        o = pattern.o;
        idxToUse = pattern.idxToUse;
        n_vars = pattern.n_vars;
    }

    string toString() {
        string line = "";
        int nvar = 0;
        if (pos_vars.size() > 0 && pos_vars[nvar] == 0) {
            line += vars[nvar++];
        } else {
            line += to_string(s);
        }

        if (pos_vars.size() > nvar && pos_vars[nvar] == 1) {
            line += " " + vars[nvar++];
        } else {
            line += " " + to_string(p);
        }

        if (pos_vars.size() > nvar && pos_vars[nvar] == 2) {
            line += " " + vars[nvar++];
        } else {
            line += " " + to_string(o);
        }
        return line;
    }

    void subject(int64_t s) {
        this->s = s;
    }

    int64_t subject() {
        return s;
    }

    void predicate(int64_t p) {
        this->p = p;
    }

    int64_t predicate() {
        return p;
    }

    void object(int64_t o) {
        this->o = o;
    }

    int64_t object() {
        return o;
    }

    void idx(int idx) {
        this->idxToUse = idx;
    }

    int idx() {
        return idxToUse;
    }

    void addVar(int pos, std::string name) {
        vars.push_back(name);
        pos_vars.push_back(pos);
        n_vars++;
    }

    int posVar(int idx) {
        return pos_vars[idx];
    }

    int posVar(std::string var) {
        for (int i = 0; i < n_vars; ++i) {
            if (vars[i] == var) {
                return pos_vars[i];
            }
        }
        return -1;
    }

    std::vector<int> *getPosVars() {
        return &pos_vars;
    }

    std::string getVar(int idx) {
        return vars[idx];
    }

    int getNVars() {
        return n_vars;
    }

    vector<int> getRepeatedVars() {
        vector<int> results;
        if (n_vars < 2) {
            return results;
        }
        bool first = true;
        for (int i = 0; i < n_vars - 1; ++i) {
            std::string var = vars[i];
            for (int j = i + 1; j < n_vars; ++j) {
                string var2 = vars[j];
                if (var.compare(var2) == 0) {
                    if (first) {
                        results.push_back(i);
                    }
                    results.push_back(j);
                }
            }
        }
        return results;
    }

    void addVarsTo(vector<std::string> &v) {
        for (std::vector<std::string>::iterator itr = vars.begin(); itr != vars.end(); ++itr) {
            bool found = false;
            for (std::vector<std::string>::iterator itr2 = v.begin(); itr2 != v.end(); ++itr2) {
                if (itr->compare(*itr2) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                v.push_back(*itr);
            }
        }
    }

    int joinsWith(vector<string> &v) {
        int njoins = 0;
        for (vector<string>::iterator itr = vars.begin(); itr != vars.end(); ++itr) {
            for (std::vector<std::string>::iterator itr2 = v.begin(); itr2 != v.end(); ++itr2) {
                if (itr->compare(*itr2) == 0) {
                    njoins++;
                    break;
                }
            }
        }
        return njoins;
    }

    int joinsWith(Pattern *p) {
        int njoins = 0;
        for (int i = 0; i < n_vars; ++i) {
            for (int j = 0; j < p->n_vars; ++j) {
                if (vars[i].compare(p->vars[j]) == 0) {
                    njoins++;
                    break;
                }
            }
        }
        return njoins;
    }

    bool containsVar(const string &v) const {
        for (int i = 0; i < n_vars; ++i) {
            if (vars[i] == v) {
                return true;
            }
        }
        return false;
    }

};

class Query {
private:
    std::vector<Pattern*> patterns;
    std::vector<Filter*> filters;
    std::vector<std::string> projections;

public:
    //void parseFromRDF3XQueryGraph(SPARQLParser &parser, QueryGraph &graph);
    Query(std::vector<Pattern*> &patterns,
          std::vector<Filter*> &filters,
          std::vector<std::string> &projections) :
        patterns(patterns),
        filters(filters),
        projections(projections) {
    }

    int npatterns() {
        return patterns.size();
    }

    std::vector<Pattern*> getPatterns() {
        return patterns;
    }

    std::vector<Filter*> getFilters() {
        return filters;
    }

    std::vector<std::string> getProjections() {
        return projections;
    }

    ~Query() {
        int size = patterns.size();
        for (int i = 0; i < size; ++i) {
            delete patterns[i];
            if (filters[i] != NULL) {
                delete filters[i];
            }
        }
        patterns.clear();
        projections.clear();
    }
};

#endif
