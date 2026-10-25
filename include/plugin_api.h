// exe <-> dll interface: plain C types only, no std::string or exceptions
#pragma once

#define CALC_EXPORT extern "C" __declspec(dllexport)

enum CalcPluginKind { CALC_FUNCTION = 0, CALC_OPERATOR = 1 };

struct CalcPluginInfo {
    int kind;          // CALC_FUNCTION: name(x)   CALC_OPERATOR: a name b
    const char* name;  // "sin" or "^"
};

// args: 1 number for a function, 2 numbers for an operator.
// Returns 0 on success, otherwise writes an error message to err.
using CalcInfoFn = const CalcPluginInfo* (*)();
using CalcEvalFn = int (*)(const double* args, double* result, char* err, int errSize);