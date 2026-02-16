#pragma once
#include <unistd.h>

#include <algorithm>
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

#include "metric.hpp"

namespace analyzer::metric::metric_impl {

struct CyclomaticComplexityMetric : IMetric {
    static inline const std::string kName = "Cyclomatic Complexity";

protected:
    std::string Name() const override;

    MetricResult::ValueType CalculateImpl(const function::Function& f) const override;};

}  // namespace analyzer::metric::metric_impl
