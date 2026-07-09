// test_main.cpp
// Aggregates all TEST_CASE registrations (linked in from the other test
// translation units) and runs them.
#include "test_framework.hpp"

int main() {
    return testfw::runAll();
}
