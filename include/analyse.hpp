//#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"
#include "utils.hpp"

namespace analyzer
{
    namespace rv = std::ranges::views;
    namespace rs = std::ranges;
    /**
     * @brief Анализирует список Python-файлов и извлекает метрики для всех функций и методов.
     *
     * Эта функция — центральный "конвейер" обработки:
     * 1. Принимает имена файлов.
     * 2. Для каждого файла создаёт объект `File`, который автоматически парсит его через tree-sitter
     *    и строит AST.
     * 3. Извлекает из AST все функции и методы с помощью `FunctionExtractor`.
     * 4. Объединяет все функции из всех файлов в один плоский список (`join`).
     * 5. Для каждой функции вычисляет набор метрик через переданный `metric_extractor`.
     * 6. Возвращает вектор пар: (функция, результаты её метрик).
     */
    inline auto AnalyseFunctions(const std::vector<std::string>& files, TreeSitterOpt sitter_opt,
                                 const analyzer::metric::MetricExtractor& metric_extractor)
    {
        auto functions_for_file = files | rv::transform([&sitter_opt](const std::string& next_file) -> std::vector<function::Function>
            {
                std::string real_next_file = !sitter_opt.common_files_path_prefix.empty() ?
                    (fs::path(sitter_opt.common_files_path_prefix) / fs::path(next_file)).string() : next_file;
                file::File parsed_file(real_next_file, sitter_opt.tree_sitter_exec, sitter_opt.tree_sitter_config);
                return function::FunctionExtractor{}.Get(parsed_file);
            }) |
            rv::join |
            rv::transform
                ([&metric_extractor](const function::Function& analizing_function) -> std::pair<function::Function, metric::MetricResults>
                    {
                        return {analizing_function, metric_extractor.Get(analizing_function)};
                    }
                );
    
            return rs::to<std::vector<std::pair<function::Function, metric::MetricResults>>>(functions_for_file);
    }

    /**
     * 
     * @brief Группирует результаты анализа по классам.
     *
     * Эта функция:
     * 1. Отфильтровывает только те функции, которые являются **методами классов**
     *    (у них `class_name.has_value()` == true).
     * 2. Группирует последовательные элементы с одинаковым именем класса с помощью `chunk_by`.
     *
     * Важно:
     * - `chunk_by` работает только с **последовательными** одинаковыми элементами!
     *   Поэтому предполагается, что входной диапазон уже упорядочен по классам
     *   (например, порядок методов в AST сохраняется как в исходном файле).
     * - Если порядок нарушен, один и тот же класс может быть разбит на несколько групп.
     *
     *  Чтобы убедиться, что фильтрация работает, проверьте, что свободные функции (без class_name)
     * действительно исчезают из результата.
     */
    auto SplitByClasses(const auto& analysis)
    {
        // analysis - это диапазон (точнее, вектор), состоящий из пар, первый член которых -  это function::Function,
        // а второй - metric::MetricResults.
        return analysis | rv::filter([](const std::pair<function::Function, metric::MetricResults>& test_function) -> bool
            {
                return test_function.first.class_name.has_value();
            }) |
            rv::chunk_by([](const std::pair<function::Function, metric::MetricResults>& test_function,
                            const std::pair<function::Function, metric::MetricResults>& next_function) -> bool
                {
                    return test_function.first.filename == next_function.first.filename &&
                           test_function.first.class_name == next_function.first.class_name;
                });
    }

    /**
     * @brief Группирует результаты анализа по исходным файлам.
     *
     * Эта функция:
     * - Разбивает весь список функций на группы, где каждая группа содержит
     *   только функции из одного и того же файла (`filename`).
     * - Использует `chunk_by`, поэтому **порядок функций в `analysis` должен быть по файлам**.
     */
    auto SplitByFiles(const auto& analysis)
    {
        // analysis - это диапазон (точнее, вектор), состоящий из пар, первый член которых -  это function::Function,
        // а второй - metric::MetricResults.
        return analysis | rv::chunk_by([]
            (const std::pair<function::Function, metric::MetricResults>& test_function,
             const std::pair<function::Function, metric::MetricResults>& next_function) -> bool
            {
                return test_function.first.filename == next_function.first.filename;
            });
    }

    /**
     * @brief Агрегирует метрики всех функций с помощью аккумулятора.
     *
     * Эта функция:
     * - Проходит по каждому элементу результата `AnalyseFunctions`
     *   (то есть по каждой функции и её метрикам).
     * - Передаёт результаты метрик (`elem.second`) в аккумулятор через `AccumulateNextFunctionResults`.
     */
    void AccumulateFunctionAnalysis(const auto& analysis,
                                    const analyzer::metric_accumulator::MetricsAccumulator& accumulator)
    {
        // analysis - это диапазон, элементы которого - пары, состоящие из function::Function и metric::MetricResults.
        rs::for_each(analysis, [&accumulator](const std::pair<function::Function, metric::MetricResults>& act_function)
            {
                accumulator.AccumulateNextFunctionResults(act_function.second);
            });
    }
}  // namespace analyzer
