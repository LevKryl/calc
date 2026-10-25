#include <windows.h>
#include <iomanip>
#include <iostream>
#include <string>

#include "calculator.h"

int main() {
    SetErrorMode(SEM_FAILCRITICALERRORS); 

    try {
        Calculator calc;  

        LoadReport report = loadPlugins("plugins", calc);
        for (const auto& e : report.errors) std::cerr << "Skipped plugin: " << e << '\n';
        if (report.loaded.empty())
            std::cerr << "No plugins loaded, only + - * / ( ) are available\n";
        for (const auto& name : report.loaded) std::cout << "Loaded: " << name << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "exit") {
            if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
            try {
                double result = calc.evaluate(line);
                if (result == 0.0) result = 0.0;  
                std::cout << std::setprecision(12) << result << '\n';
            }
            catch (const SyntaxError& e) {
                std::cout << "Syntax error: " << e.what() << '\n';
            }
            catch (const MathError& e) {
                std::cout << "Math error: " << e.what() << '\n';
            }
            catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << '\n';
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}