#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"

using namespace std::literals;

struct FunctionParamsCount
{
    std::string function_name;
    int         params_count;   // Количество формальных параметров функции (метода класса) function_name.
};

using FunctionParamsCountV = std::vector<FunctionParamsCount>;

class OneMetricParamsCountSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionParamsCountV>>
{};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricParamsCountSuite, Func)
    {
        auto [filename, metric_results] = GetParam();

        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        // "Доготавливаем" имя испытательного файла.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> test_file_pathname = ConstructFileNamesWithPath({filename}, use_tree_sitter_opt);
        // ----- Вычисляем интересующую нас метрику (CountParametersMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions(test_file_pathname, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionParamsCount::function_name);
                // Требуем наличия найденной функции в списке metric_results, совпадения их стилей наименования с требуемым,
                // а также правильное имя метрики (хотя последнее и тривиальность).
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                ASSERT_EQ(func_metric.second[0].metric_name, CountParametersMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->params_count, std::get<int>(func_metric.second[0].value));
            });
    }

    INSTANTIATE_TEST_SUITE_P
        (OneMetricParamsCntPrefix, OneMetricParamsCountSuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, FunctionParamsCountV{FunctionParamsCount{"testIf"s, 1}}),
                std::make_tuple("many_parameters.py"s, FunctionParamsCountV{FunctionParamsCount{"__test_multiparameters__"s, 5}}),
                std::make_tuple("sample.py"s,
                    FunctionParamsCountV{FunctionParamsCount{"__init__"s, 5}, FunctionParamsCount{"process"s, 6},
                    FunctionParamsCount{"__call__"s, 4}, FunctionParamsCount{"lambda_demo"s, 0}})
            ));
}  // namespace analyzer::metric::metric_impl
