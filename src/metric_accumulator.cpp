#include "metric_accumulator.hpp"

//#include <unistd.h>

#include <algorithm>
#include <any>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyzer::metric_accumulator
{
    /**
     * @brief Накапливает результаты метрик для одной функции.
     *
     * Эта функция принимает вектор результатов метрик (например, "цикломатическая сложность = 4",
     * "число строк = 12", "параметров = 3") и передаёт каждый результат соответствующему аккумулятору.
     *
     * Как это работает:
     * - Для каждого `metric_result` из `metric_results` извлекается имя метрики (`metric_name`).
     * - По этому имени в контейнере `accumulators` находится нужный аккумулятор.
     * - Вызывается метод `Accumulate(metric_result)`, который обновляет внутреннее состояние аккумулятора.
     */
    void MetricsAccumulator::AccumulateNextFunctionResults(const std::vector<metric::MetricResult>& metric_results) const
    {
        rs::for_each(metric_results, [this](const metric::MetricResult& one_metric)
            {
                if (auto accum_it = accumulators.find(one_metric.metric_name); accum_it != accumulators.end())
                    accum_it->second->Accumulate(one_metric);
            });
    }
    /**
     * @brief Сбрасывает состояние всех аккумуляторов.
     *
     * Эта функция вызывается, когда нужно начать новую агрегацию (например, для нового файла или класса).
     * Она проходит по всем зарегистрированным аккумуляторам и вызывает у каждого метод `Reset()`,
     * который обнуляет накопленные значения (сумму, счётчик и т.д.).
     */
    void MetricsAccumulator::ResetAccumulators()
    {
        rs::for_each(accumulators, [](auto& accum)
            {
                accum.second->Reset();
            });
    }
}  // namespace analyzer::metric_accumulator
