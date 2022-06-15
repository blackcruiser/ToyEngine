#pragma once

#define TOYENGINE_DEBUG 1

#ifndef TOYENGINE_DEBUG
    #define TOYENGINE_RELEASE
#endif

#include <memory>
#include <vector>
#include <map>

template<typename T>
using TEPtr = std::shared_ptr<T>;

template<typename T>
using TEPtrArr = std::vector<std::shared_ptr<T>>;
