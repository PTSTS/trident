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


#include <trident/tree/intermediatenode.h>
#include <trident/tree/treecontext.h>
#include <trident/tree/cache.h>

#include <kognac/utils.h>

#include <iostream>
#include <assert.h>
#include <mutex>

#define CHILD_NOT_FOUND -1

IntermediateNode::IntermediateNode(TreeContext *context, Node *child1,
        Node *child2) :
    Node(context) {
        children = new Node*[context->getMaxElementsPerNode() + 1];
        idChildren = new int64_t[context->getMaxElementsPerNode() + 1];

        if (context->textKeys()) {
            int size = 0;
            tTerm *key = child1->largestTextualKey(&size);
            putkeyAt(key, size, 0);
        } else {
            int64_t key1 = child1->largestNumericKey();
            int64_t key2 = child2->smallestNumericKey();
            putkeyAt((key1 + key2) / 2, 0);
        }
        children[0] = child1;
        idChildren[0] = child1->getId();
        children[1] = child2;
        idChildren[1] = child2->getId();
        lastUpdatedChild = -1;
        setState(STATE_MODIFIED);
    }

int IntermediateNode::unserialize(char *bytes, int pos) {
    pos = Node::unserialize(bytes, pos);

    if (children == NULL) {
        children = new Node*[getCurrentSize() + 1];
        idChildren = new int64_t[getCurrentSize() + 1];
    }

    for (int i = 0; i < getCurrentSize() + 1; ++i) {
        int64_t id = Utils::decode_long(bytes, pos);
        pos += 8;
        idChildren[i] = id;
        children[i] = NULL;
    }
    return pos;
}

int IntermediateNode::serialize(char *bytes, int pos) {
    pos = Node::serialize(bytes, pos);

    for (int i = 0; i < getCurrentSize() + 1; ++i) {
        int64_t id;
        if (children[i] != NULL) {
            id = children[i]->getId();
        } else {
            id = idChildren[i];
        }
        Utils::encode_long(bytes, pos, id);
        pos += 8;
    }
    return pos;
}

Node *IntermediateNode::getChildForKey(int64_t key) {
    int p = pos(key);
    if (p < 0) {
        p = -p - 1;
    }
    ensureChildIsLoaded(p);
    return children[p];
}

Node *IntermediateNode::getChildAtPos(int p) {
    ensureChildIsLoaded(p);
    return children[p];
}

int IntermediateNode::getPosChild(Node *child) {
    if (lastUpdatedChild < getCurrentSize()
            && children[lastUpdatedChild + 1] == child) {
        return lastUpdatedChild + 1;
    }

    int p = 0;
    if (!getContext()->textKeys()) {
        p = pos(child->smallestNumericKey());
    } else {
        int size = 0;
        tTerm *t = child->smallestTextualKey(&size);
        p = pos(t, size);
    }
    if (p < 0) {
        p = -p - 1;
    }
    if (children[p] == child) {
        return p;
    }

    return CHILD_NOT_FOUND;
}

int64_t IntermediateNode::smallestNumericKey() {
    ensureChildIsLoaded(0);
    return children[0]->smallestNumericKey();
}

int64_t IntermediateNode::largestNumericKey() {
    ensureChildIsLoaded(getCurrentSize());
    return children[getCurrentSize()]->largestNumericKey();
}

tTerm *IntermediateNode::smallestTextualKey(int *size) {
    ensureChildIsLoaded(0);
    return children[0]->smallestTextualKey(size);
}

tTerm *IntermediateNode::largestTextualKey(int *size) {
    ensureChildIsLoaded(getCurrentSize());
    return children[getCurrentSize()]->largestTextualKey(size);
}

void IntermediateNode::cacheChild(Node *child) {
    int p = getPosChild(child);
    if (p == CHILD_NOT_FOUND) {
        LOG(ERRORL) << "Child: " << child->getId() << " is not found on node " << getId();
        LOG(ERRORL) << "CacheChild(): Position not found!";
    }
    children[p] = NULL;
    idChildren[p] = child->getId();
    lastUpdatedChild = p;
}

bool IntermediateNode::get(tTerm *key, const int sizeKey, nTerm *value) {
    int p = pos(key, sizeKey);
    if (p < 0) {
        p = -p - 1;
    }
    ensureChildIsLoaded(p);
    return children[p]->get(key, sizeKey, value);
}

//bool IntermediateNode::get(nTerm key, tTerm *container) {
//  int p = pos(key);
//  if (p < 0) {
//      p = -p - 1;
//  }
//  ensureChildIsLoaded(p);
//  return children[p]->get(key, container);
//}
bool IntermediateNode::get(nTerm key, int64_t &coordinates) {
    int p = pos(key);
    if (p < 0) {
        p = -p - 1;
    }
    ensureChildIsLoaded(p);
    return children[p]->get(key, coordinates);
}

bool IntermediateNode::get(nTerm key, TermCoordinates *value) {
    int p = pos(key);
    if (p < 0) {
        p = -p - 1;
    }
    ensureChildIsLoaded(p);
    return children[p]->get(key, value);
}

void numAvg(Node *parent, int p, Node *child1, Node *child2) {
    int64_t key1 = child1->largestNumericKey();
    int64_t key2 = child2->smallestNumericKey();
    parent->putkeyAt((key1 + key2) / 2, p);
}

void textAvg(Node *parent, int p, Node *child1, Node *child2) {
    int size = 0;
    tTerm *key = child1->largestTextualKey(&size);
    parent->putkeyAt(key, size, p);
}

Node *IntermediateNode::updateChildren(Node *split, int p,
        void (*insertAverage)(Node*, int p, Node*, Node*)) {
    setState(STATE_MODIFIED);
    if (shouldSplit()) {
        IntermediateNode *n1 = getContext()->getCache()->newIntermediateNode();
        n1->setId(getContext()->getNewNodeID());
        n1->setState(STATE_MODIFIED);

        if (n1->children == NULL) {
            n1->children = new Node*[getContext()->getMaxElementsPerNode() + 1];
            n1->idChildren =
                new int64_t[getContext()->getMaxElementsPerNode() + 1];
        }

        int minSize = getContext()->getMinElementsPerNode();
        if (p < minSize) {
            for (int i = 0; i < minSize + 1; ++i) {
                n1->children[i] = children[minSize + i];
                if (n1->children[i] != NULL) {
                    n1->children[i]->setParent(n1);
                } else {
                    n1->idChildren[i] = idChildren[minSize + i];
                }
                children[minSize + i] = NULL;
            }
            this->split(n1);
            removeLastKey();
            insertAverage(this, p, children[p], split);

            memmove(children + p + 2, children + p + 1,
                    (getCurrentSize() - p - 1) * sizeof(Node *));
            memmove(idChildren + p + 2, idChildren + p + 1,
                    (getCurrentSize() - p - 1) * sizeof(int64_t));
            children[p + 1] = split;
            split->setParent(this);

        } else {
            for (int i = 0; i < minSize; ++i) {
                n1->children[i] = children[minSize + 1 + i];
                if (n1->children[i] != NULL) {
                    n1->children[i]->setParent(n1);
                } else {
                    n1->idChildren[i] = idChildren[minSize + 1 + i];
                }
                children[minSize + 1 + i] = NULL;
            }
            this->split(n1);

            if (p > minSize) {
                n1->removeFirstKey();
                p -= minSize + 1;
                memmove(n1->children + p + 2, n1->children + p + 1,
                        sizeof(Node *) * (minSize - p - 1));
                memmove(n1->idChildren + p + 2, n1->idChildren + p + 1,
                        sizeof(int64_t) * (minSize - p - 1));

                insertAverage(n1, p, n1->children[p], split);
                n1->children[p + 1] = split;
            } else { // CASE: pos == getMinSize()
                memmove(n1->children + 1, n1->children,
                        minSize * sizeof(Node*));
                memmove(n1->idChildren + 1, n1->idChildren,
                        minSize * sizeof(int64_t));
                n1->children[0] = split;
            }

            split->setParent(n1);
        }
        getContext()->getCache()->registerNode(split);
        return n1;
    } else {
        insertAverage(this, p, children[p], split);
        memmove(children + p + 2, children + p + 1,
                (getCurrentSize() - p - 1) * sizeof(Node*));
        memmove(idChildren + p + 2, idChildren + p + 1,
                (getCurrentSize() - p - 1) * sizeof(int64_t));
        children[p + 1] = split;
        split->setParent(this);
        getContext()->getCache()->registerNode(split);
        return NULL;
    }
}

Node *IntermediateNode::put(nTerm key, int64_t coordinatesTTerm) {
    // Forward the request to the children
    int p = pos(key);
    if (p < 0) {
        p = -p - 1;
    }

    ensureChildIsLoaded(p);
    Node *split = children[p]->put(key, coordinatesTTerm);
    if (split != NULL) {
        return updateChildren(split, p, &numAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::put(nTerm key, TermCoordinates *value) {
    // Forward the request to the children
    int p = pos(key);
    if (p < 0) {
        p = -p - 1;
    }

    ensureChildIsLoaded(p);

    Node *split = children[p]->put(key, value);
    if (split != NULL) {
        return updateChildren(split, p, &numAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::put(tTerm *key, int sizeKey, nTerm value) {
    // Forward the request to the children
    int p = pos(key, sizeKey);
    if (p < 0) {
        p = -p - 1;
    }

    ensureChildIsLoaded(p);

    Node *split = children[p]->put(key, sizeKey, value);
    if (split != NULL) {
        return updateChildren(split, p, &textAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::append(tTerm *key, int sizeKey, nTerm value) {
    int p = getCurrentSize();
    ensureChildIsLoaded(p);

    Node *split = children[p]->append(key, sizeKey, value);
    if (split != NULL) {
        return updateChildren(split, p, &textAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::append(nTerm key, TermCoordinates *value) {
    int p = getCurrentSize();
    ensureChildIsLoaded(p);

    Node *split = children[p]->append(key, value);
    if (split != NULL) {
        return updateChildren(split, p, &numAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::append(nTerm key, int64_t coordinatesTerm) {
    int p = getCurrentSize();
    ensureChildIsLoaded(p);

    Node *split = children[p]->append(key, coordinatesTerm);
    if (split != NULL) {
        return updateChildren(split, p, &numAvg);
    } else {
        return NULL;
    }
}

Node *IntermediateNode::putOrGet(tTerm *key, int sizeKey, nTerm &value,
        bool &insertResult) {
    // Forward the request to the children
    int p = pos(key, sizeKey);
    if (p < 0) {
        p = -p - 1;
    }

    ensureChildIsLoaded(p);

    Node *split = children[p]->putOrGet(key, sizeKey, value, insertResult);
    if (insertResult && split != NULL) {
        return updateChildren(split, p, &textAvg);
    } else {
        return NULL;
    }
}

void IntermediateNode::ensureChildIsLoaded(int p) {
    if (children[p] == NULL) {
#ifdef MT
        std::recursive_mutex &mutex = getContext()->getMutex();
        std::unique_lock<std::recursive_mutex> lock(mutex);
        if (children[p] == NULL) {
#endif
            children[p] = getContext()->getCache()->getNodeFromCache(idChildren[p]);
            children[p]->setParent(this);
            getContext()->getCache()->registerNode(children[p]);
#ifdef MT
        }
        lock.unlock();
#endif
    }
}

Node *IntermediateNode::getChild(const int p) {
    return children[p];
}

Node *IntermediateNode::getChildForKey(tTerm *key, const int sizeKey) {
    int p = pos(key, sizeKey);
    if (p < 0) {
        p = -p - 1;
    }
    ensureChildIsLoaded(p);
    return children[p];
}

IntermediateNode::~IntermediateNode() {
    if (idChildren != NULL)
        delete[] idChildren;

    if (children != NULL) {
        if (getContext()->isReadOnly()) {
            for (int i = 0; i < getCurrentSize() + 1; ++i) {
                if (children[i] != NULL && children[i]->canHaveChildren()) {
                    delete children[i];
                }
            }
        }
        delete[] children;
    }
}
