#pragma once
#include <map>
#include <string>
#include <functional>


template <typename ReturnT, typename... Args>
class FunctionMap {
public:
    // Define the function type based on the template parameters
    using FunctionType = std::function<ReturnT(Args...)>;

    // Constructor that takes a map of functions and populates the FunctionMap
    FunctionMap(const std::map<std::string, FunctionType>& functionMap) : functionMap(functionMap) {}

    // Add a function to the map
    void addFunction(const std::string& key, FunctionType function) {
        functionMap[key] = function;
    }

    FunctionType get(const std::string& key) const {
        auto it = functionMap.find(key);
        if (it != functionMap.end()) {
            return it->second;
        } 
        else {
            throw std::runtime_error("Function not found");
        }
    }

private:
    std::map<std::string, FunctionType> functionMap;
};

// Partial specialization for functions with no return value and no params
template <>
class FunctionMap<void, void> {
public:
    using FunctionType = std::function<void(void)>;

    // Constructor that takes a map of functions and populates the FunctionMap
    FunctionMap(const std::map<std::string, FunctionType>& functionMap) : functionMap(functionMap) {}

    void addFunction(const std::string& key, FunctionType function) {
        functionMap[key] = function;
    }

    FunctionType get(const std::string& key) const {
        auto it = functionMap.find(key);
        if (it != functionMap.end()) {
            return it->second;
        } 
        else {
            throw std::runtime_error("Function not found");
        }
    }

private:
    std::map<std::string, FunctionType> functionMap;
};
