#include "metric_accumulator_impl/average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include "metric_impl/metrics.hpp"
#include "analyse.hpp"
#include "utils.hpp"
#include "test_database.hpp"

// Параметризованный стенд для теста усредняющкго аккумулятора трёх разных метрик набора исходный Питонофайлов - количества кодовых
// строк, числа аргументов функции и их цикломатической сложности.
class MetricsAveragingSuite : public ::testing::TestWithParam<std::tuple<std::string, double, double, double>>
{
public:
    TestDatabase test_db;
};

namespace analyzer::metric_accumulator::metric_accumulator_impl::test
{
    using namespace analyzer;
    using namespace analyzer::metric;
    using namespace analyzer::metric::metric_impl;

    // Параметризованный набор тестов усредняющего аккумулятора над семейством из трех разных метрик, независимо вычисленных
    // по набору входных файлов.
    TEST_P(MetricsAveragingSuite, AverageThreeMetricsCalculate)
    {
        auto [filename, correct_avg_codelines, correct_avg_functios_argums, correct_avg_func_cycl_cmlx] = GetParam();

        // Сначала вычисляем параметры настроек tree-sitter'а и создаём временный файл конфигурации, необходимый для его работы.
        TreeSitterOpt use_tree_sitter_opt = test_db.MakeTestTriSitterOpt(::testing::internal::GetArgvs()[0]);
        analyzer::metric::MetricExtractor metric_extractor;
        // Регистрируем все три имеющихся численных метрики, которые возиожно усреднить.
        metric_extractor.RegisterMetric(std::make_unique<CodeLinesCountMetric>());
        metric_extractor.RegisterMetric(std::make_unique<CountParametersMetric>());
        metric_extractor.RegisterMetric(std::make_unique<CyclomaticComplexityMetric>());

        // Создаём и регистрируем для этих метрик усредняющие аккумуляторы.
        analyzer::metric_accumulator::MetricsAccumulator accumulator;
        accumulator.RegisterAccumulator(CodeLinesCountMetric::kName, std::make_unique<AverageAccumulator>());
        accumulator.RegisterAccumulator(CountParametersMetric::kName, std::make_unique<AverageAccumulator>());
        accumulator.RegisterAccumulator(CyclomaticComplexityMetric::kName, std::make_unique<AverageAccumulator>());

        // Готовим и разбираем очередной (текущий) испытательный файл.
        test_db.CreateTestFile(filename);
        auto file_analysis = AnalyseFunctions({filename}, use_tree_sitter_opt, metric_extractor);
        // -----
        AccumulateFunctionAnalysis(file_analysis, accumulator);
        auto codelines_accumulated_data = accumulator.GetFinalizedAccumulator<AverageAccumulator>(CodeLinesCountMetric::kName).Get();
        auto count_params_accumulated_data = accumulator.GetFinalizedAccumulator<AverageAccumulator>(CountParametersMetric::kName).Get();
        auto cyclo_cplx_accumulated_data = accumulator.GetFinalizedAccumulator<AverageAccumulator>(CyclomaticComplexityMetric::kName).Get();
        
        // Сверка с переданным нам через параметры теста заведомо верного значения метрики.
        ASSERT_TRUE(fabs(codelines_accumulated_data - correct_avg_codelines) < ZERO_TOLERANCE);
        ASSERT_TRUE(fabs(count_params_accumulated_data - correct_avg_functios_argums) < ZERO_TOLERANCE);
        ASSERT_TRUE(fabs(cyclo_cplx_accumulated_data - correct_avg_func_cycl_cmlx) < ZERO_TOLERANCE);
    }

    INSTANTIATE_TEST_SUITE_P
        (MetricsAveragingPrefix, MetricsAveragingSuite,
            ::testing::Values
            (
                std::make_tuple("if.py", 3.0, 1.0, 2.0),
                std::make_tuple("many_parameters.py", 1.0, 5.0, 2.0),
                std::make_tuple("sample.py", 3.25, 3.75, 1.0)
            )
        );

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
