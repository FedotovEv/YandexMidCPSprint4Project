
#include "metric_accumulator_impl/categorical_accumulator.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include "metric_impl/metrics.hpp"
#include "analyse.hpp"
#include "utils.hpp"
#include "test_database.hpp"

using namespace std::literals;

class MetricsNameCategorySuite : public ::testing::TestWithParam<std::tuple<std::string, int, int, int, int, int>>
{
public:
    static constexpr std::array<std::string_view, 5> DETECTED_NAMING_STYLES = {"Snake Case"sv, "Pascal Case"sv, "Camel Case"sv, "Lower Case"sv, "Unknown"sv};

    TestDatabase test_db;
};

namespace analyzer::metric_accumulator::metric_accumulator_impl::test
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    TEST_P(MetricsNameCategorySuite, IdentCategoryDistribution)
    {
        auto [filename, snake_case_count, pascal_case_count, camel_case_count, lower_case_count, unknown_case_count] = GetParam();
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);

        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<NamingStyleMetric>());

        // Создаём и регистрируем сумматор метрики количества кодовых строк.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(NamingStyleMetric::kName, std::make_unique<CategoricalAccumulator>());

        // Готовим очередной (текущий) испытательный файл.
        test_db.CreateTestFile(filename);
        // ----- Вычисляем интересующую нас метрику (NamingStyleMetric) для каждой функции входного файла.
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
        // А затем подводим итоги, строя категориальное распределение (гистограмму) для всего файла.
        AccumulateFunctionAnalysis(file_analysis, accumulator);
        auto file_accumulated_data = accumulator.GetFinalizedAccumulator<CategoricalAccumulator>(NamingStyleMetric::kName).Get();

        // Сверка полученного распределения с истинными значениями.
        rs::for_each(file_accumulated_data, 
            [snake_case_count, pascal_case_count, camel_case_count, lower_case_count, unknown_case_count](const auto& naming_style_count_pair)
            {
                auto cnt_naming_style_it = std::find
                    (MetricsNameCategorySuite::DETECTED_NAMING_STYLES.begin(), MetricsNameCategorySuite::DETECTED_NAMING_STYLES.end(),
                     naming_style_count_pair.first);
                if (cnt_naming_style_it != MetricsNameCategorySuite::DETECTED_NAMING_STYLES.end())
                {
                    switch (cnt_naming_style_it - MetricsNameCategorySuite::DETECTED_NAMING_STYLES.begin())
                    {
                    case 0:
                        EXPECT_EQ(naming_style_count_pair.second, snake_case_count);
                        break;
                    case 1:
                        EXPECT_EQ(naming_style_count_pair.second, pascal_case_count);
                        break;
                    case 2:
                        EXPECT_EQ(naming_style_count_pair.second, camel_case_count);
                        break;
                    case 3:
                        EXPECT_EQ(naming_style_count_pair.second, lower_case_count);
                        break;
                    case 4:
                        [[fallthrough]];
                    default:
                        EXPECT_EQ(naming_style_count_pair.second, unknown_case_count);
                        break;
                    }
                }
            });
    }

    INSTANTIATE_TEST_SUITE_P
        (MetricsCategoricalPrefix, MetricsNameCategorySuite,
            ::testing::Values
            (
                std::make_tuple("if.py"s, -1, -1, 1, -1, -1),
                std::make_tuple("many_parameters.py"s, 1, -1, -1, -1, -1),
                std::make_tuple("sample.py"s, 3, -1, -1, 1, 1)
            )
        );
}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
