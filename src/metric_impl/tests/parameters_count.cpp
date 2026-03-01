#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"
#include "test_database.hpp"

using namespace std::literals;

struct FunctionParamsCount
{
    std::string function_name;
    int         params_count;   // Количество формальных параметров функции (метода класса) function_name.
};

using FunctionParamsCountV = std::vector<FunctionParamsCount>;

class OneMetricParamsCountSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionParamsCountV>>
{
public:
    TestDatabase test_db;
};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricParamsCountSuite, Func)
    {
        auto [filename, metric_results] = GetParam();
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        // Создадим подопытного кролика с ключом filename из базы данных test_db.
        test_db.CreateTestFile(filename);
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        // ----- Вычисляем интересующую нас метрику (CountParametersMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionParamsCount::function_name);
                // Требуем наличия найденной функции в списке metric_results, причём только однократного.
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                ASSERT_TRUE(metric_result_for_func_it->params_count >= 0);
                // Проверка правильности рассчитанных значений метрики (количества параметров функции) и ее имени.
                ASSERT_EQ(func_metric.second[0].metric_name, CountParametersMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->params_count, std::get<int>(func_metric.second[0].value));
                // Пометка задействованного для этой функции эталонного элемента для последующего контроля полноты и уникальности покрытия
                // функций тестового файла.
                metric_result_for_func_it->params_count = -1;
            });
        // Наконец, убедимся, что все ожидаемые в данном тестовом примере функции действительно были там обнаружены.
        ASSERT_EQ(rs::find_if(metric_results, [](int val) -> bool {return val >= 0;}, &FunctionParamsCount::params_count), metric_results.end());
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
