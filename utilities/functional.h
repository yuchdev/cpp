#pragma once
#include <map>
#include <string>
#include <functional>

// Define a type for functions that take no arguments and return void
using FunctionType = std::function<void()>;
using FunctionMap = std::map<std::string, FunctionType>;
