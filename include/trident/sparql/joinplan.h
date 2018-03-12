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


#ifndef JOIN_PLAN_H_
#define JOIN_PLAN_H_

#include <trident/sparql/query.h>
#include <trident/sparql/filter.h>

#include <iostream>

using namespace std;

struct JoinPoint {
    int posRow;
    int posIndex;
    int posPattern;
    int index;

    int sourcePattern;
    int sourcePosIndex;

    bool merge;

    int64_t lastValue;

    JoinPoint() {
        lastValue = 0;
    }
};

typedef struct PatternInfo {
    vector<std::pair<Pattern *, uint64_t> >::iterator pos;
    int nvars;
    int njoins;
    int64_t card;
} PatternInfo;

class Querier;

struct SorterByPerm {
    int *perm;
    SorterByPerm(int* perm) : perm(perm) {}

    bool operator()(const std::pair<int, int> p1, const std::pair<int, int> p2) const {
        return perm[p1.second] < perm[p2.second];
    }
};

struct SorterByJoinOrder {
    bool operator()(const JoinPoint* p1, const JoinPoint* p2) {
        return p1->posRow < p2->posRow;
    }
};

class Scan;
class SPARQLOperator;
class NestedJoinPlan {

private:
    static void rearrange(std::vector<std::pair<Pattern *, uint64_t> > &patterns);

    bool canIApplyMergeJoin(Querier *q, std::vector<Pattern *> *patterns, int i,
                            JoinPoint *j, int *idxs);
public:
    Pattern *patterns;
    Filter **filters;

    int nPatterns;

    int **posVarsToCopyInIdx;
    int **posVarsToCopyInPattern;
    int *nPosVarsToCopyInIdx;

    int *posVarsToReturn;
    int nPosVarsToReturn;

    int *sizeRowsPatterns;

    JoinPoint **joins;
    int *nJoins;

    DDLEXPORT NestedJoinPlan() {
        patterns = NULL;
        filters = NULL;
        posVarsToCopyInIdx = NULL;
        nPosVarsToCopyInIdx = NULL;

        posVarsToReturn = NULL;
        sizeRowsPatterns = NULL;

        joins = NULL;
        nJoins = NULL;
    }

	DDLEXPORT NestedJoinPlan(Pattern &p2, Querier *q, std::vector<std::vector<int>> &posToCopy,
                   std::vector<std::pair<int, int>> &joins, std::vector<int> &posToReturn);

    static std::vector<int> reorder(std::vector <Pattern*> patterns,
                                    std::vector<std::shared_ptr<SPARQLOperator>> scans);

    void prepare(Querier *q, std::vector<Pattern *> patterns,
                 std::vector<Filter *>filters, std::vector<string> projections);

    void printInfo();

    ~NestedJoinPlan() {
        if (patterns != NULL) {
            delete[] patterns;
        }
        if (filters != NULL) {
            delete[] filters;
        }

        if (posVarsToCopyInIdx != NULL) {
            for (int i = 0; i < nPatterns; ++i) {
                if (posVarsToCopyInPattern[i] != NULL)
                    delete[] posVarsToCopyInPattern[i];
                if (posVarsToCopyInIdx[i] != NULL)
                    delete[] posVarsToCopyInIdx[i];
            }
            delete[] posVarsToCopyInIdx;
            delete[] posVarsToCopyInPattern;
        }

        if (joins != NULL) {
            for (int i = 0; i < nPatterns; ++i) {
                if (joins[i] != NULL)
                    delete[] joins[i];

            }
            delete[] joins;
        }

        if (nPosVarsToCopyInIdx != NULL) {
            delete[] nPosVarsToCopyInIdx;
        }

        if (posVarsToReturn != NULL) {
            delete[] posVarsToReturn;
        }

        if (sizeRowsPatterns != NULL) {
            delete[] sizeRowsPatterns;
        }

        if (nJoins != NULL) {
            delete[] nJoins;
        }
    }
};

class SPARQLOperator;
class JoinPlan {
private:

public:
    std::vector<std::vector<JoinPoint>> joins;
    std::vector<std::vector<uint8_t>> posVarsToCopy;
    std::vector<uint8_t> posVarsToReturn;

    static std::vector<string> mergeFields(std::vector<string> s1, std::vector<string> s2);

    void prepare(std::vector<std::shared_ptr<SPARQLOperator>> children);

    void prepare(std::vector<std::shared_ptr<SPARQLOperator>> children,
                 std::vector<string> &projections);
};

#endif
