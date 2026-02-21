#include "metric_accumulator_impl/sum_average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include "metric_impl/metrics.hpp"
#include "analyse.hpp"
#include "utils.hpp"

// Параметризованный стенд для теста суммирующего аккумулятора метрики количества параметров функций и методов.
class CountParametersSummningSuite : public ::testing::TestWithParam<std::tuple<std::string, int>>
{};
// Параметризованный стенд для теста суммирующего аккумулятора метрики цикломатической сложности функций.
class CyclomaticComplexitySummningSuite : public ::testing::TestWithParam<std::tuple<std::string, int>>
{};

namespace analyzer::metric_accumulator::metric_accumulator_impl::test
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    // Непараметризованный групповой тест суммирующего аккумулятора метрик на материале подсчёта количества
    // содержательных строк набора Питонофайлов.
    TEST(SummAccumulatorTests, CodeLineSummning)
    {
        std::vector<std::pair<std::string, int>> test_files_data{{"if.py", 3}, {"comments.py",  3}, {"many_lines.py", 11}};
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());

        // Создаём и регистрируем сумматор метрики количества кодовых строк.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CodeLinesCountMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Готовим испытательные объекты.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> first_test_file = ConstructFileNamesWithPath({test_files_data[0].first}, use_tree_sitter_opt);
        std::vector<std::string> second_test_file = ConstructFileNamesWithPath({test_files_data[1].first}, use_tree_sitter_opt);
        std::vector<std::string> third_test_file = ConstructFileNamesWithPath({test_files_data[2].first}, use_tree_sitter_opt);

        auto test_files_view = test_files_data | rv::transform([](const auto& data_air) -> std::string
            {
                return data_air.first;
            });
        std::vector<std::string> all_test_files = ConstructFileNamesWithPath
            (rs::to<std::vector<std::string>>(test_files_view), use_tree_sitter_opt);
        
        auto test_files_summ_view = test_files_data | rv::transform([](const auto& data_air) -> int
            {
                return data_air.second;
            });
        std::vector<int> all_test_files_summ = rs::to<std::vector<int>>(test_files_summ_view);

        auto first_analysis = AnalyseFunctions(first_test_file, use_tree_sitter_opt, metric_extractor);
        auto second_analysis = AnalyseFunctions(second_test_file, use_tree_sitter_opt, metric_extractor);
        auto third_analysis = AnalyseFunctions(third_test_file, use_tree_sitter_opt, metric_extractor);
        auto all_analysis = AnalyseFunctions(all_test_files, use_tree_sitter_opt, metric_extractor);

        // -----
        AccumulateFunctionAnalysis(first_analysis, accumulator);
        auto first_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CodeLinesCountMetric::kName).Get();
        accumulator.ResetAccumulators();
        // -----
        AccumulateFunctionAnalysis(second_analysis, accumulator);
        auto second_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CodeLinesCountMetric::kName).Get();
        accumulator.ResetAccumulators();
        // -----
        AccumulateFunctionAnalysis(third_analysis, accumulator);
        auto third_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CodeLinesCountMetric::kName).Get();
        accumulator.ResetAccumulators();
        // -----
        AccumulateFunctionAnalysis(all_analysis, accumulator);
        auto all_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CodeLinesCountMetric::kName).Get();
        accumulator.ResetAccumulators();
        // Сверка полученных результатов расчёта метрик с вычисленными вручную заведомо верными значениями.
        EXPECT_EQ(first_accumulated_data.sum, all_test_files_summ[0]);
        EXPECT_EQ(second_accumulated_data.sum, all_test_files_summ[1]);
        EXPECT_EQ(third_accumulated_data.sum, all_test_files_summ[2]);
        EXPECT_EQ(first_accumulated_data.sum + second_accumulated_data.sum + third_accumulated_data.sum, all_accumulated_data.sum);
    }

    // Параметризованный набор тестов суммирующего аккумулятора над набором метрик, представляющих соой количество параметров функций
    // и методов Питонопрограммы.
    TEST_P(CountParametersSummningSuite, CountParametersSummning)
    {
        auto [filename, correct_functions_param_count] = GetParam();

        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем единственную метрику - количество параметров функции.
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        // Будем суммировать количество всех параметров всех функций испытательного Питонофайла. Хотя такая метрика и не
        // имеет логического смысла, но вполне пригодна для тестирования правильности работы суммирующего аккумулятора.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CountParametersMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Готовим испытательные объекты.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> test_file_pname = ConstructFileNamesWithPath({filename}, use_tree_sitter_opt);
        auto file_analysis = AnalyseFunctions(test_file_pname, use_tree_sitter_opt, metric_extractor);
        // -----
        AccumulateFunctionAnalysis(file_analysis, accumulator);
        auto first_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CountParametersMetric::kName).Get();
        // Сверка с переданным нам через параметры теста заведомо верного значения метрики.
        ASSERT_EQ(first_accumulated_data.sum, correct_functions_param_count);
    }

    INSTANTIATE_TEST_SUITE_P
        (CountParametersSummningPrefix, CountParametersSummningSuite,
            ::testing::Values
            (
                std::make_tuple("if.py", 1),
                std::make_tuple("many_parameters.py", 5),
                std::make_tuple("sample.py", 15)
            )
        );

    // Ещё один параметризованный набор тестов суммирующего аккумулятора, но уже поверх набора величин циеломатической сложности
    // функций и методов Питонопрограммы.
    TEST_P(CyclomaticComplexitySummningSuite, CyclomaticComplexitySummning)
    {
        auto [filename, correct_functions_param_count] = GetParam();

        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем единственную метрику - цикломатическую сложность функции.
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());
        // Производим суммирование цикломатической сложности всех имеющихся функций испытательного Питонофайла.
        // Сама по себе такая операция также бессмысленна и служит только проверочным целям.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CyclomaticComplexityMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Проводим некоторую предобработку очередного испытательного файла.
        TreeSitterOpt use_tree_sitter_opt = GetTriSitterOpt();
        std::vector<std::string> test_file_pname = ConstructFileNamesWithPath({filename}, use_tree_sitter_opt);
        auto file_analysis = AnalyseFunctions(test_file_pname, use_tree_sitter_opt, metric_extractor);
        // -----
        AccumulateFunctionAnalysis(file_analysis, accumulator);
        auto first_accumulated_data = accumulator.GetFinalizedAccumulator<SumAverageAccumulator>(CyclomaticComplexityMetric::kName).Get();
        // Сверка с переданным нам через параметры теста заведомо верного значения метрики.
        ASSERT_EQ(first_accumulated_data.sum, correct_functions_param_count);
    }

    INSTANTIATE_TEST_SUITE_P
        (CountParametersSummningPrefix, CyclomaticComplexitySummningSuite,
            ::testing::Values
            (
                std::make_tuple("nested_if.py", 4),
                std::make_tuple("loops.py", 4),
                std::make_tuple("sample.py", 4)
            )
        );
}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
