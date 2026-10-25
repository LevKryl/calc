#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "plugin_api.h"

static const CalcPluginInfo info = { CALC_OPERATOR, "^" };

CALC_EXPORT const CalcPluginInfo* calc_plugin_info() { return &info; }

CALC_EXPORT int calc_plugin_eval(const double* args, double* result, char* err, int errSize) {
    try {
        double base = args[0], exp = args[1];
        if (base == 0.0 && exp < 0.0)
            throw std::domain_error("cannot raise 0 to a negative power");
        if (base < 0.0 && exp != std::floor(exp))
            throw std::domain_error("cannot raise a negative number to a fractional power");
        *result = std::pow(base, exp);
        return 0;
    }
    catch (const std::exception& e) {  // exceptions must not cross the dll boundary
        std::snprintf(err, errSize, "%s", e.what());
        return 1;
    }
}