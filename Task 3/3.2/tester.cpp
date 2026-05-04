#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>

bool verify(const std::string& line) {
    size_t op = line.find('('), cl = line.find(')', op);
    std::string f = line.substr(0, op);
    std::string args = line.substr(op + 1, cl - op - 1);
    double expected = std::stod(line.substr(cl + 2));
    double result;

    if (f == "POW") {
        size_t comma = args.find(',');
        result = std::pow(std::stod(args.substr(0, comma)),
                          std::stod(args.substr(comma + 1)));
    } else if (f == "SIN") {
        result = std::sin(std::stod(args));
    } else if (f == "SQRT") {
        result = std::sqrt(std::stod(args));
    } else {
        return false;
    }

    double diff = std::fabs(result - expected);
    double tol = 1e-12;
    bool ok = (std::fabs(expected) > 1e-12) ? (diff / std::fabs(expected) <= tol)
                                            : (diff <= tol);
    if (!ok) {
        std::cerr << "MISMATCH: " << line << "\n  FOUND: " << result
                  << "\n  EXPECTED: " << expected << std::endl;
    }
    return ok;
}

int main() {
    std::vector<std::string> files = {
        "pow_results.txt", "sin_results.txt", "sqrt_results.txt"
    };
    bool all_ok = true;
    for (const auto& name : files) {
        std::ifstream file(name);
        if (!file) {
            std::cerr << "CANNOT OPEN " << name << std::endl;
            all_ok = false;
            continue;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && !verify(line))
                all_ok = false;
        }
    }
    std::cout << (all_ok ? "ALL OK\n"
                         : "ERRORS PRESENT\n");
    return all_ok ? 0 : 1;
}