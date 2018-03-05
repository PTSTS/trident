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


#ifndef _DIFF_TERMITR_H
#define _DIFF_TERMITR_H

#include <trident/iterators/pairitr.h>
#include <trident/kb/consts.h>
#include <trident/tree/coordinates.h>

#include <memory>

class TreeItr;
class DiffTermItr : public PairItr {
private:
    int perm;
    int64_t nkeys;
    int64_t nuniquekeys;

    uint8_t mode;
    std::unique_ptr<TreeItr> itr;
    TermCoordinates values;

    const char *s;
    const char *e;
    uint8_t nbytes;

    int64_t constantvalue;
    uint8_t remaining;

public:
    int getTypeItr() {
        return DIFFTERM_ITR;
    }

    void clear() {
    }

    int64_t getValue1() {
        return 0;
    }

    int64_t getValue2() {
        return 0;
    }

    void ignoreSecondColumn() {
        throw 10; //not supported
    }

    void mark() {
        throw 10; //not supported
    }

    void reset(const char i) {
        throw 10; //not supported
    }

    void moveto(const int64_t c1, const int64_t c2) {
        throw 10; //not implemented
    }

    void init(int perm, int64_t nkeys, int64_t nuniquekeys, TreeItr *itr);

    void init(int perm, int64_t nkeys, int64_t nuniquekeys, const char *array, const uint8_t nbytes);

    void init(int perm, int64_t nkeys, int64_t nuniquekeys, const int64_t value);

    int64_t getCount();

    void gotoKey(int64_t keyToSearch);

    bool hasNext();

    void next();

    uint64_t getCardinality();

    uint64_t estCardinality();

    uint64_t getNUniqueKeys();
};

#endif
