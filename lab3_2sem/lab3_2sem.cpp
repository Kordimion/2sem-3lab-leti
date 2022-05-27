#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

constexpr auto CHECK_INVERSE = "chkinv";
constexpr auto CHECK_DIRECT = "chkdir";
constexpr auto CALCULATE_INVERSE = "calcinv";
constexpr auto CALCULATE_DIRECT = "calcdir";
constexpr auto STANDARD_TO_INVERSE = "stdtoinv";
constexpr auto STANDARD_TO_DIRECT = "stdtodir";
constexpr auto ABOUT = "about";
constexpr auto HELP = "help";
constexpr auto EXIT = "exit";

inline void askFor(const char* item)
{
	std::cout << "\n" << item << " : ";
}

inline void printLine(const char* item)
{
	std::cout << "\n" << item;
}
void infoEndpoint()
{
	std::cout << "\nThis lab is about direct and inverse polish notations";
	std::cout << "\nThis is an example use case of stacks";
	std::cout << "\nUser should be able to check validity of math expressions in polish notations";
	std::cout << "\nUser should be able to calculate math expressions in polish notations";
	std::cout << "\nUser should be able to convert standard math expressions to direct and inverse polish notations";
	std::cout << "\nTo view all available operations, print " << HELP;
}

inline void printCommandDescription(const char* commandName, const char* desc)
{
	std::cout << "\n" << commandName << " - " << desc;
}

void helpEndpoint()
{
	std::cout << "\nCommands:";
	printCommandDescription(STANDARD_TO_DIRECT, "Convert standard math expression into direct polish notation [1+2] -> [+12]");
	printCommandDescription(CHECK_DIRECT, "Check validity of math expression in direct polish notation [+12]");
	printCommandDescription(CALCULATE_DIRECT, "Calculate math expression in direct polish notation [+12]");
	printCommandDescription(STANDARD_TO_INVERSE, "Convert standard math expression into inverse polish notation [1+2] -> [12+]");
	printCommandDescription(CHECK_INVERSE, "Check validity of math expression in inverse polish notation [12+]");
	printCommandDescription(CALCULATE_INVERSE, "Calculate math expression in inverse polish notation [12+]");
	printCommandDescription(ABOUT, "View info about this program");
	printCommandDescription(EXIT, "Stop program execution");
}

void exitEndpoint()
{
	std::cout << "\nProgram execution stopped";
}

enum class loggerMode
{
	verbose = 0,
	debug = 1,
	information = 2,
	warning = 3,
	error = 4
};

class Logger
{
private:
	loggerMode mode = loggerMode::verbose;
public:
	void setLoggerMode(const loggerMode newMode)
	{
		mode = newMode;
	}

	void verbose(const std::initializer_list<const char *> messages) const
	{
		if (mode > loggerMode::verbose) return;

		std::cout << "\n[VERBOSE] ";
		for (const auto message : messages) std::cout << message;
	}

	void debug(const std::initializer_list<const char *> messages) const
	{
		if (mode > loggerMode::debug) return;

		std::cout << "\n[DEBUG] ";
		for (const auto message : messages) std::cout << message;
	}

	void information(const std::initializer_list<const char *> messages) const
	{
		if (mode > loggerMode::information) return;

		std::cout << "\n[INFO] ";
		for (const auto message : messages) std::cout << message;
	}

	void warning(const std::initializer_list<const char *> messages) const
	{
		if (mode > loggerMode::warning) return;

		std::cout << "\n[WARNING] ";
		for (const auto message : messages) std::cout << message;
	}

	void error(const std::initializer_list<const char *> messages) const
	{
		if (mode > loggerMode::error) return;

		std::cout << "\n[ERROR] ";
		for (const auto message : messages) std::cout << message;
	}
};

Logger logger;

template <typename T>
struct Result
{
public:
	std::vector<std::string> errors;
	T res;
	bool isSuccess;

	static Result<T> error(std::vector<std::string> err)
	{
		Result<T> res;
		res.errors = err;
		res.isSuccess = false;
		return res;
	}

	static Result<T> success(T val)
	{
		Result<T> res;
		res.res = val;
		res.isSuccess = true;
		return res;
	}

private:
	Result()
	{
	}
};

template <typename T>
struct Node
{
	T val;
	Node<T>* ptr;

	Node(T val, Node<T>* ptr) : val(std::move(val)), ptr(ptr)
	{
	}
};

template <typename T>
class Stack
{
private:
	Node<T>* head = nullptr;
public:
	void push(const T& elem)
	{
		auto* newNode = new Node<T>(elem, head);
		head = newNode;
	}

	bool empty() const
	{
		return head == nullptr;
	}

	T top() const
	{
		if (head == nullptr) throw std::out_of_range("can't view element of an empty stack");
		return head->val;
	}

	void pop()
	{
		if (head == nullptr) throw std::out_of_range("can't pop from empty stack");

		Node<T>* prevHead = head->ptr;

		delete head;
		head = prevHead;
	}

	std::string toString() const
	{
		std::stringstream ss;
		Node<T>* ptr = head;
		if (ptr != nullptr)
		{
			ss << ptr->val;
			ptr = ptr->ptr;
			while(ptr != nullptr)
			{
				ss << " " << ptr->val;
				ptr = ptr->ptr;
			}
		}
		return ss.str();
	}
};

std::map <std::string, int> weight = { {"+", 1 },
							 {"-", 1 },
							 {"*", 2 },
							 {"/", 2 },
							 {"(", 0 },
							 {")", 0 } };

bool isOperator(const char ch)
{
	const auto iter = weight.find(std::string() + ch);
	return iter != weight.end();
}

int operatorWeight(std::string op)
{
	const auto iter = weight.find(op);
	if (iter != weight.end()) return iter->second;
	return -1;
}

inline bool isInteger(const std::string& s)
{
	if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

	char* p;
	strtol(s.c_str(), &p, 10);

	return (*p == 0);
}

std::string viewTokens(const std::vector<std::string>& tokens)
{
	std::stringstream ss;
	ss << "[|";
	std::for_each(tokens.begin(), tokens.end(), [&ss](const std::string& s) { ss << s << "|"; });
	ss << "]";
	return ss.str();
}

std::vector<std::string> tokenize(const std::string& str)
{
	logger.verbose({"Tokenization started. Received string: ", str.c_str()});
	std::vector<std::string> res;
	bool foundForeignSymbol = true;
	for (const char ch : str)
	{
		const bool currentSymbolIsForeign = !(std::isdigit(ch) || std::isalpha(ch) || isOperator(ch));
		if(foundForeignSymbol)
			res.push_back(std::string() + ch);
		else
			if(!currentSymbolIsForeign) *res.rbegin() += ch;

		foundForeignSymbol = currentSymbolIsForeign;
	}
	logger.verbose({"Tokenization completed. Returned tokens: ", viewTokens(res).c_str()});
	return res;
}

void replaceVariables(std::vector<std::string>& tokens)
{
	logger.verbose({"Variable replacement started. Received tokens: ", viewTokens(tokens).c_str()});
	for (int i = 0; i < tokens.size(); ++i)
	{
		std::string& token = tokens[i];
		if(std::isalpha(token[0]))
		{
			logger.information({ "Found variable (", tokens[i].c_str(),")"});
			askFor(token.c_str());
			int tokenValue;
			std::cin >> tokenValue;

			int variableOccurrences = 1;
			for(int j = i+1; j < tokens.size(); ++j)
			{
				std::string& nextToken = tokens[j];

				if (nextToken == token) {
					nextToken = std::to_string(tokenValue);
					++variableOccurrences;
				}
			}
			logger.information({ "Replaced ", 
				std::to_string(variableOccurrences).c_str(),
				" occurrences of variable (",
				token.c_str(),
				") with value ",
				std::to_string(tokenValue).c_str()
			});
			token = std::to_string(tokenValue);
		}
	}
	logger.verbose({"Variable replacement completed. Returned tokens: ", viewTokens(tokens).c_str()});
}


Result<int> calculateDirect(const std::vector<std::string>& tokens)
{
	Stack<int> stack;
	for (int i = tokens.size() - 1; i >= 0; --i)
	{
		const std::string& token = tokens[i];
		if (isInteger(token))
		{
			logger.information({ "Processing number: ", token.c_str() });
			stack.push(stoi(token));
		}
		else if (operatorWeight(token) > 0)
		{
			if (token == "+" || token == "-" || token == "*" || token == "/")
			{
				if (stack.empty()) return Result<int>::error({ "Not enough operands in expression" });
				const int val2 = stack.top();
				const std::string val2String = std::to_string(val2);
				stack.pop();
				if (stack.empty()) return Result<int>::error({ "Not enough operands in expression" });
				const int val1 = stack.top();
				const std::string val1String = std::to_string(val1);
				stack.pop();

				int res;

				if (token == "+") res = val1 + val2;
				if (token == "-") res = val1 - val2;
				if (token == "*") res = val1 * val2;
				if (token == "/") {
					if (val2 != 0) res = val1 / val2;
					else return Result<int>::error({ "Encountered division by zero" });
				}

				const std::string resString = std::to_string(res);

				logger.information({ "Values : ", val2String.c_str(), " ", val1String.c_str(), " were popped from the stack" });
				logger.information({ "Performed calculation: ", val1String.c_str(), " ", token.c_str(), " ", val2String.c_str(), " = ", resString.c_str() });
				logger.information({ "Value ", resString.c_str(), " was pushed to the stack" });
				stack.push(res);
			}
		}
		else
		{
			const std::string errorMessage = std::string("Received unexpected token: ") + token;
			logger.error({ errorMessage.c_str() });
			return Result<int>::error({ errorMessage });
		}
		logger.debug({ "Encountered token: ", token.c_str() });
		logger.debug({ "Current stack: ", stack.toString().c_str() });
	}
	if (stack.empty())
	{
		const char* errorMessage = "Not enough operands in expression";
		logger.error({ errorMessage });
		return Result<int>::error({ errorMessage });
	}
	const int res = stack.top();
	stack.pop();
	if (!stack.empty())
	{
		const char* errorMessage = "Not enough operators in expression";
		logger.error({ errorMessage });
		return Result<int>::error({ errorMessage });

	}
	return Result<int>::success(res);
}

Result<int> calculateInverse(const std::vector<std::string>& tokens, const bool ignoreVariables = false)
{
	Stack<int> stack;
	for(const std::string& token : tokens)
	{
		if (isInteger(token))
			stack.push(stoi(token));
		else if (operatorWeight(token) > 0)
		{
			if(token == "+" || token == "-" || token == "*" || token == "/")
			{
				if (stack.empty()) return Result<int>::error({ "Not enough operands in expression" });
				const int val2 = stack.top();
				const std::string val2String = std::to_string(val2);
				stack.pop();
				if (stack.empty()) return Result<int>::error({ "Not enough operands in expression" });
				const int val1 = stack.top();
				const std::string val1String = std::to_string(val1);
				stack.pop();

				int res;

				if (token == "+") res = val1 + val2;
				if (token == "-") res = val1 - val2;
				if (token == "*") res = val1 * val2;
				if (token == "/") {
					if (val2 != 0) res = val1 / val2;
					else return Result<int>::error({ "Encountered division by zero" });
				}

				const std::string resString = std::to_string(res);

				logger.information({ "Calculation: ", val1String.c_str(), " ", token.c_str(), " ", val2String.c_str(), " = ", resString.c_str()});
				stack.push(res);
			}
		}
		else if (!ignoreVariables)
		{
			const std::string errorMessage = std::string("Received unexpected token: ") + token;
			logger.error({ errorMessage.c_str() });
			return Result<int>::error({ errorMessage });
		}
		logger.debug({ "Token: ", token.c_str() });
		logger.debug({ "Stack: ", stack.toString().c_str() });
	}
	if(stack.empty())
	{
		const char* errorMessage = "Not enough operands in expression";
		logger.error({ errorMessage });
		return Result<int>::error({ errorMessage });
	}
	const int res = stack.top();
	stack.pop();
	if(!stack.empty())
	{
		const char* errorMessage = "Not enough operators in expression";
		logger.error({ errorMessage });
		return Result<int>::error({ errorMessage });

	}
	return Result<int>::success(res);
}

void printErrorMessage(std::vector<std::string> messages)
{
	std::cout << "\n\nCould not calculate direct polish notation.";
	std::cout << "\nErrors: ";
	for(int i = 0; i < messages.size(); ++i)
		std::cout << "\n" << i + 1 << ") " << messages[i];
}

void checkDirectEndpoint()
{
	std::string expr;
	askFor("direct polish notation expression to validate");
	std::getline(std::cin, expr);

	std::vector<std::string> tokens = tokenize(expr);
	replaceVariables(tokens);

	const Result<int> res = calculateDirect(tokens);

	if (res.isSuccess) std::cout << "\n\nOperation valid" << res.res;
	else printErrorMessage(res.errors);
}
void checkInverseEndpoint()
{
	std::string expr;
	askFor("inverse polish notation expression to validate");
	std::getline(std::cin, expr);

	std::vector<std::string> tokens = tokenize(expr);
	replaceVariables(tokens);

	const Result<int> res = calculateInverse(tokens, true);

	if (res.isSuccess) std::cout << "\n\nOperation valid" << res.res;
	else printErrorMessage(res.errors);
}

void calculateDirectEndpoint()
{
	std::string expr;
	askFor("inverse polish notation expression to calculate");
	std::getline(std::cin, expr);

	std::vector<std::string> tokens = tokenize(expr);
	replaceVariables(tokens);

	const Result<int> res = calculateDirect(tokens);

	if (res.isSuccess) std::cout << "\n\nResult: " << res.res;
	else printErrorMessage(res.errors);
}

void calculateInverseEndpoint()
{
	std::string expr;
	askFor("inverse polish notation expression to calculate");
	std::getline(std::cin, expr);

	std::vector<std::string> tokens = tokenize(expr);
	replaceVariables(tokens);

	const Result<int> res = calculateInverse(tokens);

	if (res.isSuccess) std::cout << "\n\nResult: " << res.res;
	else printErrorMessage(res.errors);
}

std::string convertStandardToDirect(const std::vector<std::string>& tokens)
{
	Stack<std::string> resStack;
	Stack<std::string> opStack;

	logger.verbose({"Conversion [standard notation -> direct polish notation] started"});
	
	for(auto token : tokens)
	{
		const int opWeight = operatorWeight(token);
		if (token == "(")
		{
			logger.information({ "Found opening bracket. Pushing to operation stack" });
			opStack.push(token);
		}
		else if (isInteger(token) || isalpha(token[0]))
		{
			logger.information({"Found number/variable(",token.c_str(),"). Pushing to resulting stack"});
			resStack.push(token);
		}
		if (token == ")")
		{
			logger.information({ "Found closing bracket. Pushing operation stack to resulting stack until opening bracket is found" });
			while(!opStack.empty() && opStack.top() != "(")
			{
				resStack.push(opStack.top());
				opStack.pop();
			}
			if (opStack.empty())
			{
				logger.error({ "Opening bracket not found" });
				return "Opening bracket not found";
			} 
			opStack.pop();
		}
		else if(opWeight > 0)
		{
			const char* tokenStr = token.c_str();
			std::string opWeightTempStr = std::to_string(operatorWeight(token));
			const char* opWeightStr = opWeightTempStr.c_str();
			logger.information({"Found operator ", tokenStr, " with weight ", opWeightStr});

			while(!opStack.empty() && operatorWeight(opStack.top()) >= opWeight)
			{
				const std::string& stackTopVar = opStack.top();
				const char* stackTokenStr = stackTopVar.c_str();
				std::string stackOpWeightTempStr = std::to_string(operatorWeight(stackTopVar));
				const char* stackOpWeightStr = stackOpWeightTempStr.c_str();
				logger.information({"Stack operator[",stackTokenStr,"] weight(",stackOpWeightStr,
					") >= found operator[", tokenStr, "] weight(", opWeightStr,
					"). Pushing ", stackTokenStr, " to resulting stack."});
				resStack.push(stackTopVar);
				opStack.pop();
			}

			logger.information({"Pushing operator ", tokenStr, " with weight ", opWeightStr, " into operation stack"});
			opStack.push(token);
		}
		logger.debug({ "Operation stack: ", opStack.toString().c_str() });
		logger.debug({ "Resulting stack: ", resStack.toString().c_str() });
	}
	logger.information({"Pushing everything from stack into resulting expression"});
	while(!opStack.empty())
	{
		resStack.push(opStack.top());
		opStack.pop();
	}
	logger.debug({ "Operation stack: ", opStack.toString().c_str() });
	logger.debug({ "Resulting stack: ", resStack.toString().c_str() });

	const auto res = resStack.toString();

	logger.verbose({"Conversion [standard notation -> direct polish notation] is completed"});
	return resStack.toString();
}

void standardToDirectEndpoint()
{
	std::string expr;
	askFor("standard expression to convert to direct polish notation");
	std::getline(std::cin, expr);
	
	const std::vector<std::string> tokens = tokenize(expr);

	std::cout << "\n\nResulting expression: " << convertStandardToDirect(tokens);
}

std::string convertStandardToInverse(const std::vector<std::string>& tokens)
{
	std::string expr;
	Stack<std::string> stack;
	logger.verbose({"Conversion [standard notation -> inverse polish notation] started"});
	
	for(auto token : tokens)
	{
		const int opWeight = operatorWeight(token);
		if (token == "(")
		{
			logger.information({ "Found opening bracket. Pushing to stack" });
			stack.push(token);
		}
		else if (isInteger(token) || isalpha(token[0]))
		{
			logger.information({"Found number/variable(",token.c_str(),"). Pushing to resulting string"});
			expr += token + " ";
		}
		if (token == ")")
		{
			logger.information({ "Found closing bracket. Pushing stack to resulting string until opening bracket is found" });
			while(!stack.empty() && stack.top() != "(")
			{
				expr += stack.top();
				stack.pop();
			}
			if (stack.empty())
			{
				logger.error({ "Opening bracket not found" });
				return "Opening bracket not found";
			} 
			stack.pop();
		}
		else if(opWeight > 0)
		{
			const char* tokenStr = token.c_str();
			std::string opWeightTempStr = std::to_string(operatorWeight(token));
			const char* opWeightStr = opWeightTempStr.c_str();
			logger.information({"Found operator ", tokenStr, " with weight ", opWeightStr});

			while(!stack.empty() && operatorWeight(stack.top()) >= opWeight)
			{
				const std::string& stackTopVar = stack.top();
				const char* stackTokenStr = stackTopVar.c_str();
				std::string stackOpWeightTempStr = std::to_string(operatorWeight(stackTopVar));
				const char* stackOpWeightStr = stackOpWeightTempStr.c_str();
				logger.information({"Stack operator[",stackTokenStr,"] weight(",stackOpWeightStr,
					") >= found operator[", tokenStr, "] weight(", opWeightStr,
					"). Pushing ", stackTokenStr, " to resulting expression."});
				expr += stack.top() + ' ';
				stack.pop();
			}

			logger.information({"Pushing operator ", tokenStr, " with weight ", opWeightStr, " into stack"});
			stack.push(token);
		}

		logger.debug({"Resulting string: ", expr.c_str()});
		logger.debug({"Stack: ", stack.toString().c_str()});
	}
	logger.information({"Pushing everything from stack into resulting expression"});
	while(!stack.empty())
	{
		expr += stack.top() + ' ';
		stack.pop();
	}
	logger.debug({"Resulting string: ", expr.c_str()});
	logger.debug({"Stack: ", stack.toString().c_str()});
	logger.verbose({"Conversion [standard notation -> inverse polish notation] is completed"});

	return expr;
}

void standardToInverseEndpoint()
{
	std::string str;
	askFor("standard expression to convert to inverse polish notation");
	std::getline(std::cin, str);

	const std::vector<std::string> tokens = tokenize(str);

	std::cout << "\n\nResulting expression: " << convertStandardToInverse(tokens);
}

void processEndpoint(const char* endpoint)
{
	if (strcmp(endpoint, CHECK_DIRECT) == 0) return checkDirectEndpoint();
	if (strcmp(endpoint, CHECK_INVERSE) == 0) return checkInverseEndpoint();
	if (strcmp(endpoint, CALCULATE_DIRECT) == 0) return calculateDirectEndpoint();
	if (strcmp(endpoint, CALCULATE_INVERSE) == 0) return calculateInverseEndpoint();
	if (strcmp(endpoint, STANDARD_TO_DIRECT) == 0) return standardToDirectEndpoint();
	if (strcmp(endpoint, STANDARD_TO_INVERSE) == 0) return standardToInverseEndpoint();
	if (strcmp(endpoint, HELP) == 0) return helpEndpoint();
	if (strcmp(endpoint, ABOUT) == 0) return infoEndpoint();
	if (strcmp(endpoint, EXIT) == 0) return exitEndpoint();

	std::cout << "\nCommand not found";
	std::cout << "\nPrint help to view list of all commands\n";
}

int main()
{
	std::string endpoint;
	infoEndpoint();
	bool shouldAskForYourCommand = true;
	while (endpoint != EXIT)
	{
		if (shouldAskForYourCommand) askFor("\nYour Command");
		std::getline(std::cin, endpoint);
		shouldAskForYourCommand = true;

		if (!endpoint.empty())
			processEndpoint(endpoint.c_str());
		else shouldAskForYourCommand = false;
	}
}
