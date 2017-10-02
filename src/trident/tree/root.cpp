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


#include <trident/tree/root.h>
#include <trident/tree/coordinates.h>
#include <trident/tree/leaf.h>
#include <trident/tree/cache.h>
#include <trident/tree/treecontext.h>
#include <trident/tree/stringbuffer.h>
#include <trident/tree/leaf.h>
#include <trident/utils/propertymap.h>

#include <kognac/utils.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <fstream>

using namespace std;
namespace fs = boost::filesystem;

Root::Root(string path, StringBuffer *buffer, bool readOnly, PropertyMap &conf) :
    readOnly(readOnly), path(path) {
    cache = new Cache(conf.getInt(MAX_NODES_IN_CACHE, 4),
                      conf.getBool(COMPRESSED_NODES, false));

    stringbuffer = buffer;
    bool textKeys = conf.getBool(TEXT_KEYS);
    bool textValues = conf.getBool(TEXT_VALUES);
    int maxElementsPerNode = conf.getInt(MAX_EL_PER_NODE, 2048);

    nodesKeysFactory = new PreallocatedArraysFactory<long>(maxElementsPerNode,
            conf.getInt(NODE_KEYS_FACTORY_SIZE, 10),
            conf.getInt(NODE_KEYS_PREALL_FACTORY_SIZE, 10));

//  LOG(DEBUG)<< "Size factory for the nodes keys " << conf.getInt(NODE_KEYS_FACTORY_SIZE) << " preallocated " << conf.getInt(NODE_KEYS_PREALL_FACTORY_SIZE);

//  timens::system_clock::time_point start = timens::system_clock::now();
    if (!textKeys && !textValues) {
        ilFactory = new PreallocatedFactory<Coordinates>(
            conf.getInt(LEAF_MAX_INTERNAL_LINES, 10),
            conf.getInt(LEAF_MAX_PREALL_INTERNAL_LINES, 10));
        ilBufferFactory = new PreallocatedArraysFactory<Coordinates*>(
            maxElementsPerNode / 2, conf.getInt(LEAF_ARRAYS_FACTORY_SIZE, 10),
            conf.getInt(LEAF_ARRAYS_PREALL_FACTORY_SIZE, 10));
    } else {
        ilFactory = NULL;
        ilBufferFactory = NULL;
    }
//  boost::chrono::duration<double> sec = boost::chrono::system_clock::now()
//          - start;
//  LOG(DEBUG)<< "Time init node factories: " << (sec.count() * 1000);

    context = new TreeContext(cache, stringbuffer, readOnly, maxElementsPerNode,
                              textKeys, textValues, ilFactory, ilBufferFactory, nodesKeysFactory);
    cache->init(context, path, conf.getInt(FILE_MAX_SIZE, 64 * 1024 * 1024),
                conf.getInt(MAX_N_OPENED_FILES, 2), conf.getLong(CACHE_MAX_SIZE, 32 * 1024 * 1024),
                conf.getInt(LEAF_SIZE_FACTORY, 1),
                conf.getInt(LEAF_SIZE_PREALL_FACTORY, 10), conf.getInt(NODE_MIN_BYTES, 0));

    // Check the directory to see whether the intermediate nodes are stored
    // on disk
    std::string f = path + string("/tree");
    std::ifstream is(f.c_str());

    if (is.good()) {
        is.seekg(0, std::ios_base::end);
        std::size_t size = is.tellg();
        is.seekg(0, std::ios_base::beg);
        char raw_input[size];
        is.read(raw_input, size);
        long id = Utils::decode_long(raw_input, 0);
        is.close();
        rootNode = cache->getNodeFromCache(id);
    } else { // new tree
        rootNode = cache->newLeaf();
        rootNode->setId(context->getNewNodeID());
        rootNode->setParent(NULL);
        rootNode->setState(STATE_MODIFIED);
        cache->registerNode(rootNode);

        //Create the directory if it does not exist
        if (!readOnly && !fs::exists(path)) {
            fs::create_directories(path);
        }
    }
}

bool Root::get(nTerm key, TermCoordinates *value) {
    Node *node = rootNode;
    while (node->canHaveChildren()) {
        node = node->getChildForKey(key);
    }
    bool resp = node->get(key, value);
    return resp;
}

bool Root::get(nTerm key, long &coordinates) {
    Node *node = rootNode;
    while (node->canHaveChildren()) {
        node = node->getChildForKey(key);
    }
    return node->get(key, coordinates);
}

bool Root::get(tTerm *key, const int sizeKey, nTerm *value) {
    Node *node = rootNode;
    while (node->canHaveChildren()) {
        node = node->getChildForKey(key, sizeKey);
    }
    return node->get(key, sizeKey, value);
}

void Root::put(nTerm key, long coordinates) {
    if (readOnly) {
        LOG(ERROR) << "Put is requested on a read-only tree";
        throw 10;
    }

    Node *n = rootNode->put(key, coordinates);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
}

void Root::put(nTerm key, TermCoordinates *value) {
    if (readOnly) {
        LOG(ERROR) << "Put is requested on a read-only tree";
        throw 10;
    }

    Node *n = rootNode->put(key, value);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
}

void Root::append(tTerm *key, int sizeKey, nTerm &value) {
    Node *n = rootNode->append(key, sizeKey, value);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
}

void Root::append(nTerm key, long coordinates) {
    Node *n = rootNode->append(key, coordinates);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
}

void Root::append(nTerm key, TermCoordinates *value) {
    Node *n = rootNode->append(key, value);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
}

bool Root::insertIfNotExists(tTerm *key, int sizeKey, nTerm &value) {
    bool insertResult;
    Node *n = rootNode->putOrGet(key, sizeKey, value, insertResult);
    if (n != NULL) {
        IntermediateNode *newRoot = cache->newIntermediateNode(rootNode, n);
        newRoot->setId(context->getNewNodeID());
        newRoot->setParent(NULL);
        rootNode->setParent(newRoot);
        n->setParent(newRoot);
        cache->registerNode(n);
        rootNode = newRoot;
    }
    return insertResult;
}

tTerm _smallest_text[] = "";
tTerm *SMALLEST_TEXT = _smallest_text;

TreeItr *Root::itr() {
    //Get smallest leaf
    Node *node = rootNode;
    if (context->textKeys()) {
        while (node->canHaveChildren()) {
            node = node->getChildForKey(SMALLEST_TEXT, strlen((char*)SMALLEST_TEXT));
        }
    } else {
        while (node->canHaveChildren()) {
            node = node->getChildForKey((long) 0);
        }
    }

    return new TreeItr(rootNode, (Leaf *) node);
}

void Root::flushChildrenToCache() {

    vector<Node*> nodesToRegister;

    if (rootNode->canHaveChildren()) {
        nodesToRegister.push_back(rootNode);
    }

    while (nodesToRegister.size() > 0) {
        Node *n = nodesToRegister.back();
        nodesToRegister.pop_back();

        for (int i = 0; i < n->getCurrentSize() + 1; ++i) {
            Node *child = n->getChild(i);
            if (child != NULL) {
                nodesToRegister.push_back(child);
            }
        }
        context->getCache()->flushNode(n, false);
    }
}

Root::~Root() {
    if (!readOnly) {
        // Save metainformation about the tree
        ofstream file(path + string("/tree"));
        char buffer[16];
        Utils::encode_long(buffer, 0, rootNode->getId());
        Utils::encode_long(buffer, 8, context->getNewNodeID());
        file.write(buffer, 16);
        file.close();

        // Flush all the leaves in the cache
        context->getCache()->flushAllCache();
    }

    // Now all the leaves are offloaded. Time to save also the other nodes.
    if (rootNode->canHaveChildren()) {
        // Save the intermediate nodes
        if (!readOnly) {
            flushChildrenToCache();
        } else {
            delete rootNode;
        }
    } else {
        cache->releaseLeaf(rootNode);
    }

    delete cache;

    if (ilFactory != NULL) {
        delete ilFactory;
    }

    if (ilBufferFactory != NULL) {
        delete ilBufferFactory;
    }

    delete nodesKeysFactory;
    delete context;
}
