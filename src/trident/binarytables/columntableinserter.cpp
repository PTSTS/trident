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


#include <trident/binarytables/columntableinserter.h>

void ColumnTableInserter::startAppend() {
    tmpfirstpairs.clear();
    tmpsecondpairs.clear();
    largestElement = 0;
    maxGroupSize = 0;
}

void ColumnTableInserter::append(int64_t t1, int64_t t2) {
    if (tmpfirstpairs.size() == 0 || tmpfirstpairs.at(tmpfirstpairs.size() - 1).first != t1) {
        if (!tmpfirstpairs.empty() && tmpsecondpairs.size() - tmpfirstpairs.back().second > maxGroupSize) {
            maxGroupSize = tmpsecondpairs.size() - tmpfirstpairs.back().second;
        }
        tmpfirstpairs.push_back(std::make_pair(t1, tmpsecondpairs.size()));
    }
    tmpsecondpairs.push_back(t2);
    if (t2 >= largestElement) {
        largestElement = t2;
    }
}

uint8_t ColumnTableInserter::getNBytes(const int comprType, const int64_t value) const {
    switch (comprType) {
    case COMPR_1:
        return (uint8_t) Utils::numBytes(value);
    case COMPR_2:
        return (uint8_t) Utils::numBytes2(value);
    case NO_COMPR:
        return 8;
    }
    LOG(ERRORL) << "Should not happen";
    throw 10;
}

void ColumnTableInserter::stopAppend() {
    if (tmpsecondpairs.size() - tmpfirstpairs.back().second > maxGroupSize) {
        maxGroupSize = tmpsecondpairs.size() - tmpfirstpairs.back().second;
    }

    createNewFileIfCurrentIsTooLarge();
    uint64_t firstElement = tmpfirstpairs[0].first;
    writeFirstTerm((int64_t)firstElement);
    if (tmpfirstpairs.size() > 1) {
        writeVLong2((int64_t)tmpfirstpairs[1].second);
    } else {
        writeVLong2((int64_t)tmpsecondpairs.size());
    }

    short fileCoordinates = getCurrentFile();
    size_t posCoordinates = getCurrentPosition();

    if (tmpfirstpairs.size() > 1) {
        uint64_t diff = tmpfirstpairs[tmpfirstpairs.size() - 1].first - firstElement;
        bytesPerFirstEntry = getNBytes(compr1, diff);
        const uint64_t sizeSecondElements = tmpsecondpairs.size() *
                                            getNBytes(compr2, largestElement);

        bytesPerNElements = (uint8_t) Utils::numBytes2(maxGroupSize);
        writeByte(bytesPerNElements);
        bytesPerPointer = (uint8_t) Utils::numBytes2(sizeSecondElements);
        writeByte((uint8_t) (bytesPerFirstEntry << 4 | bytesPerPointer));

        writeVLong2(tmpfirstpairs.size());

        createNewFileIfCurrentIsTooLarge();
        fileCoordinates = getCurrentFile();
        posCoordinates = getCurrentPosition();

        //Write all the other first terms
        vector<std::pair<uint64_t, uint64_t>>::iterator itr = tmpfirstpairs.begin();
        itr++;
        uint64_t i = 1;
        for (; itr != tmpfirstpairs.end(); ++itr) {
            if (i % FIRST_INDEX_SIZE == 0) {
                index->add(itr->first - 1, getCurrentFile(), getCurrentPosition());
            }

            //reserve space for pointer
            uint64_t cp = getCurrentPosition();
            reserveBytes(bytesPerPointer);
            assert((cp + bytesPerPointer) == getCurrentPosition());

            //write diff first term
            writeFirstTerm((int64_t)(itr->first - firstElement));
            uint8_t bytesToFill = (uint8_t) (cp + bytesPerPointer +
                                             bytesPerFirstEntry - getCurrentPosition());
            assert(bytesToFill < bytesPerFirstEntry);
            while (bytesToFill-- > 0) {
                writeByte(0);
            }

            //Write the number of second elements for the previous entry
            cp = getCurrentPosition();
            if (i  < tmpfirstpairs.size() - 1) {
                writeVLong2(tmpfirstpairs[i + 1].second - itr->second);
            } else {
                writeVLong2(tmpsecondpairs.size() - itr->second);
            }
            assert(getCurrentPosition() > cp);
            int usedBytes = cp + bytesPerNElements - getCurrentPosition();
            assert(usedBytes < bytesPerNElements);
            assert(usedBytes >= 0);
            while (usedBytes-- > 0) {
                writeByte(0);
            }


            i++;
            createNewFileIfCurrentIsTooLarge();
        }
    } else {
        writeByte(0);
    }

    createNewFileIfCurrentIsTooLarge();
    std::vector<uint64_t>::iterator itr = tmpsecondpairs.begin();
    std::vector<std::pair<uint64_t, uint64_t>>::iterator itrFirst = tmpfirstpairs.begin();

    size_t currentCoordinate = 0;
    uint64_t firstSecondTerm = 0;
    bool addPos = true;

    int64_t bp = getCurrentPosition();
    short bpf = getCurrentFile();
    uint64_t nElements = 0;
    uint64_t nElementsGroup = 0;

    FileIndex *additionalIdx = NULL;
    uint64_t keyAdditionalIdx = 0;

    //Write the second elements
    for (; itr != tmpsecondpairs.end(); ++itr) {
        if (addPos && itrFirst->second == nElements) {
            if (additionalIdx != NULL) {
                index->addAdditionalIndex(keyAdditionalIdx, additionalIdx);
                additionalIdx = NULL;
            }

            keyAdditionalIdx = itrFirst->first;
            itrFirst++;
            if (itrFirst == tmpfirstpairs.end()) {
                addPos = false;
            }

            nElementsGroup = 0;

            if (nElements != 0) {
                //Update the pointer
                //short origFile = getCurrentFile();
                //size_t origPos = getCurrentPosition();

                int64_t diff = getNBytesFrom(bpf, bp);
                short fileToWrite = fileCoordinates;
                size_t posToWrite = posCoordinates + currentCoordinate * (bytesPerFirstEntry + bytesPerPointer + bytesPerNElements);
                while (posToWrite >= getFileSize(fileToWrite)) {
                    posToWrite -= getFileSize(fileToWrite);
                    fileToWrite++;
                }
                assert(fileToWrite != fileCoordinates || posToWrite >= posCoordinates);
                overwriteVLong2(fileToWrite, posToWrite, diff);
                currentCoordinate++;

                //setPosition(origFile, origPos);
            }
            firstSecondTerm = *itr;
            createNewFileIfCurrentIsTooLarge();
            writeSecondTerm(firstSecondTerm);
        } else {
            createNewFileIfCurrentIsTooLarge();
            writeSecondTerm(*itr - firstSecondTerm);
        }

        nElements++;
        nElementsGroup++;

        if (nElementsGroup % ADDITIONAL_SECOND_INDEX_SIZE == 0) {
            if (additionalIdx == NULL) {
                additionalIdx = new FileIndex();
            }
            createNewFileIfCurrentIsTooLarge();
            additionalIdx->add(*itr, getCurrentFile(), getCurrentPosition());
            firstSecondTerm = *itr;
        }
    }

    if (additionalIdx != NULL) {
        index->addAdditionalIndex(keyAdditionalIdx, additionalIdx);
    }
    tmpfirstpairs.clear();
    tmpsecondpairs.clear();
}

void ColumnTableInserter::writeFirstTerm(int64_t t) {
    switch (compr1) {
    case COMPR_1:
        writeVLong(t);
        return;
    case COMPR_2:
        writeVLong2(t);
        return;
    case NO_COMPR:
        writeLong(t);
        return;
    }
}

void ColumnTableInserter::writeSecondTerm(int64_t t) {
    switch (compr2) {
    case COMPR_1:
        writeVLong(t);
        return;
    case COMPR_2:
        writeVLong2(t);
        return;
    case NO_COMPR:
        writeLong(t);
    }
}

void ColumnTableInserter::setCompressionMode(int v1, int v2) {
    compr1 = v1;
    compr2 = v2;
}
