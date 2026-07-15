#include <assert.h>
#include <iostream>
#include "my_hashset.hpp"

void testConstructor() {
    MyHashSet<int> set1;
    MyHashSet<int> set2(64);
    assert(set1.length() == 0);
    assert(set2.length() == 0);
    std::cout << "constructor test passed\n";
}

void testBasicInsert() {
    MyHashSet<int> set;
    assert(set.insert(1));
    assert(set.length() == 1);
    assert(!set.insert(1));
    std::cout << "basic insert test passed\n";
}

void testBasicContains() {
    MyHashSet<int> set;
    set.insert(2);
    assert(set.contains(2));
    assert(!set.contains(1));
    std::cout << "basic contains tests passed\n";
}

void testBasicRemove() {
    MyHashSet<int> set;
    assert(set.insert(2));
    assert(set.length() == 1);
    assert(!set.remove(1));
    assert(set.length() == 1);
    assert(set.remove(2));
    assert(set.length() == 0);
    assert(!set.remove(2));
    std::cout << "basic remove tests passed\n";
}

void testInsertTriggersResize() {
    MyHashSet<int> set;
    for (int i = -100; i <= 100; i++)
        assert(set.insert(i));
    assert(set.length() == 201);
    for (int i = -100; i <= 100; i++)
        assert(set.contains(i));
    assert(set.length() == 201);
    std::cout << "insert tests passed\n";
}

void testInserDelete() {
    MyHashSet<int> set;
    for (int i = -100; i <= 100; i++)
        assert(set.insert(i));
    assert(set.length() == 201);
    for (int i = -100; i <= 100; i++)
        assert(set.remove(i));
    assert(set.length() == 0);
    std::cout << "insert + delete tests passed\n";
}

void testInserDeleteObject() {
    MyHashSet<std::string> set;
    assert(set.insert("hello"));
    assert(set.contains("hello"));
    assert(!set.contains("aaa"));
    assert(!set.remove("aaa"));
    assert(set.remove("hello"));
    std::cout << "string tests passed\n";
}

int main() {
    testConstructor();
    testBasicInsert();
    testBasicContains();
    testBasicRemove();
    testInsertTriggersResize();
    testInserDelete();
    testInserDeleteObject();
    std::cout << "all test passed\n";
    return 0;
}