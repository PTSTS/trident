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


#include <snap/readers.h>
#include <trident/binarytables/factorytables.h>
#include <trident/binarytables/newcolumntable.h>
#include <trident/kb/kb.h>

const SnapReaders::pReader SnapReaders::readers[256] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &NewColumnTable::s_getValue1AtRow<1,2>,
    &NewColumnTable::s_getValue1AtRow<1,3>,
    &NewColumnTable::s_getValue1AtRow<1,4>,
    &NewColumnTable::s_getValue1AtRow<1,5>,
    &NewColumnTable::s_getValue1AtRow<1,6>,
    &NewColumnTable::s_getValue1AtRow<1,7>,
    &NewColumnTable::s_getValue1AtRow<1,8>,
    &NewColumnTable::s_getValue1AtRow<1,9>,
    &NewColumnTable::s_getValue1AtRow<1,10>,
    &NewColumnTable::s_getValue1AtRow<1,11>,
    &NewColumnTable::s_getValue1AtRow<1,12>,
    &NewColumnTable::s_getValue1AtRow<1,13>,
    &NewColumnTable::s_getValue1AtRow<1,14>,
    &NewColumnTable::s_getValue1AtRow<1,15>,
    &NewColumnTable::s_getValue1AtRow<1,16>,
    NULL,
    &NewColumnTable::s_getValue1AtRow<2,2>,
    &NewColumnTable::s_getValue1AtRow<2,3>,
    &NewColumnTable::s_getValue1AtRow<2,4>,
    &NewColumnTable::s_getValue1AtRow<2,5>,
    &NewColumnTable::s_getValue1AtRow<2,6>,
    &NewColumnTable::s_getValue1AtRow<2,7>,
    &NewColumnTable::s_getValue1AtRow<2,8>,
    &NewColumnTable::s_getValue1AtRow<2,9>,
    &NewColumnTable::s_getValue1AtRow<2,10>,
    &NewColumnTable::s_getValue1AtRow<2,11>,
    &NewColumnTable::s_getValue1AtRow<2,12>,
    &NewColumnTable::s_getValue1AtRow<2,13>,
    &NewColumnTable::s_getValue1AtRow<2,14>,
    &NewColumnTable::s_getValue1AtRow<2,15>,
    &NewColumnTable::s_getValue1AtRow<2,16>,
    NULL,
    &NewColumnTable::s_getValue1AtRow<3,2>,
    &NewColumnTable::s_getValue1AtRow<3,3>,
    &NewColumnTable::s_getValue1AtRow<3,4>,
    &NewColumnTable::s_getValue1AtRow<3,5>,
    &NewColumnTable::s_getValue1AtRow<3,6>,
    &NewColumnTable::s_getValue1AtRow<3,7>,
    &NewColumnTable::s_getValue1AtRow<3,8>,
    &NewColumnTable::s_getValue1AtRow<3,9>,
    &NewColumnTable::s_getValue1AtRow<3,10>,
    &NewColumnTable::s_getValue1AtRow<3,11>,
    &NewColumnTable::s_getValue1AtRow<3,12>,
    &NewColumnTable::s_getValue1AtRow<3,13>,
    &NewColumnTable::s_getValue1AtRow<3,14>,
    &NewColumnTable::s_getValue1AtRow<3,15>,
    &NewColumnTable::s_getValue1AtRow<3,16>,
    NULL,
    &NewColumnTable::s_getValue1AtRow<4,2>,
    &NewColumnTable::s_getValue1AtRow<4,3>,
    &NewColumnTable::s_getValue1AtRow<4,4>,
    &NewColumnTable::s_getValue1AtRow<4,5>,
    &NewColumnTable::s_getValue1AtRow<4,6>,
    &NewColumnTable::s_getValue1AtRow<4,7>,
    &NewColumnTable::s_getValue1AtRow<4,8>,
    &NewColumnTable::s_getValue1AtRow<4,9>,
    &NewColumnTable::s_getValue1AtRow<4,10>,
    &NewColumnTable::s_getValue1AtRow<4,11>,
    &NewColumnTable::s_getValue1AtRow<4,12>,
    &NewColumnTable::s_getValue1AtRow<4,13>,
    &NewColumnTable::s_getValue1AtRow<4,14>,
    &NewColumnTable::s_getValue1AtRow<4,15>,
    &NewColumnTable::s_getValue1AtRow<4,16>,
    NULL,
    &NewColumnTable::s_getValue1AtRow<5,2>,
    &NewColumnTable::s_getValue1AtRow<5,3>,
    &NewColumnTable::s_getValue1AtRow<5,4>,
    &NewColumnTable::s_getValue1AtRow<5,5>,
    &NewColumnTable::s_getValue1AtRow<5,6>,
    &NewColumnTable::s_getValue1AtRow<5,7>,
    &NewColumnTable::s_getValue1AtRow<5,8>,
    &NewColumnTable::s_getValue1AtRow<5,9>,
    &NewColumnTable::s_getValue1AtRow<5,10>,
    &NewColumnTable::s_getValue1AtRow<5,11>,
    &NewColumnTable::s_getValue1AtRow<5,12>,
    &NewColumnTable::s_getValue1AtRow<5,13>,
    &NewColumnTable::s_getValue1AtRow<5,14>,
    &NewColumnTable::s_getValue1AtRow<5,15>,
    &NewColumnTable::s_getValue1AtRow<5,16>,
    NULL,
    &NewColumnTable::s_getValue1AtRow<6,2>,
    &NewColumnTable::s_getValue1AtRow<6,3>,
    &NewColumnTable::s_getValue1AtRow<6,4>,
    &NewColumnTable::s_getValue1AtRow<6,5>,
    &NewColumnTable::s_getValue1AtRow<6,6>,
    &NewColumnTable::s_getValue1AtRow<6,7>,
    &NewColumnTable::s_getValue1AtRow<6,8>,
    &NewColumnTable::s_getValue1AtRow<6,9>,
    &NewColumnTable::s_getValue1AtRow<6,10>,
    &NewColumnTable::s_getValue1AtRow<6,11>,
    &NewColumnTable::s_getValue1AtRow<6,12>,
    &NewColumnTable::s_getValue1AtRow<6,13>,
    &NewColumnTable::s_getValue1AtRow<6,14>,
    &NewColumnTable::s_getValue1AtRow<6,15>,
    &NewColumnTable::s_getValue1AtRow<6,16>,
    NULL,
    &NewRowTable<ByteReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<ByteReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<ShortReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<IntReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, ShortReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, IntReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, LongIntReader>::s_getValue1AtRow,
    &NewRowTable<LongIntReader, LongIntReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, ByteReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, ByteReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, ShortReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, ShortReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, IntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, IntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ByteReader, LongIntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, ByteReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, ByteReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, ShortReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, ShortReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, IntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, IntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<ShortReader, LongIntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, ByteReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, ByteReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, ShortReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, ShortReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, IntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, IntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<IntReader, LongIntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, ByteReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, ByteReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, ShortReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, ShortReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, IntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, IntReader, IntReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, LongIntReader, ByteReader>::s_getValue1AtRow,
    &NewClusterTable<LongIntReader, LongIntReader, IntReader>::s_getValue1AtRow,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


std::vector<const char*> SnapReaders::f_sop;
std::vector<const char*> SnapReaders::f_osp;

void SnapReaders::loadAllFiles(KB *kb) {
    //Load all files and put them in a static array
    SnapReaders::f_sop = kb->openAllFiles(IDX_SOP);
    SnapReaders::f_osp = kb->openAllFiles(IDX_OSP);
}
