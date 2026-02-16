#include "metric_impl/naming_style.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyzer::metric::metric_impl {
std::string NamingStyleMetric::Name() const { return kName; }

MetricResult::ValueType NamingStyleMetric::CalculateImpl(const function::Function &f) const {
    auto &functionName = f.name;
    bool hasUnderscore = functionName.find('_') != std::string::npos;
    bool hasHyphen = functionName.find('-') != std::string::npos;
    bool hasUpper = std::any_of(functionName.begin(), functionName.end(), [](char c) { return isupper(c); });

    if (hasHyphen) {
        return "Unknown";
    }

    // Проверяем snake_case
    if (hasUnderscore) {
        // Должны быть только строчные буквы и подчеркивания
        bool allLower = std::all_of(functionName.begin(), functionName.end(),
                                    [](char c) { return islower(c) || c == '_' || isdigit(c); });
        if (allLower) {
            return "Snake Case";
        }
        return "Unknown";
    }

    // Проверяем PascalCase/CamelCase
    if (hasUpper) {
        // Первая буква заглавная - PascalCase
        if (isupper(functionName[0])) {
            return "Pascal Case";
        }
        // Первая буква строчная - camelCase
        else {
            return "Camel Case";
        }
    }

    // Все символы строчные без разделителей
    return "Lower Case";
}

}  // namespace analyzer::metric::metric_impl
