#pragma once
#include <filesystem>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct CalcError : std::runtime_error { using std::runtime_error::runtime_error; };
struct SyntaxError : CalcError { using CalcError::CalcError; };
struct MathError : CalcError { using CalcError::CalcError; };
struct PluginError : CalcError { using CalcError::CalcError; };

class Calculator {
public:
    using Function = std::function<double(double)>;
    using Operator = std::function<double(double, double)>;

    void addFunction(const std::string& name, Function f);
    void addOperator(const std::string& symbol, Operator op);
    double evaluate(const std::string& text);

private:
    using OperatorEntry = std::pair<const std::string, Operator>;

    // From lowest to highest priority:
    //   sum     = product { (+|-) product }
    //   product = unary { (*|/) unary }
    //   unary   = (+|-) unary | power
    //   power   = primary [ plugin_operator unary ]
    //   primary = number | ( sum ) | name ( sum )
    double parseSum();
    double parseProduct();
    double parseUnary();
    double parsePower();
    double parsePrimary();
    double parseNumber();
    double parseCall();

    const OperatorEntry* peekOperator();
    void skipSpaces();
    bool match(char c);
    void expect(char c);
    SyntaxError error(const std::string& what) const;

    std::map<std::string, Function> functions_;
    std::map<std::string, Operator> operators_;
    std::string text_;
    size_t pos_ = 0;
};

struct LoadReport {
    std::vector<std::string> loaded;
    std::vector<std::string> errors;
};

// Loads every .dll from dir. A broken dll is skipped, the others still load.
LoadReport loadPlugins(const std::filesystem::path& dir, Calculator& calc);