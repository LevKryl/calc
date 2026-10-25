#include "calculator.h"

#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include "plugin_api.h"

namespace fs = std::filesystem;

namespace {

   
    class Library {
    public:
        explicit Library(const fs::path& path) : handle_(LoadLibraryW(path.c_str())) {
            if (!handle_)
                throw PluginError("failed to load dll (error code " + std::to_string(GetLastError()) + ")");
        }
        ~Library() { FreeLibrary(handle_); }

        Library(const Library&) = delete;
        Library& operator=(const Library&) = delete;

        template <class Fn>
        Fn get(const char* name) const {
            FARPROC p = GetProcAddress(handle_, name);
            if (!p) throw PluginError(std::string("dll does not export ") + name);
            return reinterpret_cast<Fn>(reinterpret_cast<void*>(p));
        }

    private:
        HMODULE handle_;
    };

   
    double callPlugin(CalcEvalFn eval, const std::string& name, double a, double b = 0.0) {
        double args[2] = { a, b };
        double result = 0.0;
        char err[256] = {};
        if (eval(args, &result, err, sizeof(err)) != 0) {
            err[sizeof(err) - 1] = '\0';
            throw MathError(name + ": " + (err[0] ? err : "evaluation error"));
        }
        if (!std::isfinite(result)) throw MathError(name + ": result is undefined");
        return result;
    }

    bool isFunctionName(const std::string& s) {
        auto ok = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
        return !s.empty() && !std::isdigit(static_cast<unsigned char>(s[0])) && std::all_of(s.begin(), s.end(), ok);
    }

    bool isOperatorSymbol(const std::string& s) {
        const std::string allowed = "^%!&|~<>=@#$";
        return !s.empty() && s.size() <= 3 &&
            std::all_of(s.begin(), s.end(), [&](char c) { return allowed.find(c) != std::string::npos; });
    }

    std::string loadOne(const fs::path& path, Calculator& calc) {
        auto lib = std::make_shared<Library>(path);
        const CalcPluginInfo* info = lib->get<CalcInfoFn>("calc_plugin_info")();
        CalcEvalFn eval = lib->get<CalcEvalFn>("calc_plugin_eval");
        if (!info || !info->name) throw PluginError("invalid plugin info");

        const std::string name = info->name; 

        if (info->kind == CALC_FUNCTION) {
            if (!isFunctionName(name)) throw PluginError("invalid function name '" + name + "'");
            calc.addFunction(name, [lib, eval, name](double x) { return callPlugin(eval, name, x); });
            return name + "()";
        }
        if (info->kind == CALC_OPERATOR) {
            if (!isOperatorSymbol(name)) throw PluginError("invalid operator symbol '" + name + "'");
            calc.addOperator(name, [lib, eval, name](double a, double b) { return callPlugin(eval, name, a, b); });
            return "operator " + name;
        }
        throw PluginError("unknown plugin kind");
    }

}  
LoadReport loadPlugins(const fs::path& dir, Calculator& calc) {
    LoadReport report;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        report.errors.push_back("folder " + dir.u8string() + " not found");
        return report;
    }
   
    for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
        const fs::path& path = it->path();
        if (path.extension() != ".dll") continue;
        try {
            report.loaded.push_back(loadOne(path, calc));
        }
        catch (const std::exception& e) {
            report.errors.push_back(path.filename().u8string() + ": " + e.what());
        }
    }
    return report;
}