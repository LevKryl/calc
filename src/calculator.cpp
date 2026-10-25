#include "calculator.h"

#include <charconv>
#include <cmath>

namespace {

    bool isDigit(char c) { return c >= '0' && c <= '9'; }
    bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

}  

void Calculator::addFunction(const std::string& name, Function f) {
    if (!functions_.emplace(name, std::move(f)).second)
        throw PluginError("function '" + name + "' is already defined");
}

void Calculator::addOperator(const std::string& symbol, Operator op) {
    if (!operators_.emplace(symbol, std::move(op)).second)
        throw PluginError("operator '" + symbol + "' is already defined");
}

double Calculator::evaluate(const std::string& text) {
    text_ = text;
    pos_ = 0;
    double result = parseSum();
    skipSpaces();
    if (pos_ < text_.size()) throw error("unexpected character");
    if (!std::isfinite(result)) throw MathError("result is out of range");
    return result;
}

double Calculator::parseSum() {
    double result = parseProduct();
    for (;;) {
        if (match('+')) result += parseProduct();
        else if (match('-')) result -= parseProduct();
        else return result;
    }
}

double Calculator::parseProduct() {
    double result = parseUnary();
    for (;;) {
        if (match('*')) {
            result *= parseUnary();
        }
        else if (match('/')) {
            double divisor = parseUnary();
            if (divisor == 0.0) throw MathError("division by zero");
            result /= divisor;
        }
        else {
            return result;
        }
    }
}


double Calculator::parseUnary() {
    if (match('-')) return -parseUnary();
    if (match('+')) return parseUnary();
    return parsePower();
}


double Calculator::parsePower() {
    double left = parsePrimary();
    if (const OperatorEntry* op = peekOperator()) {
        pos_ += op->first.size();
        double right = parseUnary();
        return op->second(left, right);
    }
    return left;
}

double Calculator::parsePrimary() {
    skipSpaces();
    if (pos_ >= text_.size()) throw SyntaxError("unexpected end of expression");
    if (match('(')) {
        double value = parseSum();
        expect(')');
        return value;
    }
    if (isDigit(text_[pos_]) || text_[pos_] == '.') return parseNumber();
    if (isAlpha(text_[pos_])) return parseCall();
    throw error("unexpected character");
}

double Calculator::parseNumber() {
    size_t start = pos_;
    while (pos_ < text_.size() && (isDigit(text_[pos_]) || text_[pos_] == '.')) ++pos_;
    double value = 0.0;
    auto res = std::from_chars(text_.data() + start, text_.data() + pos_, value);
    if (res.ec != std::errc() || res.ptr != text_.data() + pos_)
        throw SyntaxError("invalid number '" + text_.substr(start, pos_ - start) + "'");
    return value;
}

double Calculator::parseCall() {
    size_t start = pos_;
    while (pos_ < text_.size() && (isAlpha(text_[pos_]) || isDigit(text_[pos_]))) ++pos_;
    std::string name = text_.substr(start, pos_ - start);

    auto it = functions_.find(name);
    if (it == functions_.end()) throw SyntaxError("unknown function '" + name + "'");

    expect('(');
    double argument = parseSum();
    expect(')');
    return it->second(argument);
}


const Calculator::OperatorEntry* Calculator::peekOperator() {
    skipSpaces();
    const OperatorEntry* best = nullptr;
    for (const auto& entry : operators_)
        if (text_.compare(pos_, entry.first.size(), entry.first) == 0 &&
            (!best || entry.first.size() > best->first.size()))
            best = &entry;
    return best;
}

void Calculator::skipSpaces() {
    while (pos_ < text_.size() && (text_[pos_] == ' ' || text_[pos_] == '\t')) ++pos_;
}

bool Calculator::match(char c) {
    skipSpaces();
    if (pos_ < text_.size() && text_[pos_] == c) {
        ++pos_;
        return true;
    }
    return false;
}

void Calculator::expect(char c) {
    if (!match(c))
        throw SyntaxError(std::string("expected '") + c + "' at position " + std::to_string(pos_ + 1));
}

SyntaxError Calculator::error(const std::string& what) const {
    if (pos_ >= text_.size()) return SyntaxError(what + ": end of expression");
    return SyntaxError(what + " '" + text_[pos_] + "' at position " + std::to_string(pos_ + 1));
}