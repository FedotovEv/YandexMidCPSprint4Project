#include "metric_accumulator_impl/sum_average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include "metric_impl/metrics.hpp"
#include "analyse.hpp"
#include "utils.hpp"
#include <test_database.hpp>

class CodeLineSummingFixture : public ::testing::Test
{
public:
    TestDatabase test_db;
};

// Параметризованный стенд для теста суммирующего аккумулятора метрики количества параметров функций и методов.
class CountParametersSummningSuite : public ::testing::TestWithParam<std::tuple<std::string, int>>
{
public:
    TestDatabase test_db;
};
// Параметризованный стенд для теста суммирующего аккумулятора метрики цикломатической сложности функций.
class CyclomaticComplexitySummningSuite : public ::testing::TestWithParam<std::tuple<std::string, int>>
{
public:
    TestDatabase test_db;
};

namespace analyzer::metric_accumulator::metric_accumulator_impl::test
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    // Непараметризованный групповой тест суммирующего аккумулятора метрик на материале подсчёта количества
    // содержательных строк набора Питонофайлов.
    TEST_F(CodeLineSummingFixture, CodeLineSummning)
    {
        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        // Список образцов, которые будут задействованы в этом тесте.
        std::vector<std::pair<std::string, int>> test_files_data{{"if.py", 3}, {"comments.py",  3}, {"many_lines.py", 11}};
        // Готовим к работе вычислитель количества кодосодержащих строк программного файла.
        analyzer::metric::MetricExtractor metric_extractor;
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());

        // Создаём и регистрируем сумматор метрики количества кодовых строк.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CodeLinesCountMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Готовим испытательные объекты.
        test_db.CreateTestFile(test_files_data[0].first);
        test_db.CreateTestFile(test_files_data[1].first);
        test_db.CreateTestFile(test_files_data[2].first);

        std::vector<std::string> all_test_files = test_files_data | rv::transform([](const auto& data_air) -> std::string
            {
                return data_air.first;
            }) | rs::to<std::vector<std::string>>();
        
        auto first_analysis = AnalyseFunctions({test_files_data[0].first}, use_tree_sitter_opt, metric_extractor);
        auto second_analysis = AnalyseFunctions({test_files_data[1].first}, use_tree_sitter_opt, metric_extractor);
        auto third_analysis = AnalyseFunctions({test_files_data[2].first}, use_tree_sitter_opt, metric_extractor);
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
        EXPECT_EQ(first_accumulated_data.sum, test_files_data[0].second);
        EXPECT_EQ(second_accumulated_data.sum, test_files_data[1].second);
        EXPECT_EQ(third_accumulated_data.sum, test_files_data[2].second);
        EXPECT_EQ(first_accumulated_data.sum + second_accumulated_data.sum + third_accumulated_data.sum, all_accumulated_data.sum);
    }

    // Параметризованный набор тестов суммирующего аккумулятора над набором метрик, представляющих соой количество параметров функций
    // и методов Питонопрограммы.
    TEST_P(CountParametersSummningSuite, CountParametersSummning)
    {
        auto [filename, correct_functions_param_count] = GetParam();

        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем единственную метрику - количество параметров функции.
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        // Будем суммировать количество всех параметров всех функций испытательного Питонофайла. Хотя такая метрика и не
        // имеет логического смысла, но вполне пригодна для тестирования правильности работы суммирующего аккумулятора.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CountParametersMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Готовим (создаём) испытательный объект.
        test_db.CreateTestFile(filename);
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
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

        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем единственную метрику - цикломатическую сложность функции.
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());
        // Производим суммирование цикломатической сложности всех имеющихся функций испытательного Питонофайла.
        // Сама по себе такая операция также бессмысленна и служит только проверочным целям.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CyclomaticComplexityMetric::kName, std::make_unique<SumAverageAccumulator>());

        // Проводим некоторую предобработку очередного испытательного файла.
        test_db.CreateTestFile(filename);
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
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
