#include "metric_impl/parameters_count.hpp"

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
std::string CountParametersMetric::Name() const { return kName; }

MetricResult::ValueType CountParametersMetric::CalculateImpl(const function::Function &f) const {
    auto &function_ast = f.ast;
    // 1. Находим начало блока параметров
    const std::string parameters_marker = "(parameters";
    size_t params_start = function_ast.find(parameters_marker);
    if (params_start == std::string::npos) {
        return 0;
    }

    // 2. Находим конец блока параметров
    size_t balance = 1;
    size_t params_end = params_start + parameters_marker.length();
    while (params_end < function_ast.size() && balance > 0) {
        if (function_ast[params_end] == '(')
            balance++;
        else if (function_ast[params_end] == ')')
            balance--;
        params_end++;
    }

    // 3. Извлекаем подстроку с параметрами
    std::string_view params_block(function_ast.data() + params_start, params_end - params_start);

    // 4. Считаем параметры (идентификаторы или pattern-ы)
    int count = 0;
    size_t pos = 0;
    const std::string id_marker = "(identifier";

    while ((pos = params_block.find(id_marker, pos)) != std::string_view::npos) {
        count++;
        pos += id_marker.length();
    }

    return count;
}

}  // namespace analyzer::metric::metric_impl
