//Source: http://code.google.com/p/exparser/



#ifndef EXPARSER_H

#define EXPARSER_H

#include <string>

#include <vector>



/*!

  @def	DEFAULT_DECIMAL_PRECISION

  @brief	Default decimal precision. 

  @remarks	Justin Loundagin, 5/16/2011. 

*/



#define DEFAULT_DECIMAL_PRECISION 10



template<typename U, typename T, typename F> struct triple;

template<typename First> struct first_equal;



/*!

  @enum	ErrorIndex

  @brief	Values that represent an error index

  @verbatim

	  Error                           Explanation

	  ---------------------------     -------------------------------------------------------------

	  OPERATOR_LOGIC_ERROR			Invalid use of a mathmatical operator {(, ^, *, /, +, -, =}

	  PARENTHESIS_LOGIC_ERROR		Invalid use of parenthesis, either empty or unbalanced

	  FORMULA_PARAMETER_ERROR		Invalid use of formula, either empty or wrong number of parameters

	  FUNCTION_PARAMETER_ERROR		Invalid use of function, either empty or wrong number of parameters

	  NAN_ERROR						Not a number error

	  INFINITY_ERROR					Infinity error

  @endverbatim.

*/



enum ErrorIndex

{

	OPERATOR_LOGIC_ERROR,

	PARENTHESIS_LOGIC_ERROR,

	FORMULA_PARAMETER_ERROR,

	FUNCTION_PARAMETER_ERROR,

	NAN_ERROR,

	INFINITY_ERROR

};



/*!

  @typedef	triple<std::string, std::string, std::string> EXFormula

  @brief	Defines an alias representing the ex formula .

*/



typedef triple<std::string, std::string, std::string> EXFormula;



/*!

  @typedef	std::pair<std::string, double(*)(double)> EXFunction

  @brief	Defines an alias representing the ex function .

*/



typedef std::pair<std::string, double(*)(double)> EXFunction;



/*!

  @typedef	std::pair<char, double(*)(double, double)> EXOperator

  @brief	Defines an alias representing the ex operator .

*/



typedef std::pair<char, double(*)(double, double)> EXOperator;



/*!

  @typedef	std::pair<std::string, double> EXVariable

  @brief	Defines an alias representing the ex variable .

*/



typedef std::pair<std::string, double> EXVariable;



/*!

  @typedef	std::pair<std::string, std::string> EXSolveStep

  @brief	Defines an alias representing the ex solve step .

*/



typedef std::pair<std::string, std::string> EXSolveStep;



/*!

  @typedef	std::vector<EXFormula> EXFormulaSet

  @brief	Defines an alias representing set the ex formula belongs to .

*/



typedef std::vector<EXFormula> EXFormulaSet;



/*!

  @typedef	std::vector<EXFunction> EXFunctionSet

  @brief	Defines an alias representing set the ex function belongs to .

*/



typedef std::vector<EXFunction> EXFunctionSet;



/*!

  @typedef	std::vector<EXOperator> EXOperatorSet

  @brief	Defines an alias representing set the ex operator belongs to .

*/



typedef std::vector<EXOperator> EXOperatorSet;



/*!

  @typedef	std::vector<EXVariable> EXVariableSet

  @brief	Defines an alias representing set the ex variable belongs to .

*/



typedef std::vector<EXVariable> EXVariableSet;



/*!

  @typedef	std::vector<EXSolveStep> EXSolveSet

  @brief	Defines an alias representing set the ex solve belongs to .

*/



typedef std::vector<EXSolveStep> EXSolveSet;



/*!

  @class	EXParser

  @brief	Mathamatical Expression Parser

  @author	Justin Loundagin

  @date	5/16/2011

*/



class EXParser

{

	int decimal_precision;	



	/* Vectors */

	EXFormulaSet formulas;	

	EXFunctionSet functions;	

	EXOperatorSet operators;	

	EXVariableSet variables;	

	EXSolveSet steps;	



	/* Evaluation loop methods */

	bool priorityFormulaLoop(std::string &);

	bool priorityOperatorLoop(std::string &);

	bool priorityFunctionLoop(std::string &);
	
	bool priorityLoop(std::string &);



	/* Evaluation methods */

        bool evaluateOperator(std::string &, const int, EXOperatorSet::iterator);

	bool evaluateFormula(std::string &, const int, EXFormulaSet::iterator);

	bool evaluateFunction(std::string &, const int, EXFunctionSet::iterator);

	bool evaluateParenthesis(std::string &, const int);



	/* String parsing methods */

	void findAndReplaceVariables(std::string &, EXVariableSet &);

    void prepareExpression(std::string &_ex);

	void splitString(std::vector<std::string> &, const std::string, const char = ',');



	/* Operator parsing methods */

	std::string getLeftOfOperator(const std::string, const int);

	std::string getRightOfOperator(const std::string, const unsigned int);

	std::string isolateParenthesisExpression(const std::string &, const unsigned int);



	/* Checking methods */

    bool isNegitiveSign(const std::string, const int) const;

	bool isParenthesisBalanced(const std::string &);

	bool isScientificNotation(const std::string &, const int);

	bool isInf(double);

	bool isNan(double);



public:



	/* Constructor/Destructor methods */

    EXParser(void);



	/* Evaluation Method */

	std::string evaluate(std::string = "");



	/* Setter methods */

    void defineFunction(const std::string, double(*)(double));

    void defineVariable(const std::string, const double);

    void defineOperator(const char, double(*)(double, double));

	void defineFormula(const std::string, const std::string, const std::string);



	/* Setter methods */

	void setPrecision(const int);



	/* Getter methods */

    EXFunctionSet getFunctionSet(void) const;

	EXVariableSet getVariableSet(void) const;

    EXOperatorSet getOperatorSet(void) const;

    EXSolveSet getSolveSet(void) const;



	/* Default loading methods */

	void loadDefaultFunctions(void);

    void loadDefaultVariables(void);

    void loadDefaultOperators(void);

	void loadDefaultFormulas(void);



	/* Clearing methods */

	void clearVariables(void);

	void clearFormulas(void);

	void clearFunctions(void);

	void clearOperators(void);



	/* Conversion methods */

	std::string dblToStr(const double);

	double strToDbl(const std::string);

 private:
	std::string mErrorStr;

};



/*!

  @struct	triple

  @brief	Template struct to hold three different type variables

  @author	Justin Loundagin

  @date	5/16/2011

*/



template<typename U, typename T, typename F> struct triple{

	U first;

	T second;

	F third;

	triple(U first, T second, F third)

		:first(first), second(second), third(third){

	}

};



/*!

  @struct	first_equal

  @brief	Operator struct to compare values of type pair and triple

  @author	Justin Loundagin

  @date	5/16/2011

*/



template<typename First> struct first_equal

{

	const First value;

	first_equal(const First &_value) :value(_value){

	}



	template<typename Second> bool operator()(const std::pair<First, Second> &pair) const{

		return pair.first == value;

	}



	template<typename Second, typename Third> bool operator()(const triple<First, Second, Third> &triple) const{

		return triple.first == value;

	}

};



#endif // EXPARSER_H
