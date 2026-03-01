#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"
#include "test_database.hpp"

using namespace std::literals;

struct FunctionCodelineCount
{
    std::string function_name;
    int         codeline_count; // Количество кодосодержащих строк функции (метода класса) function_name.
};

using FunctionCodelineCountV = std::vector<FunctionCodelineCount>;

class OneMetricCodelineCountSuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionCodelineCountV>>
{
public:
    TestDatabase test_db;
};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricCodelineCountSuite, Func)
    {
        auto [filename, metric_results] = GetParam();
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл его конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        // Создадим подопытного кролика с ключом filename из базы данных test_db.
        test_db.CreateTestFile(filename);
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());
        // ----- Вычисляем интересующую нас метрику (CodeLinesCountMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);

        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionCodelineCount::function_name);
                // Требуем наличия найденной функции в списке metric_results.
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                // Проверим, что функция с тем же именем не появилась в результатах вычисления повторно (уже не встречалсь нам ранее).
                ASSERT_TRUE(metric_result_for_func_it->codeline_count >= 0);
                // Проверка корректности конкретного результата  - правильного числа строк в кодом в функции, а также верное имя
                // метрики (хотя последнее и тривиальность).
                ASSERT_EQ(func_metric.second[0].metric_name, CodeLinesCountMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->codeline_count, std::get<int>(func_metric.second[0].value));
                // Пометим найденный и проверенный эталон как использованный.
                metric_result_for_func_it->codeline_count = -1;
            });
        // Наконец, убедимся, что все нужные функции, которые ожидаются в данном тестовом примере, действительно были там обнаружены.
        ASSERT_EQ(rs::find_if(metric_results, [](int val) -> bool {return val >= 0;}, &FunctionCodelineCount::codeline_count), metric_results.end());
    }

    INSTANTIATE_TEST_SUITE_P
        (OneMetricCodelinesPrefix, OneMetricCodelineCountSuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, FunctionCodelineCountV{FunctionCodelineCount{"testIf"s, 3}}),
                std::make_tuple("many_parameters.py"s, FunctionCodelineCountV{FunctionCodelineCount{"__test_multiparameters__"s, 1}}),
                std::make_tuple("sample.py"s,
                    FunctionCodelineCountV{FunctionCodelineCount{"__init__"s, 2}, FunctionCodelineCount{"process"s, 3},
                    FunctionCodelineCount{"__call__"s, 2}, FunctionCodelineCount{"lambda_demo"s, 6}})
            ));
}  // namespace analyzer::metric::metric_impl
