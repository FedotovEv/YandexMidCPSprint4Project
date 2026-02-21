#include "metric_impl/cyclomatic_complexity.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"

using namespace std::literals;

struct FunctionCyclomaticComplex
{
    std::string function_name;
    int         cyclomatic_complexity; // Цикломатическая сложность функции (метода класса) function_name.
};

using FunctionCyclomaticComplexV = std::vector<FunctionCyclomaticComplex>;

class OneMetricCyclomaticCplxSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionCyclomaticComplexV>>
{};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricCyclomaticCplxSuite, Func)
    {
        auto [filename, metric_results] = GetParam();

        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());
        // "Доготавливаем" имя испытательного файла.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> test_file_pathname = ConstructFileNamesWithPath({filename}, use_tree_sitter_opt);
        // ----- Вычисляем интересующую нас метрику (CodeLinesCountMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions(test_file_pathname, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionCyclomaticComplex::function_name);
                // Требуем наличия найденной функции в списке metric_results, совпадения их стилей наименования с требуемым,
                // а также правильное имя метрики (хотя последнее и тривиальность).
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                ASSERT_EQ(func_metric.second[0].metric_name, CyclomaticComplexityMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->cyclomatic_complexity, std::get<int>(func_metric.second[0].value));
            });
    }

    INSTANTIATE_TEST_SUITE_P
        (OneMetricCyclomaticCplxPrefix, OneMetricCyclomaticCplxSuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, FunctionCyclomaticComplexV{FunctionCyclomaticComplex{"testIf"s, 2}}),
                std::make_tuple("many_parameters.py"s, FunctionCyclomaticComplexV{FunctionCyclomaticComplex{"__test_multiparameters__"s, 2}}),
                std::make_tuple("sample.py"s,
                    FunctionCyclomaticComplexV{FunctionCyclomaticComplex{"__init__"s, 1}, FunctionCyclomaticComplex{"process"s, 1},
                    FunctionCyclomaticComplex{"__call__"s, 1}, FunctionCyclomaticComplex{"lambda_demo"s, 1}})
            ));
}  // namespace analyzer::metric::metric_impl
