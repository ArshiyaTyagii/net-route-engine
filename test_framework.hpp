// test_framework.hpp
// A deliberately tiny, dependency-free test harness. NetRoute Engine's
// tests don't require a framework as heavyweight as Catch2/GoogleTest,
// and keeping this self-contained means `make test` works offline with
// nothing but a C++17 compiler -- no package manager, no network access.

#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testfw {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const std::string& name, std::function<void()> fn) {
        registry().push_back(TestCase{name, std::move(fn)});
    }
};

class AssertionFailure : public std::exception {
public:
    explicit AssertionFailure(std::string msg) : msg_(std::move(msg)) {}
    [[nodiscard]] const char* what() const noexcept override { return msg_.c_str(); }
private:
    std::string msg_;
};

inline int runAll() {
    int passed = 0;
    int failed = 0;
    for (const auto& test : registry()) {
        try {
            test.fn();
            std::cout << "[ PASS ] " << test.name << "\n";
            ++passed;
        } catch (const AssertionFailure& e) {
            std::cout << "[ FAIL ] " << test.name << " -- " << e.what() << "\n";
            ++failed;
        } catch (const std::exception& e) {
            std::cout << "[ FAIL ] " << test.name << " -- unexpected exception: "
                      << e.what() << "\n";
            ++failed;
        }
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed, "
              << registry().size() << " total\n";
    return failed == 0 ? 0 : 1;
}

} // namespace testfw

#define TEST_CASE(name) \
    static void name(); \
    static testfw::Registrar registrar_##name(#name, name); \
    static void name()

#define REQUIRE(cond) \
    do { \
        if (!(cond)) { \
            std::ostringstream oss; \
            oss << "REQUIRE(" #cond ") failed at " << __FILE__ << ":" << __LINE__; \
            throw testfw::AssertionFailure(oss.str()); \
        } \
    } while (0)

#define REQUIRE_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::ostringstream oss; \
            oss << "REQUIRE_EQ(" #a ", " #b ") failed at " << __FILE__ << ":" << __LINE__ \
                << " -- lhs=" << (a) << " rhs=" << (b); \
            throw testfw::AssertionFailure(oss.str()); \
        } \
    } while (0)

#define REQUIRE_THROWS_AS(expr, exceptionType) \
    do { \
        bool threw = false; \
        try { (void)(expr); } \
        catch (const exceptionType&) { threw = true; } \
        catch (...) { \
            throw testfw::AssertionFailure( \
                "REQUIRE_THROWS_AS(" #expr ", " #exceptionType ") threw wrong type at " \
                + std::string(__FILE__) + ":" + std::to_string(__LINE__)); \
        } \
        if (!threw) { \
            throw testfw::AssertionFailure( \
                "REQUIRE_THROWS_AS(" #expr ", " #exceptionType ") did not throw at " \
                + std::string(__FILE__) + ":" + std::to_string(__LINE__)); \
        } \
    } while (0)
