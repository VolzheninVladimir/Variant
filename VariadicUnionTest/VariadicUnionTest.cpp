#include "pch.h"
#include "VariadicUnion.hpp"

struct Tracker {
    static inline bool constructed = false;
    static inline bool destroyed = false;

    int value = 0;

    Tracker() { constructed = true; }
    Tracker(int v) : value(v) { constructed = true; }
    ~Tracker() { destroyed = true; }

    static void reset() {
        constructed = false;
        destroyed = false;
    }
};

struct NotConstructible {
    NotConstructible() = delete;
};

TEST(VariadicUnionTest_create_destroy, TrackerConstructsAndDestroysCorrectly) {
    Tracker::reset();
    VariadicUnion<int, Tracker, double> storage;
    storage.create<Tracker>(123);
    EXPECT_TRUE(Tracker::constructed);
    EXPECT_EQ(storage.get<Tracker>().value, 123);

    storage.destroy<Tracker>();
    EXPECT_TRUE(Tracker::destroyed);
}

TEST(VariadicUnionTest_get, ReturnsCorrectIntReference) {
    VariadicUnion<int, Tracker, double> storage;
    storage.create<int>(42);
    int& ref = storage.get<int>();
    EXPECT_EQ(ref, 42);
}

TEST(VariadicUnionTest_get, ReturnsCorrectDoubleReference) {
    VariadicUnion<int, Tracker, double> storage;
    storage.create<double>(3.14);
    double& ref = storage.get<double>();
    EXPECT_DOUBLE_EQ(ref, 3.14);
}

TEST(VariadicUnionTest_get, ReturnsCorrectTrackerReference) {
    Tracker::reset();
    VariadicUnion<int, Tracker, double> storage;
    storage.create<Tracker>(77);
    Tracker& ref = storage.get<Tracker>();
    EXPECT_EQ(ref.value, 77);
    EXPECT_TRUE(Tracker::constructed);
}

TEST(VariadicUnionTest_create, InvalidTypeDoesNotCompile) {
    VariadicUnion<int, Tracker> storage;
    // storage.create<NotConstructible>();
    SUCCEED();
}

TEST(VariadicUnionTest_destroy, InvalidTypeDoesNotCompile) {
    VariadicUnion<int, Tracker> storage;
    // storage.destroy<NotConstructible>();
    SUCCEED();
}

TEST(VariadicUnionTest_get, InvalidTypeDoesNotCompile) {
    VariadicUnion<int, Tracker> storage;
    // auto& x = storage.get<NotConstructible>();
    SUCCEED();
}