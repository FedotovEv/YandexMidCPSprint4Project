#include "metric_impl/naming_style.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <cstdlib>
#include "analyse.hpp"
#include "utils.hpp"
#include <test_database.hpp>

using namespace std::literals;

struct FunctionNamingStyle
{
    std::string function_name;
    std::string naming_style;
};

using FunctionNamingStyleV = std::vector<FunctionNamingStyle>;

class OneMetricNameCategorySuite : public ::testing::TestWithParam<std::tuple<std::string, FunctionNamingStyleV>>
{
public:
    static constexpr std::array<std::string_view, 5> DETECTED_NAMING_STYLES = {"Snake Case"sv, "Pascal Case"sv, "Camel Case"sv, "Lower Case"sv, "Unknown"sv};

    TestDatabase test_db;
};

namespace analyzer::metric::metric_impl
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(OneMetricNameCategorySuite, FuncNamingStyleCat)
    {
        auto [filename, metric_results] = GetParam();
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл его конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        // Создадим подопытного кролика с ключом filename из базы данных test_db.
        test_db.CreateTestFile(filename);
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<NamingStyleMetric>());
        // ----- Вычисляем интересующую нас метрику (NamingStyleMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
        rs::for_each(file_analysis, [&metric_results](const auto& func_metric)
            {
                auto metric_result_for_func_it = rs::find(metric_results, func_metric.first.name, &FunctionNamingStyle::function_name);
                // Требуем наличия найденной функции в списке metric_results, причём встретиться она должна только однократно.
                ASSERT_NE(metric_result_for_func_it, metric_results.end());
                ASSERT_TRUE(metric_result_for_func_it->naming_style.size() > 0);
                // Проверка рассчитанной метрики - совпадение стиля наименования функции с требуемым, а также правильное имя самой
                // метрики (хотя последнее и тривиальность).
                ASSERT_EQ(func_metric.second[0].metric_name, NamingStyleMetric::kName);
                ASSERT_EQ(metric_result_for_func_it->naming_style, std::get<std::string>(func_metric.second[0].value));
                // Пометим использованный эталон найденной функции с целью контроля возможных повторов и неполноты покрытия метриками.
                metric_result_for_func_it->naming_style.clear();
            });
        // Наконец, убедимся, что все ожидаемые в данном тестовом примере функции действительно были там обнаружены.
        ASSERT_EQ(rs::find_if(metric_results,
            [](const std::string& val) -> bool {return val.size() > 0;}, &FunctionNamingStyle::naming_style),
            metric_results.end());
    }

    INSTANTIATE_TEST_SUITE_P
        (OneMetricCategoricalPrefix, OneMetricNameCategorySuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, FunctionNamingStyleV{FunctionNamingStyle{"testIf"s, "Camel Case"s}}),
                std::make_tuple("many_parameters.py"s, FunctionNamingStyleV{FunctionNamingStyle{"__test_multiparameters__"s, "Snake Case"s}}),
                std::make_tuple("sample.py"s,
                    FunctionNamingStyleV{FunctionNamingStyle{"__init__"s, "Snake Case"s}, FunctionNamingStyle{"process"s, "Lower Case"s},
                    FunctionNamingStyle{"__call__"s, "Snake Case"s}, FunctionNamingStyle{ "lambda_demo"s, "Snake Case"s}})
            ));
}  // namespace analyzer::metric::metric_impl
