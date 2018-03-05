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


#include <trident/iterators/arrayitr.h>
#include <assert.h>

#include <iostream>

using namespace std;

bool ArrayItr::hasNext() {

    if (hasNextChecked) {
        return n;
    }

    if (ignSecondColumn) {
        //Is there a new first term?
        bool found = false;
        while (pos < nElements) {
            if (array->at(pos).first != v1) {
                found = true;
                break;
            }
            pos++;
        }
        if (found) {
            n = true;
        } else {
            n = false;
        }
        hasNextChecked = true;
    }

    if (constraint1 == -1) {
        n = pos < nElements;
    } else {
        // Check whether the elements in pos1 and pos2 satisfy the
        // constraints
        if (pos == nElements || array->at(pos).first != constraint1
                || (constraint2 != -1 && array->at(pos).second != constraint2)) {
            n = false;
        } else {
            n = true;
        }
    }
    hasNextChecked = true;
    return n;
}

int64_t ArrayItr::getCount() {
    if (!ignSecondColumn) {
        throw 10;
    }

    if (countElems == 0) {
        countElems = 1;
        //Pos points to the next element than the current one. This is
        //because this method is called after next()
        while ((pos) < nElements && array->at(pos).first ==
                array->at(pos - 1).first) {
            pos++;
            countElems++;
        }
    }

    return countElems;
}

void ArrayItr::next() {
    v1 = array->at(pos).first;
    v2 = array->at(pos).second;
    pos++;
    hasNextChecked = false;
    countElems = 0;
}

void ArrayItr::clear() {
    array = NULL;
}

void ArrayItr::mark() {
    markPos = pos;
}

void ArrayItr::reset(const char i) {
    pos = markPos;
    hasNextChecked = false;
}

void ArrayItr::moveto(const int64_t c1, const int64_t c2) {
    assert(v1 != -1);
    assert(ignSecondColumn || v2 != -1);

    if (v1 >= c1 && (ignSecondColumn || v1 > c1 || v2 >= c2)) {
	// Position is backwards.
        pos--; //so that hasNext will move to the same point
        hasNextChecked = true;
        n = true;
        return;
    }

    int p = binarySearch(array.get(), pos, nElements, c1);
    if (p < 0) {
        p = -p - 1;
    }
    pos = p;
    while (pos < nElements && (array->at(pos).first < c1 ||
                               (array->at(pos).first == c1 && array->at(pos).second < c2))) {
        pos++;
    }
    hasNextChecked = false;
}

void ArrayItr::ignoreSecondColumn() {
    ignSecondColumn = true;
}

int ArrayItr::binarySearch(Pairs *array, int start, int end, uint64_t key) {
    int low = start;
    int high = end - 1;

    while (low <= high) {
        int mid = (low + high) >> 1;
        int64_t midVal = array->at(mid).first;

        if (midVal < key)
            low = mid + 1;
        else if (midVal > key)
            high = mid - 1;
        else if (low != mid) // Equal but range is not fully scanned
            high = mid; // Set upper bound to current number and rescan
        else
            // Equal and full range is scanned
            return mid;
    }

    if (low >= end) {
        return -low - 1;
    } else {
        return high;
    }
}

void ArrayItr::init(std::shared_ptr<Pairs> values, int64_t v1, int64_t v2) {
    this->array = values;
    nElements = (int)values->size();
    constraint1 = v1;
    constraint2 = v2;
    countElems = 0;
    hasNextChecked = false;
    ignSecondColumn = false;
    this->v1 = this->v2 = -1;

    if (constraint1 != -1) {
        pos = binarySearch(array.get(), 0, nElements, constraint1);
        if (pos >= 0) {
            if (constraint2 != -1) {
                // The values are not many. Let's try to do a linear search
                // and see how it goes...
                while (pos < nElements && array->at(pos).first == constraint1
                        && array->at(pos).second < constraint2) {
                    pos++;
                }
                hasNextChecked = true;
                n = pos < nElements && array->at(pos).second == constraint2;
            } else {
                hasNextChecked = n = true;
            }
        } else {
            hasNextChecked = true;
            n = false;
            pos = 0;
        }
    } else {
        pos = 0;
        hasNextChecked = n = true;
    }
}

uint64_t ArrayItr::getCardinality() {
    //if (n) {
    if (ignSecondColumn) {
        uint64_t count = 0;
        size_t pos2 = 0;
        int64_t existingV1 = -1;
        while (pos2 < nElements) {
            if (array->at(pos2).first != existingV1) {
                count++;
                existingV1 = array->at(pos2).first;
            }
            pos2++;
        }
        return count;
    } else {
        /*uint64_t count = 1;
        size_t pos2 = pos + 1;
        while (pos2 < nElements) {
            if (array->at(pos2).first != constraint1) {
                break;
            }
            count++;
            pos2++;
        }
        return count;*/
        return nElements;
    }
    //} else {
    //    return 0;
    //}
}

uint64_t ArrayItr::estCardinality() {
    return getCardinality();
}
