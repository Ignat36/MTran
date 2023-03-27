#pragma once

#include <string>
#include <vector>
#include <unordered_set>

class LexicalAnalizer
{
public:

	struct Error
	{
		const std::string Message;

		Error(std::string msg) 
			: Message(msg) 
		{}
	};

	struct Row
	{
		std::string ValueName;
		std::string Description;
		int RowIndex;
		int ColumnIndex;

		Row(std::string _ValueName, std::string _Description, int _RowIndex, int _ColumnIndex) 
			: ValueName(_ValueName), Description(_Description), RowIndex(_RowIndex), ColumnIndex(_ColumnIndex) 
		{}
	};

	struct Table
	{
		std::vector<Row> Rows;
	};

public:

	LexicalAnalizer(std::vector<std::string>& _Code);
	const std::vector<Error>& GetErrors() { return Errors; }
	const std::vector<Row>& GetTokens() { return Tokens; }

protected:

	std::vector<std::string> Code;

	Table VariablesTable;
	Table FunctionsTable;
	Table KeywordsTable;
	Table OperatorsTable;
	Table ConstantsTable;

	std::vector<Row> Tokens;

	std::vector<Error> Errors;

private:

	bool Analyze();

	std::string ReadNumberConstant(int& x, int& y, bool& Flag);
	std::string ReadOperator(int& x, int& y, bool& Flag);
	std::string ReadWord(int& x, int& y, bool& Flag);
	std::string ReadLiteral(int& x, int& y, bool& Flag);
	void ReadFunctionSignature(int& x, int& y, bool& Flag);
	void ReadForSignature(int& x, int& y, bool& Flag);


private:

	std::unordered_set<std::string> Variables;
	std::unordered_set<std::string> Functions;

	const std::unordered_set<char> AllowedCharacters = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '+', '-',
	'*', '/', '%', '&', '|', '^', '~', '<', '>', '=', '!', '.', ',',
	':', '(', ')', '[', ']', '{', '}'
	};

	const std::unordered_set<std::string> Keywords = {
		"and", "as", "assert", "break", "class", "continue", "def", "del", "elif",
		"else", "except", "False", "finally", "for", "from", "global", "if", "import",
		"in", "is", "lambda", "None", "nonlocal", "not", "or", "pass", "raise",
		"return", "True", "try", "while", "with", "yield"
	};

	const std::unordered_set<std::string> BuiltinFunctions = {
		"abs", "all", "any", "ascii", "bin", "bool", "bytearray", "bytes", "callable",
		"chr", "classmethod", "compile", "complex", "delattr", "dict", "dir", "divmod",
		"enumerate", "eval", "exec", "filter", "float", "format", "frozenset", "getattr",
		"globals", "hasattr", "hash", "help", "hex", "id", "input", "int", "isinstance",
		"issubclass", "iter", "len", "list", "locals", "map", "max", "memoryview", "min",
		"next", "object", "oct", "open", "ord", "pow", "print", "property", "range",
		"repr", "reversed", "round", "set", "setattr", "slice", "sorted", "staticmethod",
		"str", "sum", "super", "tuple", "type", "vars", "zip"
	};

	const std::unordered_set<std::string> BuiltinTypes = {
		"bool", "bytearray", "bytes", "complex", "dict", "float", "frozenset", "int",
		"list", "set", "str", "tuple"
	};

	const std::unordered_set<char> Delimiters = {'[', ']', '(', ')', '{', '}', ' ', ':', ',', '\t'};

	std::unordered_set<char> AllowedEscapeChars = { '\\', '\'', '\"', 'n', 'r', 't', 'b', 'f', 'o', 'x' };

	const std::unordered_set<char> OperatorsCharacters = { '=', '>', '<', '|', '+', '-', '*', '/', '%', '&', '^', '~', '!'};

	const std::unordered_set<std::string> Operators = {
	"+", "-", "*", "/", "//", "%", "**",  // Arithmetic Operators
	"==", "!=", "<", "<=", ">", ">=",     // Comparison Operators
	"and", "or", "not",                   // Logical Operators
	"&", "|", "^", "<<", ">>", "~",       // Bitwise Operators
	"=", "+=", "-=", "*=", "/=",          // Assignment Operators
	"//=", "%=", "**=", "&=", "|=", "^=", // Assignment Operators
	"<<=", ">>="                          // Assignment Operators
	};
};

