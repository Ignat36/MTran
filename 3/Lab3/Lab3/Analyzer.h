#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

class PyAnalyzer
{

public:

	enum class ETokenType
	{
		Literal,
		Number,
		Variable,
		Function,
		KeyWord,
		Operator,
		Type,
		Delimeter
	};

public:

	struct Error
	{
		const std::string Message;

		Error(std::string msg)
			: Message(msg)
		{}
	};

	struct FToken
	{
		std::string ValueName;
		std::string Description;
		int RowIndex;
		int ColumnIndex;
		ETokenType TokenType;

		FToken(std::string _ValueName, std::string _Description, int _RowIndex, int _ColumnIndex)
			: ValueName(_ValueName), Description(_Description), RowIndex(_RowIndex), ColumnIndex(_ColumnIndex)
		{}

		FToken(std::string _ValueName, std::string _Description, int _RowIndex, int _ColumnIndex, ETokenType _TokenType)
			: ValueName(_ValueName), Description(_Description), RowIndex(_RowIndex), ColumnIndex(_ColumnIndex), TokenType(_TokenType)
		{}
	};

	struct Table
	{
		std::vector<FToken> Rows;
	};

private:

	struct SyntaxNode
	{
		FToken Token;
		std::vector< std::shared_ptr<SyntaxNode> > Children;
		std::weak_ptr<SyntaxNode> Parent;

		SyntaxNode(FToken _Token, std::shared_ptr<SyntaxNode> _Parent = nullptr) : Token(_Token), Parent(_Parent) {}

		void Print(int Depth = 0);
	};

public:

	PyAnalyzer(std::vector<std::string>& _Code);
	const std::vector<Error>& GetErrors() { return Errors; }
	const std::vector<FToken>& GetTokens() { return Tokens; }
	void PrintSyntaxTree();

protected:

	std::vector<std::string> Code;

	Table VariablesTable;
	Table FunctionsTable;
	Table KeywordsTable;
	Table OperatorsTable;
	Table ConstantsTable;

	std::vector<FToken> Tokens;

	std::vector<int> CodeDeepth;

	std::vector<Error> Errors;

	std::shared_ptr<SyntaxNode> SyntaxTree;

private:

	int Analyze();
	int LexicalAnalisis();
	int SyntaxAnalisis();
	int SemanticAnalisis();

	//---------------------------------------------------------
	//----------------LexicalAnalisis--------------------------
	//---------------------------------------------------------
	std::string ReadNumberConstant(int& x, int& y, bool& Flag);
	std::string ReadOperator(int& x, int& y, bool& Flag);
	std::string ReadWord(int& x, int& y, bool& Flag);
	std::string ReadLiteral(int& x, int& y, bool& Flag);
	int ReadFunctionSignature(int& x, int& y, bool& Flag);
	int ReadForSignature(int& x, int& y, bool& Flag);

	//---------------------------------------------------------
	//----------------SyntaxAnalisis---------------------------
	//---------------------------------------------------------
	int ProcessExpression(int& x, std::shared_ptr<SyntaxNode>& Node);
	int ProcessKeyWord(int& x, std::shared_ptr<SyntaxNode>& Node);

	int ReformatSyntaxTree();
	int ReformatSyntaxNode(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index);

	int checkSingleTokenDependensies();
	int checkBrackets();
	int checkSyntaxTree();
	int checkSyntaxTree(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index);
	int checkBrakContinueNode(std::shared_ptr<SyntaxNode> Node);
	int checkFunctionNode(std::shared_ptr<SyntaxNode> Node);
	int checkFirstLineWordNode(std::shared_ptr<SyntaxNode> Node);

	std::shared_ptr<SyntaxNode> BuildExpressionTree(std::shared_ptr<SyntaxNode> Node);

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
		"as", "assert", "break", "class", "continue", "def", "del", "elif",
		"else", "except", "False", "finally", "for", "from", "global", "if", "import",
		"lambda", "None", "nonlocal", "pass", "raise",
		"return", "True", "try", "while", "with", "yield"
	};

	const std::unordered_set<std::string> BuiltinFunctions = {
		"[]", "abs", "all", "any", "ascii", "bin", "bool", "bytearray", "bytes", "callable",
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

	const std::unordered_set<std::string> GoodTokensAfterLiteral = {
		"]", ",", ")", "}", ":", "+", "*", "==", "!=", "<", "<=", ">", ">=", "is", "not", "and", "or", "in"
	};

	const std::unordered_set<std::string> GoodTokensAfterNumber = {
		"]", ",", ")", "}", ":", "+", "-", "*", "/", "//", "%", "**", "==", "!=", "<", "<=", ">", ">=", "is", "not", "and", "or", "in"
	};

	const std::unordered_set<std::string> GoodTokensAfterVariable = {
		"[", "]", ",", ")", "}", ":", "+", "-", "*", "/", "//", "%", "**", "==", "!=", "<", "<=", ">", ">=", "is", "not", "and", "or",
		"&", "|", "^", "<<", ">>", "~", "=", "+=", "-=", "*=", "/=", "//=", "%=", "**=", "&=", "|=", "^=", "<<=", ">>=", "in"
	};

	const std::unordered_set<char> Delimiters = { '[', ']', '(', ')', '{', '}', ' ', ':', ',', '\t' };

	const std::unordered_set<std::string> Brackets = { "[", "]", "(", ")", "{", "}"};

	std::unordered_set<char> AllowedEscapeChars = { '\\', '\'', '\"', 'n', 'r', 't', 'b', 'f', 'o', 'x' };

	const std::unordered_set<char> OperatorsCharacters = { '=', '>', '<', '|', '+', '-', '*', '/', '%', '&', '^', '~', '!' };

	const std::unordered_set<std::string> Operators = {
	"+", "-", "*", "/", "//", "%", "**",  // Arithmetic Operators
	"==", "!=", "<", "<=", ">", ">=",     // Comparison Operators
	"and", "or", "not", "in", "is"        // Logical Operators
	"&", "|", "^", "<<", ">>", "~",       // Bitwise Operators
	"=", "+=", "-=", "*=", "/=",          // Assignment Operators
	"//=", "%=", "**=", "&=", "|=", "^=", // Assignment Operators
	"<<=", ">>="                          // Assignment Operators
	};

	const std::unordered_set<std::string> AssigmentOperators = {
	"=", "+=", "-=", "*=", "/=",          // Assignment Operators
	"//=", "%=", "**=", "&=", "|=", "^=", // Assignment Operators
	"<<=", ">>="                          // Assignment Operators
	};

	std::unordered_map<std::string, int> OperatorPrecedence{
		{"=",0},
		{"+=",0},
		{"-=",0},
		{"*=",0},
		{"/=",0},
		{"//=",0},
		{"%=",0},
		{"**=",0},
		{"&=",0},
		{"|=",0},
		{"^=",0},
		{"or", 1},
		{"and", 2},
		{"not", 3},
		{"in", 4},
		{"not in", 4},
		{"is", 4},
		{"is not", 4},
		{"<", 5},
		{">", 5},
		{"<=", 5},
		{">=", 5},
		{"!=", 5},
		{"==", 5},
		{"+", 6},
		{"-", 6},
		{"*", 7},
		{"/", 7},
		{"//", 7},
		{"%", 7},
		{"**", 8}
	};

};

