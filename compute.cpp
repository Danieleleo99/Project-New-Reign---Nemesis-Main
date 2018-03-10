#include "compute.h"

using namespace std;

void calculate(string& equation, bool& error, bool noDecimal)
{
	typedef exprtk::expression<double> expression_t;
	typedef exprtk::parser<double>         parser_t;

	std::string expression_string = equation;

	expression_t expression;

	parser_t parser;

	if (!parser.compile(expression_string, expression))
	{
		error = true;
	}

	if (noDecimal)
	{
		equation = to_string(int(expression.value()));
	}
	else
	{
		equation = to_string(expression.value());
	}

	return;
}
