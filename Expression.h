#pragma once
#include <string>
#include "exprtk.hpp"

class Expression {
private:
    std::string expr;
    bool valid;
    bool implicit;           // â add
    mutable double xVal;
    mutable double yVal;     // â add
    exprtk::symbol_table<double> symbolTable;
    exprtk::expression<double>   compiled;

public:
    Expression();
    Expression(const std::string& exprStr);
    ~Expression() = default;
    Expression(const Expression&) = delete;
    Expression& operator=(const Expression&) = delete;
    Expression(Expression&& other) noexcept;
    Expression& operator=(Expression&& other) noexcept;
    float evaluate(float x) const;
    bool isValid() const;
    void set(const std::string& exprStr);
    float evaluate2D(float x, float y) const;   // â add
    bool  isImplicit() const;                    // â add
};