#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"

using namespace std::literals;

struct FunctionCodelineCount
{
    std::string function_name;
    int         codeline_count; // Количество кодосодержащих строк функции (метода класса) function_name.
};

using FunctionCodelineCountV = std::vector<FunctionCodelineCount>;

class OneMetricCodelineCountSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionCodelineCountV>>
{};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricCodelineCountSuite, Func)
    {
        auto [filename, metric_results] = GetParam();

        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());
        // "Доготавливаем" имя испытательного файла.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> test_file_pathname = ConstructFileNamesWithPath({filename}, use_tree_sitter_opt);
        // ----- Вычисляем интересующую нас метрику (CodeLinesCountMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions(test_file_pathname, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionCodelineCount::function_name);
                // Требуем наличия найденной функции в списке metric_results, совпадения их стилей наименования с требуемым,
                // а также правильное имя метрики (хотя последнее и тривиальность).
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                ASSERT_EQ(func_metric.second[0].metric_name, CodeLinesCountMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->codeline_count, std::get<int>(func_metric.second[0].value));
            });
    }

    INSTANTIATE_TEST_SUITE_P
        (OneMetricCodelinesPrefix, OneMetricCodelineCountSuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, FunctionCodelineCountV{ FunctionCodelineCount{"testIf"s, 3}}),
                std::make_tuple("many_parameters.py"s, FunctionCodelineCountV{ FunctionCodelineCount{"__test_multiparameters__"s, 1}}),
                std::make_tuple("sample.py"s,
                    FunctionCodelineCountV{FunctionCodelineCount{"__init__"s, 2}, FunctionCodelineCount{"process"s, 3},
                    FunctionCodelineCount{"__call__"s, 2}, FunctionCodelineCount{"lambda_demo"s, 6}})
            ));
}  // namespace analyzer::metric::metric_impl
