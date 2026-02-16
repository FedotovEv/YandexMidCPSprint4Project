#include "metric_impl/code_lines_count.hpp"

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
std::string CodeLinesCountMetric::Name() const { return kName; }

MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    auto &function_ast = f.ast;

    // Вспомогательная лямбда для извлечения номера строки из диапазона узла AST.
    // Формат узла в S-выражении: (node_type [start_line,start_column] [end_line,end_column] ...)
    // Эта функция ищет открывающую скобку "[" после заданной позиции и парсит первую координату — номер строки.
    auto line_number = [&](int start_pos) {
        size_t line_pos = function_ast.find("[", start_pos);
        size_t comma_pos = function_ast.find(",", line_pos);
        return ToInt(function_ast.substr(line_pos + 1, comma_pos - line_pos - 1));
    };
    // Определяем начальную и конечную строки тела функции:
    // - начальная строка берётся из корневого узла функции (первое вхождение "[")
    // - конечная строка ищется по шаблону "] -"
    const int start_line = line_number(0);
    const int end_line = line_number(function_ast.find("] -"));
    
    // Лямбда, проверяющая, является ли конкретная строка "кодовой", то есть не комментарием.
    auto is_code_line = [&](int line) {
        std::string line_marker = "[" + std::to_string(line) + ",";
        size_t line_pos = function_ast.find(line_marker);

        if (line_pos == std::string::npos)
            return false;

        size_t node_start = function_ast.rfind('(', line_pos);
        if (node_start == std::string::npos)
            return false;

        std::string_view node_type =
            std::string_view(function_ast)
                .substr(node_start + 1, function_ast.find_first_of(" \n[", node_start + 1) - node_start - 1);

        return node_type != "comment";
    };
    // === ВАШ КОД ДОЛЖЕН БЫТЬ ЗДЕСЬ ===
    //
    // Цель: подсчитать количество строк в диапазоне [start_line + 1, end_line],
    // которые действительно содержат код (а не только комментарии или пустые строки).
    //
    // Почему start_line + 1?
    // Потому что первая строка — это строка с объявлением функции (def ...),
    // а тело функции начинается со следующей строки (обычно с отступа).                                             std::views::filter([&](int line) { return is_code_line(line); })));
}

}  // namespace analyzer::metric::metric_impl
