#define CATCH_CONFIG_MAIN
#include "counter.h"

#include "catch2/catch.hpp"

std::atomic_uint64_t Counter::_created{0};
std::atomic_uint64_t Counter::_copied{0};
std::atomic_uint64_t Counter::_moved{0};
std::atomic_uint64_t Counter::_deleted{0};

std::mutex Counter::customMutex{};
std::unordered_map<std::string, uint64_t> Counter::_custom{};
