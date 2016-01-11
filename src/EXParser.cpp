//Source: http://code.google.com/p/exparser/



#include "EXParser.h"

#include <algorithm>

#include <iostream>

#include <sstream>

#include <iomanip>

#include <cmath>



/*!

  @fn	double add(double a, double b)

  @brief	Adds variable a and b

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @param	b	double b

  @return	The sum of a and b

*/



double add(double a, double b){return a+b;}



/*!

  @fn	double subtract(double a, double b)

  @brief	Subtracts variable a and b

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @param	b	double b

  @return	The difference of a and b

*/



double subtract(double a, double b){return a-b;}



/*!

  @fn	double multiply(double a, double b)

  @brief	Multiplies variable a and b

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @param	b	double b

  @return	The product of variable a and b

*/



double multiply(double a, double b){return a*b;}



/*!

  @fn	double divide(double a, double b)

  @brief	Divides variable a and b

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @param	b	double b

  @return	The quotient of variable a and b

*/



double divide(double a, double b){return a/b;}



/*!

  @fn	double factorial(double a)

  @brief	Finds the factorial variable a

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @return	The factorial of a

*/



double factorial(double a){

	if(a == 0){

		return 1;

	}

	return (a==1) ? a : a * factorial(a-1);

}



/*!

  @fn	double equal(double a, double b)

  @brief	Finds boolean value of a equals b

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @param	b	double b

  @return	if true than 1, else 0

*/



double equal(double a, double b){

	return (double)(a==b);

}


/*!

  @fn	EXParser::EXParser(void)

  @brief	Default constructor. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



EXParser::EXParser(void)

:decimal_precision(DEFAULT_DECIMAL_PRECISION)

{

	loadDefaultFormulas();

	loadDefaultFunctions();

	loadDefaultVariables();

	loadDefaultOperators();

}



/*!

  @fn	std::string EXParser::evaluate(std::string _expression)

  @brief	Evaluates a mathamatical expression

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @return	If error exists than returns error, otherwise the evaluated expression

*/



std::string EXParser::evaluate(std::string _expression)

{

	steps.clear(); //clear previous solve steps

	findAndReplaceVariables(_expression, this->variables);

	prepareExpression(_expression);

	if(!priorityLoop(_expression)) //main calculation loop
	  return mErrorStr;

	return _expression;

}



/*!

  @fn	void EXParser::priorityLoop(std::string &_expression)

  @brief	Loop driver to iterate through all mathamatical procedures

  @author	Justin Loundagin

  @date	5/16/2011

  @exception	EXError	Thrown when ex error. 

  @param [in,out]	_expression	The expression. 

*/



bool EXParser::priorityLoop(std::string &_expression) {
    if(!isParenthesisBalanced(_expression)){
      mErrorStr = "parenthesis logic error";
      return false;
    }
    
    if(!priorityFormulaLoop(_expression))
      return false;
    
    if(!priorityFunctionLoop(_expression))
      return false;
    
    if(!priorityOperatorLoop(_expression))
      return false;
    
    return true;
}



/*!

  @fn	void EXParser::priorityOperatorLoop(std::string &_expression)

  @brief	Priority operator loop to evaluate operator arithmetic

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

*/



bool EXParser::priorityOperatorLoop(std::string &_expression)

{

	for(EXOperatorSet::iterator oiter = operators.begin(); oiter!=operators.end(); ++oiter){

		for(size_t index = 0; index<_expression.length(); index++){

			if(oiter->first == _expression[index] && !isScientificNotation(_expression, index)){

				if(oiter->first != '-' || !isNegitiveSign(_expression, index)){

					std::string prevExpression = _expression;

					if(!evaluateOperator(_expression, index, oiter))
					  return false;

					steps.push_back(EXSolveStep(prevExpression.append(" -> ").append(_expression), std::string(&oiter->first, 1)));

					oiter = operators.begin(); //reset operator loop to beginning

				}

			}

		}

	}
	return true;
}



/*!

  @fn	void EXParser::priorityFunctionLoop(std::string &_expression)

  @brief	Priority function loop to evaluate all function values

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

*/



bool EXParser::priorityFunctionLoop(std::string &_expression)

{

	size_t pos;

	for(EXFunctionSet::iterator fiter = functions.begin(); fiter!=functions.end(); ++fiter){

		while((pos=_expression.find(fiter->first, 0))!=std::string::npos){

		  if(!evaluateFunction(_expression, pos+fiter->first.length(), fiter))
		    return false;

		}

	}
	return true;
}



/*!

  @fn	void EXParser::priorityFormulaLoop(std::string &_expression)

  @brief	Priority formula loop to evaluate all formula values

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

*/



bool EXParser::priorityFormulaLoop(std::string &_expression)

{

	size_t pos;

	for(EXFormulaSet::iterator foiter = formulas.begin(); foiter!=formulas.end(); ++foiter){

		while((pos=_expression.find(foiter->first, 0))!=std::string::npos){

		  if(!evaluateFormula(_expression, pos+foiter->first.length(), foiter))
		    return false;

		}

	}
	return true;
}



/*!

  @fn	void EXParser::evaluateOperator(std::string & _expression, const int index,

  EXOperatorSet::iterator oiter)

  @brief	Evaluates an operator. 

  @author	Justin Loundagin

  @date	5/16/2011

  @exception	EXError	Thrown when ex error. 

  @param [in,out]	_expression	The expression. 

  @param	index				Zero-based index of the. 

  @param	oiter				The operator iterator

*/



bool EXParser::evaluateOperator(std::string & _expression, const int index, EXOperatorSet::iterator oiter)

{

	if(oiter->first!='('){

		std::string leftVal, rightVal, strVal;

		if((leftVal = getLeftOfOperator(_expression, index-1)).empty() || (rightVal = getRightOfOperator(_expression, index+1)).empty()){

		  mErrorStr = "operator logic error";
		  return false;
		}



		double value = oiter->second(strToDbl(leftVal.c_str()), strToDbl(rightVal.c_str()));

		if(isInf(value)){
		  mErrorStr = "infinity error";
		  return false;
		}

		if(isNan(value)){
		  mErrorStr = "not a number";
		  return false;
		}

		_expression.replace(index-leftVal.length(), leftVal.length() + rightVal.length()+1, dblToStr(value));

	}

	else{

		evaluateParenthesis(_expression, index);

	}

	return true;
}



/*!

  @fn	void EXParser::evaluateParenthesis(std::string & _expression, const int index)

  @brief	Evaluate a parenthesis operator. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

  @param	index				Zero-based index of the. 

*/



bool EXParser::evaluateParenthesis(std::string & _expression, const int index)

{

	std::string tempExpression = isolateParenthesisExpression(_expression, index);

	size_t expressionLen = tempExpression.length();

	if(!priorityOperatorLoop(tempExpression))
	  return false;

	_expression.replace(index, expressionLen+2, tempExpression);

	return true;
}



/*!

  @fn	void EXParser::evaluateFunction(std::string &_expression, const int index,

  EXFunctionSet::iterator fiter)

  @brief	Evaluates a function. 

  @author	Justin Loundagin

  @date	5/16/2011

  @exception	EXError	Thrown when ex error. 

  @param [in,out]	_expression	The expression. 

  @param	index				Zero-based index of the. 

  @param	fiter				The function iterator

*/



bool EXParser::evaluateFunction(std::string &_expression, const int index, EXFunctionSet::iterator fiter)

{

	std::string paramExpression = isolateParenthesisExpression(_expression, index);

	size_t expressionLen = paramExpression.length();

	if(expressionLen == 0){
	  mErrorStr = "function parameter error";
	  return false;
	}

	if(!priorityLoop(paramExpression))
	  return false;

	double value = fiter->second((double)atof(paramExpression.c_str()));

	if(isInf(value)){
	  mErrorStr = "infinity error";
	  return false;
	}

	if(isNan(value)){
	  mErrorStr = "not a number error";
	  return false;
	}

	steps.push_back(EXSolveStep(fiter->first+"("+paramExpression+") -> "+dblToStr(value), ""));

	_expression.replace(index-fiter->first.length(), fiter->first.length()+expressionLen+2, dblToStr(value));


	return true;
}



/*!

  @fn	void EXParser::evaluateFormula(std::string &_expression, const int index,

  EXFormulaSet::iterator foiter)

  @brief	Evaluates a formula. 

  @author	Justin Loundagin

  @date	5/16/2011

  @exception	EXError	Thrown when ex error. 

  @param [in,out]	_expression	The expression. 

  @param	index				Zero-based index of the. 

  @param	foiter				The formula iterator

*/



bool EXParser::evaluateFormula(std::string &_expression, const int index, EXFormulaSet::iterator foiter)

{

	EXVariableSet formulaVariables;

	std::vector<std::string> formulaVariableNames, formulaVariableValues;

	std::string formulaParameters = isolateParenthesisExpression(_expression, index), 

				formulaExpression = foiter->second, token;

	int formulaParamLen = formulaParameters.length();

	priorityLoop(formulaParameters);

	splitString(formulaVariableNames, foiter->third);

	splitString(formulaVariableValues, formulaParameters);

	if(formulaVariableNames.size()!=formulaVariableValues.size()){
	  mErrorStr = "formula parameter error";
	  return false;
	}

	for(unsigned int i=0; i<formulaVariableNames.size(); i++){

		formulaVariables.push_back(EXVariable(formulaVariableNames.at(i), atof(formulaVariableValues.at(i).c_str())));

	}

	findAndReplaceVariables(formulaExpression, formulaVariables);

	_expression.replace(index-foiter->first.length(), foiter->first.length()+formulaParamLen+2, formulaExpression);

	return true;
}



/*!

  @fn	std::string EXParser::isolateParenthesisExpression(const std::string &_expression,

  const int firstParen)

  @brief	Isolate parenthesis expression. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @param	firstParen	The first paren. 

  @return	The isolated string between parenthesis

*/



std::string EXParser::isolateParenthesisExpression(const std::string &_expression, const unsigned int firstParen)

{

        unsigned int opened = 0, closed = 0, i=firstParen;

	for(; i<_expression.length(); i++){

		if(_expression[i] == '('){

			opened++;

		}

		else if(_expression[i] == ')'){

			closed++;

			if(opened == closed){

				break;

			}

		}

	}

	return _expression.substr(firstParen+1, i-1-firstParen);

}



/*!

  @fn	void EXParser::splitString(std::vector<std::string> &vec, const std::string str,

  const char del)

  @brief	Splits a string. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	vec	The vector. 

  @param	str			The string. 

  @param	del			The delimeter

*/



void EXParser::splitString(std::vector<std::string> &vec, const std::string str, const char del)

{

	std::stringstream ss(str);

	std::string token;

	while(std::getline(ss, token, del)){

		vec.push_back(token);

	}

}



/*!

  @fn	void EXParser::loadDefaultFormulas(void)

  @brief	Loads the default formulas. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::loadDefaultFormulas(void)

{

	// default formulas here

}



/*!

  @fn	void EXParser::loadDefaultFunctions(void)

  @brief	Loads the default functions. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::loadDefaultFunctions(void)

{

	/* Load Default Functions Here */

	defineFunction("asin",	asin);

	defineFunction("acos",	acos);

	defineFunction("atan",	atan);

	defineFunction("sinh",	sinh);

	defineFunction("cosh",	cosh);

	defineFunction("tanh",	tanh);

	defineFunction("sin",	sin);

	defineFunction("cos",	cos);

	defineFunction("tan",	tan);

	defineFunction("sqrt",	sqrt);

	defineFunction("fac",	factorial);

	defineFunction("exp",	exp);

	defineFunction("ln" ,	log);

	defineFunction("log10",	log10);





}



/*!

  @fn	void EXParser::loadDefaultVariables(void)

  @brief	Loads the default variables. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::loadDefaultVariables(void)

{

	defineVariable("pi",   3.14159);   //pi constant

	//defineVariable("e",    2.718282);  //eulers constant

}



/*!

  @fn	void EXParser::loadDefaultOperators(void)

  @brief	Loads the default operators. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::loadDefaultOperators(void)

{

	

	/* Default order of operations : PEDMSA */

	defineOperator('(', NULL);

	defineOperator('^', pow);

	defineOperator('/', divide);

	defineOperator('*', multiply);

	defineOperator('-', subtract);

	defineOperator('+', add);

	defineOperator('=', equal);

}



/*!

  @fn	void EXParser::defineFormula(const std::string name, const std::string formula,

  const std::string variables)

  @brief	Defines a formula. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	name		The name. 

  @param	formula		The formula. 

  @param	variables	The variables. 

*/



void EXParser::defineFormula(const std::string name, const std::string formula, const std::string variables)

{

	formulas.erase(std::remove_if(formulas.begin(), formulas.end(), first_equal<std::string>(name)), formulas.end());

	formulas.push_back(EXFormula(name, formula, variables));

}



/*!

  @fn	void EXParser::defineFunction(const std::string name, double(*func)(double))

  @brief	Defines a function. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	name			The name. 

  @param [in,out]	func	If non-null, the func. 

*/



void EXParser::defineFunction(const std::string name, double(*func)(double))

{

	functions.erase(std::remove_if(functions.begin(), functions.end(), first_equal<std::string>(name)), functions.end());

	functions.push_back(EXFunction(name, func));

}



/*!

  @fn	void EXParser::defineOperator(const char sym, double(*op)(double, double))

  @brief	Defines a operator. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	sym			The symbol. 

  @param [in,out]	op	If non-null, the operation. 

*/



void EXParser::defineOperator(const char sym, double(*op)(double, double))

{

	operators.erase(std::remove_if(operators.begin(), operators.end(), first_equal<char>(sym)), operators.end());

	operators.push_back(EXOperator(sym, op));

}



/*!

  @fn	void EXParser::defineVariable(const std::string name, const double value)

  @brief	Defines a variable. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	name	The name. 

  @param	value	The value. 

*/



void EXParser::defineVariable(const std::string name, const double value)

{

	variables.erase(std::remove_if(variables.begin(), variables.end(), first_equal<std::string>(name)),variables.end());

	variables.push_back(EXVariable(name, value));

}



/*!

  @fn	EXFunctionSet EXParser::getFunctionSet(void) const

  @brief	Gets the function set. 

  @author	Justin Loundagin

  @date	5/16/2011

  @return	The function set. 

*/



EXFunctionSet EXParser::getFunctionSet(void) const

{

    return functions;

}



/*!

  @fn	EXVariableSet EXParser::getVariableSet(void) const

  @brief	Gets the variable set. 

  @author	Justin Loundagin

  @date	5/16/2011

  @return	The variable set. 

*/



EXVariableSet EXParser::getVariableSet(void) const

{

    return variables;

}



/*!

  @fn	EXOperatorSet EXParser::getOperatorSet(void) const

  @brief	Gets the operator set. 

  @author	Justin Loundagin

  @date	5/16/2011

  @return	The operator set. 

*/



EXOperatorSet EXParser::getOperatorSet(void) const

{

    return operators;

}



/*!

  @fn	EXSolveSet EXParser::getSolveSet(void) const

  @brief	Gets the solve set. 

  @author	Justin Loundagin

  @date	5/16/2011

  @return	The solve set. 

*/



EXSolveSet EXParser::getSolveSet(void) const

{

	return steps;

}



/*!

  @fn	bool EXParser::isNegitiveSign(const std::string _expression, const int index) const

  @brief	Query if '_expression' at 'index' is a negitive sign. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @param	index		Zero-based index of the. 

  @return	true if negitive sign, false if not. 

*/



bool EXParser::isNegitiveSign(const std::string _expression, const int index) const

{

	return index-1<0? true: !isdigit(_expression[index-1]);

}



/*!

  @fn	std::string EXParser::getLeftOfOperator(const std::string _expression, const int index)

  @brief	Gets a left of operator. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @param	index		Zero-based index of the. 

  @return	The left of operator. 

*/



std::string EXParser::getLeftOfOperator(const std::string _expression, const int index)

{

	std::string val;

	for(int i=index; i>=0; i--){

		if(isdigit(_expression[i])||_expression[i] == '.'||_expression[i] == '-'||_expression[i]=='e')

		{

			if(_expression[i] == '-' && (int)(i-1)>=0){

				if(isdigit(_expression[(int)(i-1)])){

					break;

				}

			}



			val.push_back(_expression[i]);

		}

		else{

			break;

		}

	}

	std::reverse(val.begin(), val.end());

	return val;

}



/*!

  @fn	std::string EXParser::getRightOfOperator(const std::string _expression, const int index)

  @brief	Gets a right of operator. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @param	index		Zero-based index of the. 

  @return	The right of operator. 

*/



std::string EXParser::getRightOfOperator(const std::string _expression, const unsigned int index)

{

	std::string val;



	for(unsigned int i=index; i<_expression.length(); i++){

		if(_expression.at(i) == '-' && i == index){

			val.push_back('-');

			continue;

		}

		if(isdigit(_expression[i])||_expression[i] == '.'|| _expression[i] == 'e'){



			val.push_back(_expression[i]);

			if((int)(i+1)<_expression.length() && _expression.at(i) == 'e'){

				if(_expression.at(i+1)=='+' || _expression.at(i+1)=='-'){

					val.push_back(_expression[i+1]);

					i++;

				}

			}

		}

		else

			break;

	}

	return val;

}



/*!

  @fn	std::string EXParser::dblToStr(const double num)

  @brief	Double to string. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	num	Number of. 

  @return	String of double num 

*/



std::string EXParser::dblToStr(const double num)

{

	std::stringstream ss;

	ss<<std::setprecision(decimal_precision)<<num;

	return ss.str();

}



/*!

  @fn	double EXParser::strToDbl(const std::string num)

  @brief	String to double. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	num	Number of. 

  @return	Double value of string num

*/



double EXParser::strToDbl(const std::string num)

{

	double val;

	std::istringstream sstr(num);

	sstr>>std::setprecision(decimal_precision)>>val;

	return val;

}



/*!

  @fn	void EXParser::setPrecision(const int _precision)

  @brief	Sets a precision. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_precision	The precision. 

*/



void EXParser::setPrecision(const int _precision){

	decimal_precision = _precision;

}



/*!

  @fn	void EXParser::findAndReplaceVariables(std::string &_expression,

  EXVariableSet &_variables)

  @brief	Searches for and replaces variables. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

  @param [in,out]	_variables	The variables. 

*/



void EXParser::findAndReplaceVariables(std::string &_expression, EXVariableSet &_variables)

{

	for(EXVariableSet::iterator vIter = _variables.begin(); vIter!=_variables.end(); ++vIter){

		size_t j, start_index = 0;

		for(; (j = _expression.find(vIter->first, start_index))!=std::string::npos;){

			bool is_var = true;

			if((int)(j+vIter->first.length())<_expression.length()){

				if(isalpha(_expression.at((int)j+vIter->first.length()))){

					is_var = false;

				}

			}

			if((int)(j-1)>0){

				if(isalpha(_expression.at((int)(j-1)))){

					is_var = false;;

				}

			}

			if(is_var){

				_expression.replace(j, vIter->first.length(), dblToStr(vIter->second));

			}

			else{

				start_index = j+vIter->first.length();

			}

		}

	}

}



/*!

  @fn	void EXParser::prepareExpression(std::string &_expression)

  @brief	Prepares expression. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param [in,out]	_expression	The expression. 

*/



void EXParser::prepareExpression(std::string &_expression)

{

	std::transform(_expression.begin(), _expression.end(), _expression.begin(), tolower);

	_expression.erase(remove_if(_expression.begin(), _expression.end(), isspace), _expression.end());

}



/*!

  @fn	void EXParser::clearVariables(void)

  @brief	Clears the variables. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::clearVariables(void)

{

	variables.clear();

}



/*!

  @fn	void EXParser::clearFormulas(void)

  @brief	Clears the formulas. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::clearFormulas(void)

{

	formulas.clear();

}



/*!

  @fn	void EXParser::clearFunctions(void)

  @brief	Clears the functions. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::clearFunctions(void)

{

	functions.clear();

}



/*!

  @fn	void EXParser::clearOperators(void)

  @brief	Clears the operators. 

  @author	Justin Loundagin

  @date	5/16/2011

*/



void EXParser::clearOperators(void)

{

	operators.clear();

}



/*!

  @fn	bool EXParser::isParenthesisBalanced(const std::string &_expression)

  @brief	Query if '_expression' is parenthesis balanced. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @return	true if parenthesis balanced, false if not. 

*/



bool EXParser::isParenthesisBalanced(const std::string &_expression)

{

	int opened = 0, closed  = 0;

	for(std::string::const_iterator siter = _expression.begin(); siter!=_expression.end(); ++siter)

	{

		if(*siter == '('){

			opened++;

		}

		else if(*siter == ')'){

			closed++;

		}

	}

	return opened == closed;

}



/*!

  @fn	bool EXParser::isScientificNotation(const std::string &_expression, const int index)

  @brief	Query if '_expression' at 'index' is scientific notation. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	_expression	The expression. 

  @param	index		Zero-based index of the. 

  @return	true if scientific notation, false if not. 

*/



bool EXParser::isScientificNotation(const std::string &_expression, const int index){

	bool isScientific = false;

	if(index-1>=0){

		if(_expression[index-1] == 'e'){

			isScientific = true;

		}

	}

	return isScientific;

}



/*!

  @fn	bool EXParser::isNan(double a)

  @brief	Query if 'a' is nan. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	a. 

  @return	true if nan, false if not. 

*/



bool EXParser::isNan(double a)

{

	return a!=a;

}



/*!

  @fn	bool EXParser::isInf(double a)

  @brief	Query if 'a' is inf. 

  @author	Justin Loundagin

  @date	5/16/2011

  @param	a	double a

  @return	true if inf, false if not. 

*/



bool EXParser::isInf(double a)

{

	return !isNan(a)&&isNan(a-a);

}


