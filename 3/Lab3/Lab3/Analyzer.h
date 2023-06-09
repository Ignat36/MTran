#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

class PyAnalyzer
{

private:

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

private:

	struct Error
	{
		std::string Message;

		Error(std::string msg)
			: Message(msg)
		{}
	};

	struct FVariable
	{
		std::string Type;
		std::string Name;
		int Scope;
		void* Value;

		FVariable(std::string _Name, std::string _Type, void* _Value)
			: Name(_Name), Type(_Type), Value(_Value), Scope(-1) {}

		FVariable() : Type(""), Scope(-2) {}

		int Print(bool doNewLine = true);
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

	//---------------------------------------------------------
	//----------------SemanticAnalisis-------------------------
	//---------------------------------------------------------
	int SemanticCheck(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index, int scope = 0);
	int GetNumberType(const std::string& val);

	//---------------------------------------------------------
	//----------------SemanticAnalisis-------------------------
	//---------------------------------------------------------
	int Execute();
	int ExecScope(std::shared_ptr<SyntaxNode> Scope, int scope_id);
	int ExecFor(std::shared_ptr<SyntaxNode> Scope, int scope_id);
	int ExecWhile(std::shared_ptr<SyntaxNode> Scope, int scope_id);

	FVariable ExecExpr(std::shared_ptr<SyntaxNode> Node);
	FVariable ExecFunction(std::shared_ptr<SyntaxNode> Node);
	FVariable ExecOperation(std::string op, FVariable l, FVariable r);
	FVariable GetNumFromString(const std::string& val);


	//---------------------------------------------------------
	//----------------FunctionsCall----------------------------
	//---------------------------------------------------------
	FVariable CallPrint(std::shared_ptr<SyntaxNode> Node);
	FVariable CallInt(std::shared_ptr<SyntaxNode> Node);
	FVariable CallInput(std::shared_ptr<SyntaxNode> Node);
	FVariable CallFloat(std::shared_ptr<SyntaxNode> Node);
	FVariable CallArray(std::shared_ptr<SyntaxNode> Node);
	FVariable CallArrayAppend(std::shared_ptr<SyntaxNode> Node);
	FVariable CallArrayPopBack(std::shared_ptr<SyntaxNode> Node);
	FVariable CallArraySize(std::shared_ptr<SyntaxNode> Node);
	FVariable CallType(std::shared_ptr<SyntaxNode> Node);
	FVariable CallString(std::shared_ptr<SyntaxNode> Node);
	FVariable CallComplex(std::shared_ptr<SyntaxNode> Node);
	std::pair<int, int> ExecRange(std::shared_ptr<SyntaxNode> Node);

private:

	std::unordered_set<std::string> TokenVariables;
	std::unordered_set<std::string> Functions;
	std::unordered_map<std::string, FVariable> Vars;

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
		"else", "except", "finally", "for", "from", "global", "if", "import",
		"lambda", "None", "nonlocal", "pass", "raise",
		"return", "try", "while", "with", "yield"
	};

	const std::unordered_set<std::string> BuiltinFunctions = {
		"[]", "abs", "all", "any", "ascii", "bin", "bool", "bytearray", "bytes", "callable",
		"chr", "classmethod", "compile", "complex", "delattr", "dict", "dir", "divmod",
		"enumerate", "eval", "exec", "filter", "float", "format", "frozenset", "getattr",
		"globals", "hasattr", "hash", "help", "hex", "id", "input", "int", "isinstance",
		"issubclass", "iter", "len", "list", "locals", "map", "max", "memoryview", "min",
		"next", "object", "oct", "open", "ord", "pow", "print", "property", "range",
		"repr", "reversed", "round", "set", "setattr", "slice", "sorted", "staticmethod",
		"string", "sum", "super", "tuple", "type", "vars", "zip", "array", "append", "pop_back"
	};

	const std::unordered_set<std::string> BuiltinTypes = {
		"bytearray", "bytes", "complex", "dict", "float", "frozenset", "int",
		"list", "set", "string", "tuple", "array"
	};

	const std::unordered_set<std::string> BuiltinNumeric = {
		"complex",  "float", "int"
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

	const std::unordered_set<std::string> ComparisonOperators = {
	"==", "!=", "<", "<=", ">", ">=",     // Comparison Operators
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

