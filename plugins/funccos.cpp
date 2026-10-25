#include <cmath>
#include "plugin_api.h"

static const CalcPluginInfo info = { CALC_FUNCTION, "cos" };

CALC_EXPORT const CalcPluginInfo* calc_plugin_info() { return &info; }

// Argument in degrees
CALC_EXPORT int calc_plugin_eval(const double* args, double* result, char*, int) {
    double r = std::cos(args[0] * 3.14159265358979323846 / 180.0);
    *result = std::fabs(r) < 1e-15 ? 0.0 : r;
    return 0;
}