#include "Expression.h"

Expression::Expression() : expr(""), valid(false), implicit(false), xVal(0.0), yVal(0.0) {
    symbolTable.add_variable("x", xVal);
    symbolTable.add_variable("y", yVal);
    symbolTable.add_constants();
    compiled.register_symbol_table(symbolTable);
}

Expression::Expression(const std::string& exprStr) : implicit(false), xVal(0.0), yVal(0.0) {
    symbolTable.add_variable("x", xVal);
    symbolTable.add_variable("y", yVal);
    symbolTable.add_constants();
    compiled.register_symbol_table(symbolTable);
    set(exprStr);
}

void Expression::set(const std::string& exprStr) 
{
    expr = exprStr;
    implicit = false;

    std::string toCompile = exprStr;
    size_t eq = exprStr.find('=');
    if (eq != std::string::npos) 
    {
        implicit = true;
        std::string lhs = exprStr.substr(0, eq);
        std::string rhs = exprStr.substr(eq + 1);
        toCompile = "(" + lhs + ")-(" + rhs + ")";
    }

    symbolTable = exprtk::symbol_table<double>();
    symbolTable.add_variable("x", xVal);
    symbolTable.add_variable("y", yVal);
    symbolTable.add_constants();
    compiled    = exprtk::expression<double>();
    compiled.register_symbol_table(symbolTable);
    exprtk::parser<double> parser;
    valid = parser.compile(toCompile, compiled);
}

Expression::Expression(Expression&& other) noexcept
{
    expr = std::move(other.expr);
    valid = other.valid;
    implicit = other.implicit;
    xVal = 0.0;
    yVal = 0.0;
    symbolTable.add_variable("x", xVal);
    symbolTable.add_variable("y", yVal);
    symbolTable.add_constants();
    compiled.register_symbol_table(symbolTable);
    if (valid) 
    {
        exprtk::parser<double> parser;
        parser.compile(expr, compiled);
    }
    other.valid = false;
}

Expression& Expression::operator=(Expression&& other) noexcept {
    if (this != &other) {
        expr     = std::move(other.expr);
        valid    = other.valid;
        implicit = other.implicit;
        xVal     = 0.0;
        yVal     = 0.0;
        symbolTable = exprtk::symbol_table<double>();
        symbolTable.add_variable("x", xVal);
        symbolTable.add_variable("y", yVal);
        symbolTable.add_constants();
        compiled = exprtk::expression<double>();
        compiled.register_symbol_table(symbolTable);
        if (valid) {
            exprtk::parser<double> parser;
            parser.compile(expr, compiled);
        }
        other.valid = false;
    }
    return *this;
}

float Expression::evaluate(float x) const 
{
    if (!valid) return 0.0f;
    xVal = static_cast<double>(x);
    return static_cast<float>(compiled.value());
}

bool Expression::isValid() const { return valid; }

float Expression::evaluate2D(float x, float y) const {
    if (!valid) return 0.0f;
    xVal = static_cast<double>(x);
    yVal = static_cast<double>(y);
    return static_cast<float>(compiled.value());
}

bool Expression::isImplicit() const { return implicit; }