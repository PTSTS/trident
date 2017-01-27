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


#ifndef TUPLE_TABLE_H
#define TUPLE_TABLE_H

#include <trident/iterators/tupleiterators.h>

#include <boost/log/trivial.hpp>

#include <vector>
#include <iostream>

class TupleTable {
private:
    const size_t sizeRow;
    const std::vector<int> signature;
    std::vector<uint64_t> values;

    struct Sorter {
        const uint8_t nfields;
        uint8_t fields[8];

        Sorter(std::vector<uint8_t> &f) : nfields((uint8_t) f.size()) {
            if (f.size() > 8) {
                BOOST_LOG_TRIVIAL(error) << "Sorting works on at most 8 fields";
                throw 10;
            }

            for (int i = 0; i < nfields; ++i) {
                fields[i] = f[i];
            }
        }

        Sorter(const uint8_t n) : nfields(n) {
            for (uint8_t i = 0; i < n; ++i)
                fields[i] = i;
        }

        bool operator ()(const uint64_t *f1, const uint64_t *f2) const {
            for (uint8_t i = 0; i < nfields; ++i) {
                if (f1[fields[i]] != f2[fields[i]])
                    return f1[fields[i]] < f2[fields[i]];
            }
            return false;
        }
    };

    void readArray(uint64_t *dest, std::vector<uint64_t>::iterator &itr);

    void copyArray(std::vector<uint64_t> &dest, uint64_t *row);

    int cmp(const uint64_t *r1, const uint64_t *r2, const size_t s);

    int cmp(const uint64_t *r1, const uint64_t *r2, const uint8_t *p1,
            const uint8_t *p2, const uint8_t npos);

public:

    struct JoinHitStats {
        //double selectivity1;
        //double selectivity2;

        double ratio;
        long size;
    };

    TupleTable(std::vector<int> sig) : sizeRow(sig.size()), signature(sig) {
    }

    TupleTable(int fields, std::vector<int> sig) : sizeRow(fields), signature(sig) {
    }

    TupleTable(int fields) : sizeRow(fields), signature() {
    }

    size_t getSizeRow() const {
        return sizeRow;
    }

    size_t getNRows() const {
        return values.size() / sizeRow;
    }

    void addRow(const uint64_t *row) {
        for (size_t i = 0; i < sizeRow; ++i) {
            values.push_back(row[i]);
        }
    }

    void addValue(const uint64_t v) {
        values.push_back(v);
    }

    void addAll(TupleTable *t) {
        assert(t != NULL && t->sizeRow == sizeRow);
        copy(t->values.begin(), t->values.end(), back_inserter(values));
    }

    void addRow(const uint64_t *row, const size_t srow) {
        for (size_t i = 0; i < srow; ++i) {
            values.push_back(row[i]);
        }
    }

    void addRow(const uint64_t *row, const int sizeProjection, int* projections) {
        for (int i = 0; i < sizeProjection; ++i) {
            values.push_back(row[projections[i]]);
        }
    }

    TupleTable *sortBy(std::vector<uint8_t> fields);

    TupleTable *sortByAll();

    TupleTable *retain(TupleTable *t);

    TupleTable *merge(TupleTable *t);

    const uint64_t *getRow(const size_t idx) const {
        return &(values.at(idx * sizeRow));
    }

    TupleTable *join(TupleTable *o);

    JoinHitStats joinHitRates(TupleTable *o);

    std::vector<std::pair<uint8_t, uint8_t>> getPosJoins(TupleTable *o);

    std::pair<std::shared_ptr<TupleTable>, std::shared_ptr<TupleTable>> getDenseSparseForBifocalSampling(TupleTable *t2);

    uint64_t getPosAtRow(const uint32_t row, const uint32_t column) const {
        return values.at(row * sizeRow + column);
    }

    void clear() {
        values.clear();
    }
};

class TupleTableItr : public TupleIterator {
    std::shared_ptr<TupleTable> table;
    long counter;
    bool skipLast;
    bool hnc, hn;
    long nextpos;

private:
    bool same(const long l1, const long l2) const;

public:
    TupleTableItr(std::shared_ptr<TupleTable> table) : table(table),
        counter(-1), skipLast(false), hnc(false), hn(false),
        nextpos(0) {}

    long count();

    void skipLastColumn();

    bool hasNext();

    void next();

    size_t getTupleSize();

    uint64_t getElementAt(const int pos);

    ~TupleTableItr() {}
};

#endif
