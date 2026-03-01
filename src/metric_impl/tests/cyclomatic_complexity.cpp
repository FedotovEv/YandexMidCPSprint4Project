#include "metric_impl/cyclomatic_complexity.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"
#include "test_database.hpp"

using namespace std::literals;

struct FunctionCyclomaticComplex
{
    std::string function_name;
    int         cyclomatic_complexity; // Цикломатическая сложность функции (метода класса) function_name.
};

using FunctionCyclomaticComplexV = std::vector<FunctionCyclomaticComplex>;

class OneMetricCyclomaticCplxSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionCyclomaticComplexV>>
{
public:
    TestDatabase test_db;
};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricCyclomaticCplxSuite, Func)
    {
        auto [filename, metric_results] = GetParam();
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        // Создадим подопытного кролика с ключом filename из базы данных test_db.
        test_db.CreateTestFile(filename);
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());
        // ----- Вычисляем интересующую нас метрику (CodeLinesCountMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionCyclomaticComplex::function_name);
                // Прежде всего, найденная функция должна быть среди ожидаемых в списке metric_results.
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                // Она не должна встретиться нам повторно.
                ASSERT_TRUE(metric_result_for_func_it->cyclomatic_complexity >= 0);
                // Проверяем правильность вычисленной метрики (цикломатической сложности функции) - её правильное имя (хотя это и
                // тривиальность) и точное значение.
                ASSERT_EQ(func_metric.second[0].metric_name, CyclomaticComplexityMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->cyclomatic_complexity, std::get<int>(func_metric.second[0].value));
                // Пометим условным значением использованный элемент в списке эталонов для целей контроля полноты покрытия
                // всех функций тестового модуля и отсутствия в составе результатов дубликатов метрик.
                metric_result_for_func_it->cyclomatic_complexity = -1;
            });
        // Наконец, убедимся, что все ожидаемые в данном тестовом примере функции действительно были там обнаружены.
        ASSERT_EQ(rs::find_if(metric_results, [](int val) -> bool {return val >= 0;}, &FunctionCyclomaticComplex::cyclomatic_complexity), metric_results.end());
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
