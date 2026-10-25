#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "plugin_api.h"

static const CalcPluginInfo info = { CALC_FUNCTION, "ln" };

CALC_EXPORT const CalcPluginInfo* calc_plugin_info() { return &info; }

CALC_EXPORT int calc_plugin_eval(const double* args, double* result, char* err, int errSize) {
    try {
        if (args[0] <= 0.0) throw std::domain_error("ln is defined only for x > 0");
        *result = std::log(args[0]);
        return 0;
    }
    catch (const std::exception& e) {  // exceptions must not cross the dll boundary
        std::snprintf(err, errSize, "%s", e.what());
        return 1;
    }
}