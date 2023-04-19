#include "Analyzer.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <complex>

PyAnalyzer::PyAnalyzer(std::vector<std::string>& _Code) : Code(_Code)
{
    Analyze();
}

void PyAnalyzer::PrintSyntaxTree()
{
    SyntaxTree->Print();
}

int PyAnalyzer::Analyze()
{
    LexicalAnalisis(); if (Errors.size()) return 1;
    checkSingleTokenDependensies(); if (Errors.size()) return 1;
    checkBrackets(); if (Errors.size()) return 1;
    SyntaxAnalisis(); if (Errors.size()) return 1;
    ReformatSyntaxTree(); if (Errors.size()) return 1;
    checkSyntaxTree(); if (Errors.size()) return 1;
    SemanticAnalisis(); if (Errors.size()) return 1;
    Execute(); if (Errors.size()) return 1;
    return 0;;
}

int PyAnalyzer::LexicalAnalisis()
{
    std::string Token = "";
    CodeDeepth.resize(Code.size());
    for (int i = 0; i < Code.size(); i++)
    {
        bool OnlySpaces = true;
        bool MakeFalse = false;

        for (int j = 0; i < Code.size() && j < Code[i].size(); j++)
        {
            bool ReturnFlag = false;
            Token.clear();
            if (MakeFalse) OnlySpaces = false;

            char c = Code[i][j];
            if (c == ' ' || c == '\t')
            {
                if (!MakeFalse)
                {
                    CodeDeepth[i] += c == ' ' ? 1 : 4;
                }
                continue;
            }
            MakeFalse = true;

            if (Delimiters.count(c))
            {
                Token = c;
                Tokens.push_back(FToken(Token, "delimeter", i, j, ETokenType::Delimeter));
                continue;
            }
            if ((c >= '0' && c <= '9') || c == '.' 
                || (j + 1 < Code[i].size() && (Code[i][j] == '+' || Code[i][j] == '-') 
                    && Code[i][j + 1] >= '0' 
                    && Code[i][j + 1] <= '9' 
                    && Tokens.back().ValueName != ")" 
                    && Tokens.back().ValueName != "]" 
                    && Tokens.back().TokenType != ETokenType::Number
                    && Tokens.back().TokenType != ETokenType::Variable
                    && Tokens.back().TokenType != ETokenType::Literal
                    ))
            {
                Token = ReadNumberConstant(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(FToken(Token, "constant", i, j, ETokenType::Number));
                continue;
            }
            if (c == '\"' || c == '\'')
            {
                Token = ReadLiteral(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(FToken(Token, "literal", i, j, ETokenType::Literal));
                continue;
            }
            if (OperatorsCharacters.count(c))
            {
                Token = ReadOperator(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(FToken(Token, "operator", i, j, ETokenType::Operator));
                continue;
            }


            Token = ReadWord(i, j, ReturnFlag);
            if (ReturnFlag) continue;


            if (Operators.count(Token))
            {
                OperatorsTable.Rows.push_back(FToken(Token, "operator", i, j));
                Tokens.push_back(FToken(Token, "operator", i, j, ETokenType::Operator));
                continue;
            }
            if (Keywords.count(Token))
            {
                KeywordsTable.Rows.push_back(FToken(Token, "key word", i, j));
                Tokens.push_back(FToken(Token, "key word", i, j, ETokenType::KeyWord));

                if (Token == "def")
                {
                    if (OnlySpaces)
                    {
                        j++;
                        ReadFunctionSignature(i, j, ReturnFlag);
                    }
                    else
                    {
                        Errors.push_back(Error(std::string("Incorrect use of |def| keyword : ") + std::to_string(i) + ":" + std::to_string(j)));
                    }
                }
                else if (Token == "for")
                {
                    j++;
                    ReadForSignature(i, j, ReturnFlag);
                }

                continue;
            }
            if (BuiltinFunctions.count(Token))
            {
                FunctionsTable.Rows.push_back(FToken(Token, "build-in function", i, j));
                Tokens.push_back(FToken(Token, "build-in function", i, j, ETokenType::Function));
                continue;
            }
            if (TokenVariables.count(Token))
            {
                VariablesTable.Rows.push_back(FToken(Token, "variable", i, j));
                Tokens.push_back(FToken(Token, "variable", i, j, ETokenType::Variable));
                continue;
            }
            if (Functions.count(Token))
            {
                OperatorsTable.Rows.push_back(FToken(Token, "function", i, j));
                Tokens.push_back(FToken(Token, "function", i, j, ETokenType::Function));
                continue;
            }

            if (OnlySpaces == false)
            {
                Errors.push_back(Error("Unknown token : " + Token + " | " + std::to_string(i) + ":" + std::to_string(j)));
                continue;
            }

            if (j + 1 < Code[i].size() && Code[i][j + 1] == '(')
            {
                Errors.push_back(Error("Unknown function name : " + Token + " | " + std::to_string(i) + ":" + std::to_string(j)));
                continue;
            }

            int jj = j + 1;
            while (jj < Code[i].size() && Code[i][jj] == ' ') jj++;

            if (jj >= Code[i].size() || Code[i][jj] != '=')
            {
                Errors.push_back(Error("Unknown token : " + Token + " | " + std::to_string(i) + ":" + std::to_string(j)));
                continue;
            }
            else
            {
                TokenVariables.insert(Token);
                Tokens.push_back(FToken(Token, "variable", i, j, ETokenType::Variable));
                continue;
            }
        }
    }

    for (int i = 0; i < CodeDeepth.size(); i++)
    {
        if (CodeDeepth[i] % 4)
        {
            Errors.push_back(Error("Incorrect code deepth on row " + std::to_string(i + 1)));
        }
        CodeDeepth[i] /= 4;
    }

    return 0;
}

int PyAnalyzer::SyntaxAnalisis()
{
    SyntaxTree = std::make_shared<SyntaxNode>(FToken("", "", 0, 0));
    SyntaxTree->Parent = SyntaxTree;
    SyntaxTree->Token.TokenType = ETokenType::Function;
    std::shared_ptr<SyntaxNode> current = SyntaxTree;;
    int deepth = 0;
    std::vector<int> brackets;
    std::vector<int> BoxBrackets;
    std::vector<std::shared_ptr<SyntaxNode>> FunctionsStack;
    
    for (int i = 0; i < Tokens.size(); i++) {

        if (i && Tokens[i - 1].RowIndex != Tokens[i].RowIndex)
        {
            while (current->Token.ValueName != "")
            {
                current = current->Parent.lock();
            }

            int diff = CodeDeepth[Tokens[i - 1].RowIndex] - CodeDeepth[Tokens[i].RowIndex];
            if (diff < -1)
            {
                Errors.push_back(Error("incorrect code deepth change at line " + std::to_string(Tokens[i].RowIndex)));
            }

            if (diff == -1 && Tokens[i-1].ValueName != ":")
            {
                Errors.push_back(Error("Unexpected indent at line " + std::to_string(Tokens[i].RowIndex)));
            }

            while (diff > 0)
            {
                current = current->Parent.lock();
                diff--;
                deepth--;
            }

            diff = deepth;
            while (diff > 0)
            {
                current = current->Children.back();
                diff--;
            }
        }
        if (Tokens[i].ValueName == ":") {
            /*std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);*/
            deepth++;
            //current = current->Children.back();
        }
        else if (Tokens[i].ValueName == "(") {
            
            brackets.push_back(i);

            if (i > 0 && Tokens[i - 1].TokenType == ETokenType::Function)
            {
                current = current->Children.back();
                FunctionsStack.push_back(current);
            }
            else
            {
                std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
                current->Children.push_back(child);
                current = current->Children.back();
            }
            
        }
        else if (Tokens[i].ValueName == "[") {


            if (i > 0 && Tokens[i - 1].TokenType == ETokenType::Variable)
            {
                FunctionsStack.push_back(current);
                current = current->Children.back();
            }
            else
            {
                std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
                child->Token.ValueName = "[]";
                child->Token.TokenType = ETokenType::Function;
                child->Token.Description = "create array";
                current->Children.push_back(child);
                current = current->Children.back();
                FunctionsStack.push_back(current);
                
            }
        }
        else if (Tokens[i].ValueName == "]") {

            // Make [] operator and not close i
            if (FunctionsStack.empty())
            {
                Errors.push_back(Error("Incoorect use of ',' outside function at | " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }

            current = FunctionsStack.back();
        }
        else if (Tokens[i].ValueName == ",") {

            if (FunctionsStack.empty())
            {
                Errors.push_back(Error("Incoorect use of ',' outside function at | " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }

            current = FunctionsStack.back();
            std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);
        }
        else if (Tokens[i].ValueName == ")") {
            // Move back up to the parent node

            if (brackets.empty())
            {
                Errors.push_back(Error("Mismatched brackets at | " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }

            if (brackets.back() > 0 && Tokens[brackets.back() - 1].TokenType == ETokenType::Function)
            {
                
                if (FunctionsStack.size())
                {
                    current = FunctionsStack.back()->Parent.lock();
                    //current = current->Parent.lock();
                    FunctionsStack.pop_back();
                }
                else
                {
                    Errors.push_back(Error("Mismatched brackets at | " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                    return 1;
                }
            }
            else
            {
                if (current->Children.size()) current = current->Children.back();
                std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
                current->Children.push_back(child);
                current = current->Children.back();
            }

            brackets.pop_back();
            
        } 
        else if (Tokens[i].TokenType == ETokenType::Operator)
        {
            ProcessExpression(i, current);
        }
        else if (Tokens[i].TokenType == ETokenType::KeyWord)
        {
            ProcessKeyWord(i, current);
        }
        else {
            // Create a new node for the variable and add it as a child of the current node
            std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);
        }
    }
    return 0;
}

int PyAnalyzer::SemanticAnalisis()
{
    /*int a = 1;
    float b = 3.14;
    std::string c = "Hellow, World!";
    std::vector<FVariable> v;
    v.push_back(FVariable("a", "int", &a));
    v.push_back(FVariable("b", "float", &b));
    v.push_back(FVariable("c", "string", &c));
    FVariable d("d", "array", &v);

    d.Print();

    return 0;*/

    for (int i = 0; i < SyntaxTree->Children.size(); i++)
    {
        if (SemanticCheck(SyntaxTree->Children[i], SyntaxTree, i)) return 1;
    }

    return 0;
}

std::string PyAnalyzer::ReadNumberConstant(int& x, int& y, bool& Flag)
{
    std::string res = "";
    bool WasDot = false;
    bool WasComplexJ = false;
    bool BadToken = false;

    if (Code[x][y] == '-') res += Code[x][y];
    if (Code[x][y] == '-' || Code[x][y] == '+') y++;
    

    for (; y < Code[x].size(); y++)
    {
        char c = Code[x][y];

        if (Delimiters.count(c) || OperatorsCharacters.count(c))
        {
            y--;
            break;
        }

        res += c;

        if (BadToken) continue;

        if (!AllowedCharacters.count(c)) continue;

        if (c >= '0' && c <= '9')
        {
            if (WasComplexJ)
            {
                BadToken = true;
                Errors.push_back(Error(std::string("Can't be numbers after complex j : ") + std::to_string(x) + ":" + std::to_string(y)));
                continue;
            }
        }

        if (c == '.')
        {
            if (WasDot)
            {
                BadToken = true;
                Errors.push_back(Error(std::string("Can't ave more than one dot in number : ") + std::to_string(x) + ":" + std::to_string(y)));
                continue;
            }
            WasDot = true;
        }

        if (c == 'j' || c == 'J')
        {
            if (WasComplexJ)
            {
                BadToken = true;
                Errors.push_back(Error(std::string("Can't ave more than one complex j in number : ") + std::to_string(x) + ":" + std::to_string(y)));
                continue;
            }
            WasComplexJ = true;
        }
    }

    Flag = BadToken;
    return res;
}

std::string PyAnalyzer::ReadOperator(int& x, int& y, bool& Flag)
{
    std::string res = "";

    for (; y < Code[x].size(); y++)
    {
        char c = Code[x][y];

        if (OperatorsCharacters.count(c))
        {
            res += c;
        }
        else
        {
            y--;
            break;
        }
    }

    if (Operators.count(res))
    {
        Flag = false;
    }
    else
    {
        Flag = true;
        Errors.push_back(Error(std::string("Unknown operator : ") + res + " | " + std::to_string(x) + ":" + std::to_string(y)));
    }

    return res;
}

std::string PyAnalyzer::ReadWord(int& x, int& y, bool& Flag)
{
    std::string res = "";
    bool BadToken = false;

    for (; y < Code[x].size(); y++)
    {
        char c = Code[x][y];

        if (Delimiters.count(c) || OperatorsCharacters.count(c))
        {
            y--;
            break;
        }

        if (!AllowedCharacters.count(c)) BadToken = true;

        res += c;
    }

    Flag = BadToken;
    return res;
}

std::string PyAnalyzer::ReadLiteral(int& x, int& y, bool& Flag)
{
    std::string res = "";
    bool BadToken = false;
    int BracesCount = 0;
    char BegBrace = Code[x][y];

    for (; y < Code[x].size(); y++)
    {
        if (BracesCount == 2)
        {
            y--;
            Flag = BadToken;
            return res;
        }

        char c = Code[x][y];

        if (c == BegBrace)
        {
            BracesCount++;
            continue;
        }

        res += c;

        if (c == '\\')
        {
            if (!(y + 1 < Code[x].size() && AllowedEscapeChars.count(Code[x][y + 1])))
            {
                BadToken = true;
            }

            if (y + 1 < Code[x].size())
            {
                y++;
                res += Code[x][y];
            }
        }
    }

    if (BracesCount == 2)
    {
        Flag = BadToken;
        return res;
    }

    Errors.push_back(Error(std::string("Braces are opend, but never closed : ") + std::to_string(x) + ":" + std::to_string(y)));
    Flag = true;
    return "";
}

int PyAnalyzer::ReadFunctionSignature(int& x, int& y, bool& Flag)
{
    while (Code[x][y] == ' ') y++;
    std::string Name = "";

    for (; y < Code[x].size() && Code[x][y] != '(' && Code[x][y] != ' '; y++)
    {
        Name += Code[x][y];
        if (!(AllowedCharacters.count(Code[x][y]) && !Delimiters.count(Code[x][y]) && !OperatorsCharacters.count(Code[x][y])))
            Flag = true;
    }

    if (y == Code[x].size())
    {
        Errors.push_back(Error(std::string("Incorrect function definition : ") + std::to_string(x) + ":" + std::to_string(0)));
        return 1;
    }

    y--;

    if (Flag)
    {
        Errors.push_back(Error(std::string("Incorrect function name : ") + std::to_string(x) + ":" + std::to_string(y)));
        return 1;
    }
    else
    {
        Functions.insert(Name);
        Tokens.push_back(FToken(Name, "function", x, y, ETokenType::Function));
    }

    return 0;
}

int PyAnalyzer::ReadForSignature(int& x, int& y, bool& Flag)
{
    while (Code[x][y] == ' ') y++;
    std::string Name = "";

    for (; y < Code[x].size() && Code[x][y] != ' '; y++)
    {
        Name += Code[x][y];
        if (!(AllowedCharacters.count(Code[x][y]) && !Delimiters.count(Code[x][y]) && !OperatorsCharacters.count(Code[x][y])))
            Flag = true;
    }

    if (Flag)
    {
        Errors.push_back(Error(std::string("Incorrect variable name : ") + std::to_string(x) + ":" + std::to_string(y)));
        return 1;
    }
    else
    {
        TokenVariables.insert(Name);
        Tokens.push_back(FToken(Name, "variable", x, y, ETokenType::Variable));
    }

    return 0;
}

int PyAnalyzer::ProcessExpression(int& x, std::shared_ptr<SyntaxNode>& Node)
{

    if (AssigmentOperators.count(Tokens[x].ValueName))
    {
        if (x - 1 >= 0 && (Tokens[x - 1].TokenType == ETokenType::Variable || Tokens[x - 1].ValueName == "]"))
        {
            Node = Node->Children.back();
            std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
            Node->Children.push_back(Child);
            Node = Child;
        }
        else
        {
            Errors.push_back(
            Error
            (
                "Incorrect expression, expected Variable before operator " 
                + Tokens[x].ValueName + " | Found : " 
                + Tokens[x-1].ValueName 
                + " | at " 
                + std::to_string(Tokens[x].RowIndex) + ":" + std::to_string(Tokens[x].ColumnIndex)
            )
            );
            return 1;
        }
    }
    else
    {
        if (Node->Children.size()) Node = Node->Children.back();
        std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
        Node->Children.push_back(Child);
        Node = Child;
    }

    return 0;
}

int PyAnalyzer::ProcessKeyWord(int& x, std::shared_ptr<SyntaxNode>& Node)
{
    std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
    Node->Children.push_back(Child);
    Node = Child;
    return 0;
}

int PyAnalyzer::ReformatSyntaxTree()
{
    for (int i = 0; i < SyntaxTree->Children.size(); i++)
    {
        if (ReformatSyntaxNode(SyntaxTree->Children[i], SyntaxTree, i)) return 1;
    }
    return 0;
}

int PyAnalyzer::ReformatSyntaxNode(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index)
{
    if (Node->Token.TokenType == ETokenType::Variable 
        || Node->Token.TokenType == ETokenType::Literal 
        || Node->Token.TokenType == ETokenType::Number
        || Node->Token.TokenType == ETokenType::Function)
    {
        if (Node->Children.size() && Node->Children.back()->Token.TokenType == ETokenType::Operator)
        {
            Parent->Children[p_child_index] = BuildExpressionTree(Node);
            Node = Parent->Children[p_child_index];
            Node->Parent = Parent;
        }
    }
    else if (Node->Token.ValueName == "(")
    {
        Parent->Children[p_child_index] = BuildExpressionTree(Node);
        Node = Parent->Children[p_child_index];
        Node->Parent = Parent;
    }

    if (!Node) return 0;

    for (int i = 0; i < Node->Children.size(); i++)
    {
        ReformatSyntaxNode(Node->Children[i], Node, i);
    }

    if (Node->Token.ValueName == "for")
    {
        if (Node->Children.size() == 0 || Node->Children[0]->Token.ValueName != "in")
        {
            Errors.push_back(Error(" Incorrect for notation at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    if (Node->Token.ValueName == "if" || Node->Token.ValueName == "while" || Node->Token.ValueName == "elif")
    {
        if (Node->Children.size() == 0 || (
            Node->Children[0]->Token.TokenType != ETokenType::Number
            && Node->Children[0]->Token.TokenType != ETokenType::Variable
            && Node->Children[0]->Token.TokenType != ETokenType::Function
            && Node->Children[0]->Token.TokenType != ETokenType::Operator
            ))
        {
            Errors.push_back(Error(std::string(" Expected expression but found ") + '\"' + Code[Node->Token.RowIndex] + '\"' + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    if (Node->Token.ValueName == "else" || Node->Token.ValueName == "elif")
    {
        if (p_child_index == 0 
            || !(Parent->Children[p_child_index - 1]->Token.ValueName == "if"
            || Parent->Children[p_child_index - 1]->Token.ValueName == "elif")
            )
        {
            Errors.push_back(Error(std::string(" Expected if or elif notations befor  ") + Node->Token.ValueName + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    return 0;
}

int PyAnalyzer::checkSingleTokenDependensies()
{
    for (int i = 0; i < Tokens.size() - 1; i++)
    {

        if (Tokens[i + 1].RowIndex != Tokens[i].RowIndex)
        {
            if (Tokens[i + 1].TokenType == ETokenType::Operator)
            {
                Errors.push_back(Error(" found operator in the beginning of file : " + Tokens[i+1].ValueName + std::string(" | at ") + std::to_string(Tokens[i+1].RowIndex) + ":" + std::to_string(Tokens[i+1].ColumnIndex)));
                return 1;
            }
        }

        if (Tokens[i].ValueName == ":")
        {
            if (Tokens[i + 1].RowIndex == Tokens[i].RowIndex)
            {
                Errors.push_back(Error(" colon ':' must be followed by new string with additional tab, but found : " + Tokens[i].ValueName + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }
            else if (i + 1 < Tokens.size()
                && CodeDeepth[Tokens[i + 1].RowIndex] - 1 !=
                CodeDeepth[Tokens[i].RowIndex])
            {
                Errors.push_back(Error(" Line after colon must be on new deepth " + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }
        }

        switch (Tokens[i].TokenType)
        {
        case ETokenType::Literal:
            if (Tokens[i].RowIndex == Tokens[i+1].RowIndex &&
                ( Tokens[i + 1].TokenType == ETokenType::Literal 
                || Tokens[i + 1].TokenType == ETokenType::Number
                || Tokens[i + 1].TokenType == ETokenType::Variable
                || Tokens[i + 1].TokenType == ETokenType::Function
                || Tokens[i + 1].TokenType == ETokenType::Variable
                || !GoodTokensAfterLiteral.count(Tokens[i+1].ValueName))
                )
            {
                Errors.push_back(Error("Unexpected token : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
                return 1;
            }
            break;
        case ETokenType::Number:
            if (Tokens[i].RowIndex == Tokens[i + 1].RowIndex &&
                (Tokens[i + 1].TokenType == ETokenType::Literal
                    || Tokens[i + 1].TokenType == ETokenType::Number
                    || Tokens[i + 1].TokenType == ETokenType::Variable
                    || Tokens[i + 1].TokenType == ETokenType::Function
                    || Tokens[i + 1].TokenType == ETokenType::Variable
                    || !GoodTokensAfterNumber.count(Tokens[i + 1].ValueName))
                )
            {
                Errors.push_back(Error("Unexpected token : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
                return 1;
            }
            break;
        case ETokenType::Function:
            if (Tokens[i + 1].ValueName != "(")
            {
                Errors.push_back(Error("Expected '(' symbol, but found : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
                return 1;
            }
            break;
        case ETokenType::KeyWord:

            if (Tokens[i].ValueName == "for" || Tokens[i].ValueName == "while" || Tokens[i].ValueName == "if" || Tokens[i].ValueName == "elif")
            {
                int j;
                for (j = i + 1; j < Tokens.size(); j++)
                {
                    if (Tokens[j].ValueName == ":")
                        break;
                }
                if (j == Tokens.size() || Tokens[i].RowIndex != Tokens[j].RowIndex)
                {
                    Errors.push_back(Error(" 'if' line must end with colon ':' " + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                    return 1;
                }
                break;
            }
            else if (Tokens[i].ValueName == "else")
            {
                if (!(i < Tokens.size() - 1 && Tokens[i].RowIndex == Tokens[i + 1].RowIndex && Tokens[i+1].ValueName == ":"))
                {
                    Errors.push_back(Error(" 'else' line must end with colon ':' " + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                    return 1;
                }
                break;
            }
            else if (Tokens[i].ValueName == "continue" || Tokens[i].ValueName == "break")
            {
                if ( i > 0 && i < Tokens.size() - 1 && (Tokens[i + 1].RowIndex == Tokens[i].RowIndex || Tokens[i - 1].RowIndex == Tokens[i].RowIndex))
                {
                    Errors.push_back(Error(Tokens[i].ValueName + " must be in a saparate line of code " + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                    return 1;
                }
            }

            break;

        case ETokenType::Operator:
            if (Tokens[i].RowIndex != Tokens[i + 1].RowIndex)
            {
                Errors.push_back(Error("Unexpected operator in the end of line : " + Tokens[i].ValueName + " | at " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                return 1;
            }
            if (Tokens[i + 1].ValueName != "(" &&
                Tokens[i + 1].ValueName != "[" &&
                 (Tokens[i + 1].TokenType == ETokenType::KeyWord
                || Tokens[i + 1].TokenType == ETokenType::Operator
                || Tokens[i + 1].TokenType == ETokenType::Delimeter)
                )
            {
                Errors.push_back(Error("Unexpected token : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
                return 1;
            }
            break;
        case ETokenType::Variable:
            if (Tokens[i].RowIndex == Tokens[i + 1].RowIndex &&
                (Tokens[i + 1].TokenType == ETokenType::Literal
                    || Tokens[i + 1].TokenType == ETokenType::Number
                    || Tokens[i + 1].TokenType == ETokenType::Variable
                    || Tokens[i + 1].TokenType == ETokenType::Function
                    || Tokens[i + 1].TokenType == ETokenType::Variable
                    || !GoodTokensAfterVariable.count(Tokens[i + 1].ValueName))
                )
            {
                Errors.push_back(Error("Unexpected token : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
                return 1;
            }
            break;
        default:
            break;
        }
    }

    if (Tokens.back().TokenType == ETokenType::Operator
        || Tokens.back().TokenType == ETokenType::Function)
    {
        Errors.push_back(Error("Unexpected token : " + Tokens.back().ValueName + " | at " + std::to_string(Tokens.back().RowIndex) + ":" + std::to_string(Tokens.back().ColumnIndex)));
        return 1;
    }

    return 0;
}

int PyAnalyzer::checkBrackets()
{
    std::vector<FToken> stack;

    for (auto i : Tokens)
    {
        if ( Brackets.count(i.ValueName))
        {
            if (i.ValueName == "{" || i.ValueName == "[" || i.ValueName == "(")
            {
                stack.push_back(i);
            }
            else
            {
                if (stack.empty())
                {
                    Errors.push_back(Error("Incorrect bracket sequence : bracket wath never opend : at | " + std::to_string(i.RowIndex) + ":" + std::to_string(i.ColumnIndex)));
                    return 1;
                }
                else if (i.ValueName == "}" && stack.back().ValueName == "{")
                {
                    stack.pop_back();
                }
                else if (i.ValueName == ")" && stack.back().ValueName == "(")
                {
                    stack.pop_back();
                }
                else if (i.ValueName == "]" && stack.back().ValueName == "[")
                {
                    stack.pop_back();
                }
                else
                {
                    Errors.push_back(Error("Incorrect bracket sequence : brackets are unmatched : at | " + std::to_string(i.RowIndex) + ":" + std::to_string(i.ColumnIndex)));
                    return 1;
                }
            }
        }
    }

    return 0;
}

int PyAnalyzer::checkSyntaxTree()
{
    // delete ',' 
    for (int i = 0; i < SyntaxTree->Children.size(); i++)
    {
        if (checkFirstLineWordNode(SyntaxTree->Children[i])) return 1;
        if (checkSyntaxTree(SyntaxTree->Children[i], SyntaxTree, i)) return 1;
    }

    return 0;
}

int PyAnalyzer::checkSyntaxTree(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index)
{
    if (!Node) return 0;
    if (Node->Token.TokenType == ETokenType::Literal
        || Node->Token.TokenType == ETokenType::Number)
    {
        if (Node->Children.size())
        {
            Errors.push_back(Error("Unexpected syntax node" + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }
    else if (Node->Token.TokenType == ETokenType::Variable)
    {
        if (Node->Children.size() == 0) {}
        else if (Node->Children.size() == 1)
        {
            if (Node->Children.back()->Token.TokenType == ETokenType::Literal
                || Node->Children.back()->Token.TokenType == ETokenType::KeyWord
                )
            {
                Errors.push_back(Error("Expected numeric expression " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else
        {
            Errors.push_back(Error("Unexpected syntax nodes after " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }
    else if (Node->Token.TokenType == ETokenType::KeyWord)
    {
        if (Node->Token.ValueName == "break"
            || Node->Token.ValueName == "continue")
        {
            if (checkBrakContinueNode(Node)) return 1;
        }
    }
    else if (Node->Token.TokenType == ETokenType::Function)
    {
        if (checkFunctionNode(Node)) return 1;
    }
    else if (Node->Token.TokenType == ETokenType::Delimeter)
    {
        Errors.push_back(Error("Unreached delimeter " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return 1;
    }
    else if (Node->Token.TokenType == ETokenType::Operator)
    {
        if (Node->Children.size() != 2)
        {
            Errors.push_back(Error("Incorrect expression in operator " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }

        if (AssigmentOperators.count(Node->Token.ValueName))
        {
            if (Node->Children[0]->Token.TokenType != ETokenType::Variable)
            {
                Errors.push_back(Error("Must be variable to assign with operator " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (ComparisonOperators.count(Node->Token.ValueName))
        {
            if (ComparisonOperators.count(Node->Children[0]->Token.ValueName)
                || ComparisonOperators.count(Node->Children[1]->Token.ValueName)
                )
            {
                Errors.push_back(Error("Cant double or triple compare with operator " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }

        if (!AssigmentOperators.count(Node->Token.ValueName)
            && (AssigmentOperators.count(Node->Children[0]->Token.ValueName)
                || AssigmentOperators.count(Node->Children[1]->Token.ValueName)))
        {
            Errors.push_back(Error("Cant use assigment operators inside expressions : operator " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    for (int i = 0; i < Node->Children.size(); i++)
    {
        if (checkSyntaxTree(Node->Children[i], Node, i)) return 1;
    }

    return 0;
}

int PyAnalyzer::checkBrakContinueNode(std::shared_ptr<SyntaxNode> Node)
{
    auto tmp = Node;
    while (tmp->Token.ValueName != "for" && tmp->Token.ValueName != "while" && tmp != SyntaxTree)
        tmp = tmp->Parent.lock();

    if (tmp == SyntaxTree)
    {
        Errors.push_back(Error("Unreached key word " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return 1;
    }

    return 0;
}

int PyAnalyzer::checkFunctionNode(std::shared_ptr<SyntaxNode> Node)
{
    std::vector<std::shared_ptr<SyntaxNode>> childs;
    for (int i = 1; i < Node->Children.size(); i += 2)
    {
        if (Node->Children[i]->Token.ValueName != ",")
        {
            Errors.push_back(Error("Statements in function must be seperated with commas : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    for (int i = 0; i < Node->Children.size(); i += 2)
    {
        childs.push_back(Node->Children[i]);
    }

    Node->Children = childs;

    return 0;
}

int PyAnalyzer::checkFirstLineWordNode(std::shared_ptr<SyntaxNode> Node)
{

    if (!Node)
    {
        return 0;
    }

    auto T = Node->Token;

    if (T.TokenType == ETokenType::Delimeter)
    {
        Errors.push_back(Error("Unexpected delimeter " + T.ValueName + " : at | " + std::to_string(T.RowIndex) + ":" + std::to_string(T.ColumnIndex)));
        return 1;
    }

    return 0;
}


std::shared_ptr<PyAnalyzer::SyntaxNode> PyAnalyzer::BuildExpressionTree(std::shared_ptr<SyntaxNode> Node)
{

    std::shared_ptr<SyntaxNode> current = Node;
    std::vector<std::shared_ptr<SyntaxNode>> RPN;
    std::stack<std::shared_ptr<SyntaxNode>> opStack;
    std::stack<std::shared_ptr<SyntaxNode>> Stack;

    if (current->Token.TokenType == ETokenType::Variable &&
        current->Token.ValueName == "b")
    {
        int a = 1;
        int b = 2;
    }

    while (true)
    {
        if (current->Token.TokenType == ETokenType::Variable
            || current->Token.TokenType == ETokenType::Number
            || current->Token.TokenType == ETokenType::Literal
            || current->Token.TokenType == ETokenType::Function)
        {
            
            RPN.push_back(current);

            if (current->Children.empty()) { break; }
            if (current->Children.back()->Token.TokenType == ETokenType::Delimeter) {}
            else if ( current->Children.back()->Token.TokenType != ETokenType::Operator) { break; }
            
        }
        else if (current->Token.ValueName == "(")
        {
            opStack.push(current);
        }
        else if (current->Token.ValueName == ")")
        {
            while (!opStack.empty() && opStack.top()->Token.TokenType != ETokenType::Delimeter)
            {
                RPN.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty() && opStack.top()->Token.TokenType == ETokenType::Delimeter && opStack.top()->Token.ValueName == "(")
            {
                opStack.pop();
            }
            else
            {
                Errors.push_back(Error("Mismatched parentheses : at | " + std::to_string(current->Token.RowIndex) + ":" + std::to_string(current->Token.ColumnIndex)));
                return std::shared_ptr<SyntaxNode>();
            }
        }
        else
        {
            while (!opStack.empty()
                && OperatorPrecedence[current->Token.ValueName] 
                <= OperatorPrecedence[opStack.top()->Token.ValueName])
            {
                RPN.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(current);
        }

        if (current->Children.empty()) { break; }
        auto tmp = current->Children.back();
        current->Children.pop_back();
        current = tmp;

    }

    while (!opStack.empty())
    {
        RPN.push_back(opStack.top());
        opStack.pop();
    }

    for (const auto& Token : RPN)
    {
        std::shared_ptr<SyntaxNode> Node = Token;

        if (Token->Token.TokenType == ETokenType::Variable
            || Token->Token.TokenType == ETokenType::Number
            || Token->Token.TokenType == ETokenType::Literal
            || Token->Token.TokenType == ETokenType::Function
            || Token->Token.TokenType == ETokenType::Operator)
        {
            if (Token->Token.TokenType == ETokenType::Operator)
            {
                if (Stack.size() < 2)
                {
                    Errors.push_back(Error("Invalid expression 1 : at | " + std::to_string(Token->Token.RowIndex) + ":" + std::to_string(Token->Token.ColumnIndex)));
                    return std::shared_ptr<SyntaxNode>();
                }

                auto Right = Stack.top();
                Stack.pop();

                auto Left = Stack.top();
                Stack.pop();

                Node->Children.push_back(Left);
                Node->Children.push_back(Right);

                Left->Parent = Node;
                Right->Parent = Node;
            }
            Stack.push(Node);
        }
        else
        {
            Errors.push_back(Error("Invalid expression 2 : at | " + std::to_string(Token->Token.RowIndex) + ":" + std::to_string(Token->Token.ColumnIndex)));
            return std::shared_ptr<SyntaxNode>();
        }
    }

    if (Stack.size() != 1)
    {
        Errors.push_back(Error("Invalid expression 3 : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return std::shared_ptr<SyntaxNode>();
    }

    auto Root = Stack.top();
    Root->Parent.reset();

    return Root;
}

int PyAnalyzer::SemanticCheck(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index, int scope)
{
    // типы операндов литералы, числа не сравниваются
    
    if (!Node) return 0;
    if (Node->Token.TokenType == ETokenType::Operator) // check operands
    {
        if (AssigmentOperators.count(Node->Token.ValueName))
        {

        }
        else
        {
            if ((Node->Token.ValueName == "/"
                || Node->Token.ValueName == "%"
                || Node->Token.ValueName == "//")
                && Node->Children[1]->Token.ValueName == "0"
                && Node->Children[1]->Token.TokenType == ETokenType::Number
                )
            {
                Errors.push_back(Error("Division by zero : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }

            if ((Node->Children[0]->Token.TokenType == ETokenType::Number
                && Node->Children[1]->Token.TokenType == ETokenType::Literal)
                || 
                (Node->Children[1]->Token.TokenType == ETokenType::Number
                && Node->Children[0]->Token.TokenType == ETokenType::Literal))
            {
                Errors.push_back(Error("Cant do operation between string and number : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
    }
    else if (Node->Token.TokenType == ETokenType::Function) // check func signature
    {
        if (Node->Token.ValueName == "print")
        {
            // не один дочирний узел, посчитать выражения, и збс
        }
        else if (Node->Token.ValueName == "range")
        {
            if (!Node->Parent.lock() 
                || !Node->Parent.lock()->Parent.lock()
                || Node->Parent.lock()->Token.ValueName != "in"
                || Node->Parent.lock()->Parent.lock()->Token.ValueName != "for")
            {
                Errors.push_back(Error("Incorrect use of function 'range' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "type")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'type' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "int")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'int' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "input")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'input' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "float")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'float' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "array")
        {
            // не один дочирний узел, посчитать выражение, и збс
        }
        else if (Node->Token.ValueName == "[]")
        {
            // не один дочирний узел, посчитать выражение, и збс
        }
        else if (Node->Token.ValueName == "complex")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'complex' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "string")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'string' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        
        }else if (Node->Token.ValueName == "append")
        {
            if (Node->Children.size() != 2)
            {
                Errors.push_back(Error("Expected two argument in function 'append' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }
        else if (Node->Token.ValueName == "pop_back")
        {
            if (Node->Children.size() != 1)
            {
                Errors.push_back(Error("Expected one argument in function 'pop_back' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return 1;
            }
        }

    }
    else if (Node->Token.TokenType == ETokenType::Variable) // check [] operator
    {
        if (Node->Children.size()
            && Node->Children[0]->Token.TokenType == ETokenType::Literal)
        {
            Errors.push_back(Error("Literal can't be an array index : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }

        if (Node->Children.size()
            && Node->Children[0]->Token.TokenType == ETokenType::Number
            && GetNumberType(Node->Children[0]->Token.ValueName) > 1)
        {
            Errors.push_back(Error("Only integers can be an array index : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    for (int i = 0; i < Node->Children.size(); i++)
    {
        if (SemanticCheck(Node->Children[i], Node, i, scope + 1)) return 1;
    }

    return 0;
}

int PyAnalyzer::GetNumberType(const std::string& val)
{
    if (val.find("j") != val.npos || val.find("J") != val.npos)
    {
        return 3;
    }
    if (val.find(".") != val.npos)
    {
        return 2;
    }
    else
    {
        return 1;
    }
    return 0;
}

int PyAnalyzer::Execute()
{
    return ExecScope(SyntaxTree, 0);
}

int PyAnalyzer::ExecScope(std::shared_ptr<SyntaxNode> Scope, int scope_id)
{
    int i = 0;
    if (Scope->Token.TokenType == ETokenType::KeyWord) i++;
    for (; i < Scope->Children.size(); i++)
    {
        auto T = Scope->Children[i]->Token;
        if (T.TokenType == ETokenType::Operator)
        {
            if (AssigmentOperators.count(T.ValueName))
            {
                std::string VarName = Scope->Children[i]->Children[0]->Token.ValueName;

                if (Scope->Children[i]->Children[0]->Children.size())
                {
                    std::vector<FVariable>* Varr = reinterpret_cast<std::vector<FVariable>*>(Vars[VarName].Value);
                    auto index = ExecExpr(Scope->Children[i]->Children[0]->Children[0]);
                    
                    if (index.Type == "")
                    {
                        if (Errors.size() && !(Errors.back().Message.back() >= '0' && Errors.back().Message.back() <= '9'))
                            Errors.back().Message = Errors.back().Message + std::string(" | at ") + std::to_string(T.RowIndex) + ":" + std::to_string(T.ColumnIndex);
                        return 1;
                    }

                    if (index.Type != "int")
                    {
                        Errors.push_back(Error("Incorrect index type, expected 'int', but found : " + index.Type + " at | " + std::to_string(T.RowIndex) + ":" + std::to_string(T.ColumnIndex)));
                        return 1;
                    }

                    int id = *reinterpret_cast<int*>(index.Value);
                    if (id < 0 || id > Varr->size() - 1)
                    {
                        Errors.push_back(Error("Index out of bounds at | " + std::to_string(T.RowIndex) + ":" + std::to_string(T.ColumnIndex)));
                        return 1;
                    }

                    Varr->at(id) = ExecExpr(Scope->Children[i]->Children[1]);
                    if (Varr->at(id).Type == "") return 1;
                    Vars[VarName].Value = Varr;
                }
                else
                {
                    if (Vars.find(VarName) == Vars.end())
                    {
                        Vars[VarName] = ExecExpr(Scope->Children[i]->Children[1]);
                        if (Vars[VarName].Type == "") return 1;
                        Vars[VarName].Scope = scope_id;
                    }
                    else
                    {
                        auto exp = ExecExpr(Scope->Children[i]->Children[1]);
                        if (exp.Type == "") return 1;
                        Vars[VarName].Value = exp.Value;
                        Vars[VarName].Type = exp.Type;
                    }
                }
            }
            else 
            {
                auto check = ExecExpr(Scope->Children[i]->Children[1]);
                if (check.Type == "") return 1;
            }
        }
        else if (T.TokenType == ETokenType::Function)
        {
            FVariable tmp = ExecFunction(Scope->Children[i]);
            if (tmp.Type == "") return 1;
        }
        else if (T.TokenType == ETokenType::KeyWord)
        {
            if (T.ValueName == "for")
            {
                if (ExecFor(Scope->Children[i], scope_id+1)) return 1;
            }
            else if (T.ValueName == "while")
            {
                if (ExecWhile(Scope->Children[i], scope_id + 1)) return 1;
            }
            else if (T.ValueName == "if")
            {
                auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                if (exp.Type != "int")
                {
                    Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                    return 1;
                }
                int st = *reinterpret_cast<int*>(exp.Value);
                if (st)
                {
                    if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                    while (i + 1 < Scope->Children.size()
                        && (Scope->Children[i + 1]->Token.ValueName == "else"
                            || Scope->Children[i + 1]->Token.ValueName == "elif"))
                    {
                        i++;
                    }
                }
            }
            else if (T.ValueName == "else")
            {
                if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
            }
            else if (T.ValueName == "elif")
            {
                auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                if (exp.Type != "int")
                {
                    Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                    return 1;
                }
                int st = *reinterpret_cast<int*>(exp.Value);
                if (st)
                {
                    if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                    while (i + 1 < Scope->Children.size()
                        && (Scope->Children[i + 1]->Token.ValueName == "else"
                            || Scope->Children[i + 1]->Token.ValueName == "elif"))
                    {
                        i++;
                    }
                }
            }
        }
        
    }

    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////////   Clear Scope  //////////////////////////
    //////////////////////////////////////////////////////////////////////////

    auto tmp = Vars;
    Vars.clear();

    for (auto i : tmp)
    {
        if (i.second.Scope != scope_id)
            Vars[i.first] = i.second;
    }
    return 0;
}

int PyAnalyzer::ExecFor(std::shared_ptr<SyntaxNode> Scope, int scope_id)
{
    int iterc = 0;
    std::string VarName = Scope->Children[0]->Children[0]->Token.ValueName;
    std::pair<int, int> rng;
    std::vector<FVariable> arr;

    if (Scope->Children[0]->Children[1]->Token.ValueName == "range")
    {
        rng = ExecRange(Scope->Children[0]->Children[1]);
        if (rng.second == -1) return 1;
        
        if (Vars.find(VarName) != Vars.end())
        {
            Vars[VarName].Value = new int(rng.first + iterc);
        }
        else
        {
            Vars[VarName] = FVariable("", "int", new int(rng.first + iterc));
            Vars[VarName].Scope = scope_id;
        }
    }
    else
    {
        auto exp = CallArray(Scope->Children[0]->Children[1]);
        if (Scope->Children[0]->Children[1]->Token.ValueName == "[]") exp.Scope = scope_id;
        if (exp.Type == "") return 1;
        if (exp.Type != "array")
        {
            Errors.push_back(Error("Loop statement mast be of array type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[0]->Children[1]->Token.RowIndex) + ":" + std::to_string(Scope->Children[0]->Children[1]->Token.ColumnIndex)));
            return 1;
        }
        arr = *reinterpret_cast<std::vector<FVariable>*>(exp.Value);

        
        if (Vars.find(VarName) == Vars.end())
        {
            Vars[VarName].Scope = scope_id;
        }
    }

    while (true)
    {

        if (Scope->Children[0]->Children[1]->Token.ValueName == "range")
        {
            if (rng.first + iterc > rng.second) break;
            Vars[VarName].Value = new int(rng.first + iterc);
        }
        else
        {
            if (iterc >= arr.size()) break;
            Vars[VarName].Value = arr[iterc].Value;
            Vars[VarName].Type = arr[iterc].Type;
        }

        iterc++;

        for (int i = 1; i < Scope->Children.size(); i++)
        {
            auto T = Scope->Children[i]->Token;
            if (T.TokenType == ETokenType::Operator)
            {
                if (AssigmentOperators.count(T.ValueName))
                {
                    std::string VarName = Scope->Children[i]->Children[0]->Token.ValueName;
                    Vars[VarName] = ExecExpr(Scope->Children[i]->Children[1]);
                    if (Vars[VarName].Type == "") return 1;
                    if (Vars[VarName].Scope < 0) Vars[VarName].Scope = scope_id;
                }
                else
                {
                    auto check = ExecExpr(Scope->Children[i]->Children[1]);
                    if (check.Type == "") return 1;
                }
            }
            else if (T.TokenType == ETokenType::Function)
            {
                FVariable tmp = ExecFunction(Scope->Children[i]);
                if (tmp.Type == "") return 1;
            }
            else if (T.TokenType == ETokenType::KeyWord)
            {
                if (T.ValueName == "for")
                {
                    if (ExecFor(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "while")
                {
                    if (ExecWhile(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "if")
                {
                    auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                    if (exp.Type != "int")
                    {
                        Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                        return 1;
                    }
                    int st = *reinterpret_cast<int*>(exp.Value);
                    if (st)
                    {
                        if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                        while (i + 1 < Scope->Children.size()
                            && (Scope->Children[i + 1]->Token.ValueName == "else"
                                || Scope->Children[i + 1]->Token.ValueName == "elif"))
                        {
                            i++;
                        }
                    }
                }
                else if (T.ValueName == "else")
                {
                    if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "elif")
                {
                    auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                    if (exp.Type != "int")
                    {
                        Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                        return 1;
                    }
                    int st = *reinterpret_cast<int*>(exp.Value);
                    if (st)
                    {
                        if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                        while (i + 1 < Scope->Children.size()
                            && (Scope->Children[i + 1]->Token.ValueName == "else"
                                || Scope->Children[i + 1]->Token.ValueName == "elif"))
                        {
                            i++;
                        }
                    }
                }
            }

        }

        //////////////////////////////////////////////////////////////////////////
        ////////////////////////////////   Clear Scope  //////////////////////////
        //////////////////////////////////////////////////////////////////////////

        auto tmp = Vars;
        Vars.clear();

        for (auto i : tmp)
        {
            if (i.second.Scope != scope_id)
                Vars[i.first] = i.second;
        }
        return 0;
    }
}

int PyAnalyzer::ExecWhile(std::shared_ptr<SyntaxNode> Scope, int scope_id)
{
    
    auto exp = ExecExpr(Scope->Children[0]->Children[0]);
    if (exp.Type != "int")
    {
        Errors.push_back(Error("while statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[0]->Token.RowIndex) + ":" + std::to_string(Scope->Children[0]->Token.ColumnIndex)));
        return 1;
    }
    int st = *reinterpret_cast<int*>(exp.Value);

    while (st)
    {
        for (int i = 1; i < Scope->Children.size(); i++)
        {
            auto T = Scope->Children[i]->Token;
            if (T.TokenType == ETokenType::Operator)
            {
                if (AssigmentOperators.count(T.ValueName))
                {
                    std::string VarName = Scope->Children[i]->Children[0]->Token.ValueName;
                    Vars[VarName] = ExecExpr(Scope->Children[i]->Children[1]);
                    if (Vars[VarName].Type == "") return 1;
                    if (Vars[VarName].Scope < 0) Vars[VarName].Scope = scope_id;
                }
                else
                {
                    auto check = ExecExpr(Scope->Children[i]->Children[1]);
                    if (check.Type == "") return 1;
                }
            }
            else if (T.TokenType == ETokenType::Function)
            {
                FVariable tmp = ExecFunction(Scope->Children[i]);
                if (tmp.Type == "") return 1;
            }
            else if (T.TokenType == ETokenType::KeyWord)
            {
                if (T.ValueName == "for")
                {
                    if (ExecFor(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "while")
                {
                    if (ExecWhile(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "if")
                {
                    auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                    if (exp.Type != "int")
                    {
                        Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                        return 1;
                    }
                    int st = *reinterpret_cast<int*>(exp.Value);
                    if (st)
                    {
                        if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                        while (i + 1 < Scope->Children.size()
                            && (Scope->Children[i + 1]->Token.ValueName == "else"
                                || Scope->Children[i + 1]->Token.ValueName == "elif"))
                        {
                            i++;
                        }
                    }
                }
                else if (T.ValueName == "else")
                {
                    if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                }
                else if (T.ValueName == "elif")
                {
                    auto exp = ExecExpr(Scope->Children[i]->Children[0]);
                    if (exp.Type != "int")
                    {
                        Errors.push_back(Error("If statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[i]->Token.RowIndex) + ":" + std::to_string(Scope->Children[i]->Token.ColumnIndex)));
                        return 1;
                    }
                    int st = *reinterpret_cast<int*>(exp.Value);
                    if (st)
                    {
                        if (ExecScope(Scope->Children[i], scope_id + 1)) return 1;
                        while (i + 1 < Scope->Children.size()
                            && (Scope->Children[i + 1]->Token.ValueName == "else"
                                || Scope->Children[i + 1]->Token.ValueName == "elif"))
                        {
                            i++;
                        }
                    }
                }
            }
        }

        exp = ExecExpr(Scope->Children[0]->Children[0]);
        if (exp.Type != "int")
        {
            Errors.push_back(Error("while statement mast be of bool type, but found " + exp.Type + " | at " + std::to_string(Scope->Children[0]->Token.RowIndex) + ":" + std::to_string(Scope->Children[0]->Token.ColumnIndex)));
            return 1;
        }
        st = *reinterpret_cast<int*>(exp.Value);
    }

    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////////   Clear Scope  //////////////////////////
    //////////////////////////////////////////////////////////////////////////

    auto tmp = Vars;
    Vars.clear();

    for (auto i : tmp)
    {
        if (i.second.Scope != scope_id)
            Vars[i.first] = i.second;
    }
    return 0;
}

PyAnalyzer::FVariable PyAnalyzer::ExecExpr(std::shared_ptr<SyntaxNode> Node)
{
    if (Node->Token.TokenType == ETokenType::Literal)
    {
        return FVariable("", "string", &Node->Token.ValueName);
    }

    if (Node->Token.TokenType == ETokenType::Number)
    {
        return GetNumFromString(Node->Token.ValueName);
    }

    if (Node->Token.TokenType == ETokenType::Variable)
    {
        if (Node->Children.size())
        {
            auto index = ExecExpr(Node->Children[0]);

            if (index.Type == "")
            {
                if (Errors.size() && !(Errors.back().Message.back() >= '0' && Errors.back().Message.back() <= '9'))
                    Errors.back().Message = Errors.back().Message + std::string(" | at ") + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex);
                return FVariable();
            }

            if (index.Type != "int")
            {
                Errors.push_back(Error("Incorrect index type, expected 'int', but found : " + index.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
                return FVariable();
            }

            std::vector<FVariable> Varr = *reinterpret_cast<std::vector<FVariable>*>(Vars[Node->Token.ValueName].Value);
            int id = *reinterpret_cast<int*>(index.Value);
            return Varr[id];
        }

        return Vars[Node->Token.ValueName];
    }

    if (Node->Token.TokenType == ETokenType::Function)
    {
        return ExecFunction(Node);
    }

    if (Node->Token.TokenType == ETokenType::Operator)
    {
        auto l = ExecExpr(Node->Children[0]);
        auto r = ExecExpr(Node->Children[1]);

        if (l.Type == "" || r.Type == "")
        {
            if (Errors.size() && !(Errors.back().Message.back() >= '0' && Errors.back().Message.back() <= '9'))
                Errors.back().Message = Errors.back().Message + std::string(" | at ") + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex);
            return FVariable();
        }
        return ExecOperation(Node->Token.ValueName, l, r);
    }
}

PyAnalyzer::FVariable PyAnalyzer::ExecFunction(std::shared_ptr<SyntaxNode> Node)
{
    if (Node->Token.ValueName == "print")
    {
        return CallPrint(Node);
    }
    else if (Node->Token.ValueName == "range")
    {
        if (!Node->Parent.lock()
            || !Node->Parent.lock()->Parent.lock()
            || Node->Parent.lock()->Token.ValueName != "in"
            || Node->Parent.lock()->Parent.lock()->Token.ValueName != "for")
        {
            Errors.push_back(Error("Incorrect use of function 'range' : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return FVariable();
        }
    }
    else if (Node->Token.ValueName == "type")
    {
        return CallType(Node);
    }
    else if (Node->Token.ValueName == "int")
    {
        return CallInt(Node);
    }
    else if (Node->Token.ValueName == "input")
    {
        return CallInput(Node);
    }
    else if (Node->Token.ValueName == "float")
    {
        return CallFloat(Node);
    }
    else if (Node->Token.ValueName == "array")
    {
        return CallArray(Node);
    }
    else if (Node->Token.ValueName == "[]")
    {
        return CallArray(Node);
    }
    else if (Node->Token.ValueName == "string")
    {
        return CallString(Node);
    }
    else if (Node->Token.ValueName == "append")
    {
        return FVariable();
    }
    else if (Node->Token.ValueName == "pop_back")
    {
        return FVariable();
    }

    Errors.push_back(Error("Unknown function : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
    return FVariable();
}

PyAnalyzer::FVariable PyAnalyzer::ExecOperation(std::string op, FVariable l, FVariable r)
{

    if ((l.Type == "string"
        && r.Type != "string")
        || (l.Type != "string"
            && r.Type == "string"))
    {
        Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
        return FVariable();
    }

    if (l.Type == "array"
        && r.Type == "array" 
        && op != "=="
        && op != "!=")
    {
        if (op != "=="
            && op != "!=")
        {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }

        if (   op == "=="
            || op == "!=")
        {
            std::vector<FVariable> larray = *reinterpret_cast<std::vector<FVariable>*>(l.Value);
            std::vector<FVariable> rarray = *reinterpret_cast<std::vector<FVariable>*>(r.Value);

            if (larray.size() != rarray.size())
            {
                if (op == "==")
                    return FVariable("", "int", new int(0));
                else
                    return FVariable("", "int", new int(1));
            }

            for (int i = 0; i < larray.size(); i++)
            {
                FVariable res = ExecOperation(op, larray[i], rarray[i]);
                if (res.Type == "") { Errors.clear(); }
                if (op == "==")
                {
                    if (res.Type == "")
                        return FVariable("", "int", new int(0));
                    int isEq = *reinterpret_cast<int*>(res.Value);
                    if (!isEq)
                        return FVariable("", "int", new int(0));
                }
                else
                {
                    if (res.Type == "")
                        return FVariable("", "int", new int(1));
                    int isnEq = *reinterpret_cast<int*>(res.Value);
                    if (isnEq)
                        return FVariable("", "int", new int(1));
                }
                
                int isIn = *reinterpret_cast<int*>(res.Value);
                if (isIn)
                    return res;
            }

            return FVariable("", "int", new int(1));
        }

    }

    if (l.Type == "array")
    {
        Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
        return FVariable();
    }

    if (r.Type == "array")
    {
        if (op != "in")
        {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }

        std::vector<FVariable> array = *reinterpret_cast<std::vector<FVariable>*>(r.Value);
        for (auto i : array)
        {
            FVariable res = ExecOperation("==", l, i);
            if (res.Type == "") { Errors.clear(); continue; }
            int isIn = *reinterpret_cast<int*>(res.Value);
            if (isIn)
                return res;
        }
        return FVariable("", "int", new int(0));        
    }

    if (l.Type == "string" && r.Type == "string")
    {
        std::string lval = *reinterpret_cast<std::string*>(l.Value);
        std::string rval = *reinterpret_cast<std::string*>(r.Value);

        if (op == "+") {
            return FVariable("", "string", new std::string(lval + rval));
        }
        else if (op == "-") {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }
        else if (op == "*") {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }
        else if (op == "/") {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }
        else if (op == "%") {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type));
            return FVariable();
        }
        else if (op == "<") {
            return FVariable("", "int", new int(lval < rval));
        }
        else if (op == ">") {
            return FVariable("", "int", new int(lval > rval));
        }
        else if (op == "<=") {
            return FVariable("", "int", new int(lval <= rval));
        }
        else if (op == ">=") {
            return FVariable("", "int", new int(lval >= rval));
        }
        else if (op == "==") {
            return FVariable("", "int", new int(lval == rval));
        }
        else if (op == "!=") {
            return FVariable("", "int", new int(lval != rval));
        }
        else {
            throw std::invalid_argument("Unsupported operator for type int");
        }
    }

    if (BuiltinNumeric.count(l.Type) && BuiltinNumeric.count(r.Type))
    {
        std::complex<float> lval; 
        std::complex<float> rval; 
        std::complex<float> res;
        std::string returnType = "";

        if (l.Type == "complex") { lval = *reinterpret_cast<std::complex<float>*>(l.Value); }
        else if (l.Type == "float") { lval = *reinterpret_cast<float*>(l.Value); }
        else if (l.Type == "int") { lval = *reinterpret_cast<int*>(l.Value); }

        if (r.Type == "complex") { rval = *reinterpret_cast<std::complex<float>*>(r.Value); }
        else if (r.Type == "float") { rval = *reinterpret_cast<float*>(r.Value); }
        else if (r.Type == "int") { rval = *reinterpret_cast<int*>(r.Value); }

        if (op == "+") {
            res = lval + rval;
        }
        else if (op == "-") {
            res = lval - rval;
        }
        else if (op == "*") {
            res = lval * rval;
        }
        else if (op == "/") {
            res = lval / rval;
        }
        else if (op == "%") {
            if (!(l.Type == "int" && r.Type == "int"))
            {
                Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | mod operation only applyable to integers"));
                return FVariable();
            }
            res = int(lval.real()) % int(rval.real());
        }
        else if (op == "<") {

            if (l.Type == "complex" || r.Type == "complex")
            {
                Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | complex numbers are uncompareable"));
                return FVariable();
            }
            res = lval.real() < rval.real();
            returnType = "int";
        }
        else if (op == ">") {
            if (l.Type == "complex" || r.Type == "complex")
            {
                Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | complex numbers are uncompareable"));
                return FVariable();
            }
            res = lval.real() > rval.real();
            returnType = "int";
        }
        else if (op == "<=") {

            if (l.Type == "complex" || r.Type == "complex")
            {
                Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | complex numbers are uncompareable"));
                return FVariable();
            }
            res = lval.real() <= rval.real();
            returnType = "int";
        }
        else if (op == ">=") {
            if (l.Type == "complex" || r.Type == "complex")
            {
                Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | complex numbers are uncompareable"));
                return FVariable();
            }
            lval.real() >= rval.real();
            returnType = "int";
        }
        else if (op == "==") {
            res = lval == rval;
            returnType = "int";
        }
        else if (op == "!=") {
            res = lval != rval;
            returnType = "int";
        }
        else if (op == "in") {
            Errors.push_back(Error("Can't execute " + l.Type + " " + op + " " + r.Type + " | numbers can't be included"));
            return FVariable();
        }
        else {
            throw std::invalid_argument("Unsupported operator for type int");
        }

        if (returnType == "")
        {
            if (l.Type == "complex" || r.Type == "complex")
            {
                returnType = "complex";
            }
            else if (l.Type == "float" || r.Type == "float")
            {
                returnType = "float";
            }
            else if (l.Type == "int" || r.Type == "int")
            {
                returnType = "int";
            }
        }

        if (returnType == "complex")
        {
            return FVariable("", returnType, new std::complex<float>(res));
        }
        else if (returnType == "float")
        {
            return FVariable("", returnType, new float(res.real()));
        }
        else if (returnType == "int")
        {
            return FVariable("", returnType, new int(res.real()));
        }
        else
        {
            Errors.push_back(Error("Invalid numeric type"));
            return FVariable();
        }
    }

    
}

PyAnalyzer::FVariable PyAnalyzer::GetNumFromString(const std::string& val)
{
    try {
        // Try parsing as integer
        int value = std::stoi(val);
        return  FVariable("", "int", new int(value));
    }
    catch (std::invalid_argument&) {}

    try {
        // Try parsing as float
        float value = std::stof(val);
        return  FVariable("", "float", new float(value));
    }
    catch (std::invalid_argument&) {}

    try {
        // Try parsing as complex number
        std::complex<float> value = std::stof(val);
        return  FVariable("", "complex", new std::complex<float>(value));
    }
    catch (std::invalid_argument&) {}

    Errors.push_back(Error("Incorrect number literal " + val));

    // If no match is found, return a null pointer
    return FVariable();
}

void PyAnalyzer::SyntaxNode::Print(int Depth)
{
    std::cout << " " << std::string(std::max(0, Depth - 1), '\t') << Token.ValueName << std::endl;

    for (auto Child : Children) {
         Child->Print(Depth + 1);
    }
}

int PyAnalyzer::FVariable::Print(bool doNewLine)
{
    if (Type == "int")
    {
        int val = *reinterpret_cast<int*>(Value);
        std::cout << val;
    }
    else if (Type == "float")
    {
        float val = *reinterpret_cast<float*>(Value);
        std::cout << val;
    }
    else if (Type == "string")
    {
        std::string val = *reinterpret_cast<std::string*>(Value);
        std::cout << val;
    }else if (Type == "complex")
    {
        std::complex<float> val = *reinterpret_cast<std::complex<float>*>(Value);
        std::cout << val;
    }
    else if (Type == "array")
    {
        std::vector<FVariable> val = *reinterpret_cast<std::vector<FVariable>*>(Value);
        
        std::cout << "[";

        if (val.size())
        {
            val[0].Print(false);
        }

        for (int i = 0; i < val.size(); i++)
        {
            std::cout << ", ";
            val[i].Print(false);
        }
        std::cout << "]";
    }

    if (doNewLine) std::cout << "\n";

    return 0;
}

PyAnalyzer::FVariable PyAnalyzer::CallPrint(std::shared_ptr<SyntaxNode> Node)
{
    for (auto child : Node->Children)
    {
        auto el = ExecExpr(child);
        if (el.Type == "") return FVariable();
        el.Print(false);
    }
    std::cout << "\n";
    return FVariable("", "int", new int(1));
}

PyAnalyzer::FVariable PyAnalyzer::CallInt(std::shared_ptr<SyntaxNode> Node)
{
    auto el = ExecExpr(Node->Children[0]);
    if (el.Type == "") return FVariable();
    if (el.Type == "array")
    {
        Errors.push_back(Error("Incorrect int() arg type, cannot covert : " + el.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return FVariable();
    }
    else if (el.Type == "string")
    {
        el =  GetNumFromString(*reinterpret_cast<std::string*>(el.Value));
        if (el.Type == "") return FVariable();
    }
    
    if (el.Type == "Complex")
    {
        return FVariable("", "int", new int((*reinterpret_cast<std::complex<float>*>(el.Value)).real()));
    }
    else if (el.Type == "float")
    {
        return FVariable("", "int", new int(*reinterpret_cast<float*>(el.Value)));

    }
    else if (el.Type == "int")
    {
        return el;
    }

    return FVariable();
}

PyAnalyzer::FVariable PyAnalyzer::CallInput(std::shared_ptr<SyntaxNode> Node)
{
    if (Node->Children.size())
    {
        auto el = ExecExpr(Node->Children[0]);
        if (el.Type == "") return FVariable();
        if (el.Type != "string")
        {
            Errors.push_back(Error("Incorrect input() arg type, should be string, but found : " + el.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return FVariable();
        }
        std::cout << *reinterpret_cast<std::string*>(el.Value);
    }
    std::string line;
    std::getline(std::cin, line);
    return FVariable("", "string", new std::string(line));
}

PyAnalyzer::FVariable PyAnalyzer::CallFloat(std::shared_ptr<SyntaxNode> Node)
{
    auto el = ExecExpr(Node->Children[0]);
    if (el.Type == "") return FVariable();
    if (el.Type == "array")
    {
        Errors.push_back(Error("Incorrect float() arg type, cannot covert : " + el.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return FVariable();
    }
    else if (el.Type == "string")
    {
        el = GetNumFromString(*reinterpret_cast<std::string*>(el.Value));
        if (el.Type == "") return FVariable();
    }

    if (el.Type == "Complex")
    {
        return FVariable("", "float", new float((*reinterpret_cast<std::complex<float>*>(el.Value)).real()));
    }
    else if (el.Type == "float")
    {
        return el;

    }
    else if (el.Type == "int")
    {
        return FVariable("", "float", new float(*reinterpret_cast<int*>(el.Value)));
    }

    return FVariable();
}

PyAnalyzer::FVariable PyAnalyzer::CallArray(std::shared_ptr<SyntaxNode> Node)
{
    std::vector<FVariable>* arr = new std::vector<FVariable>();
    for (auto i : Node->Children)
    {
        auto var = ExecExpr(i);
        if (var.Type == "") return FVariable();
        arr->push_back(var);
    }
    return FVariable("", "array", arr);
}

PyAnalyzer::FVariable PyAnalyzer::CallType(std::shared_ptr<SyntaxNode> Node)
{
    auto el = ExecExpr(Node->Children[0]);
    if (el.Type == "") return FVariable();
    return FVariable("", "string", new std::string(el.Type));
}

PyAnalyzer::FVariable PyAnalyzer::CallString(std::shared_ptr<SyntaxNode> Node)
{
    auto el = ExecExpr(Node->Children[0]);
    if (el.Type == "") return FVariable();
    if (el.Type == "array")
    {
        Errors.push_back(Error("Incorrect string() arg type, cannot covert : " + el.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return FVariable();
    }
    else if (el.Type == "string")
    {
        return el;
    }

    if (el.Type == "Complex")
    {
        std::complex<float> c = *reinterpret_cast<std::complex<float>*>(el.Value);
        std::string res = std::to_string(c.real()) + " + " + std::to_string(c.imag()) + "i";
        return FVariable("", "string", new std::string(res));
    }
    else if (el.Type == "float")
    {
        return FVariable("", "string", new std::string(std::to_string( *reinterpret_cast<float*>(el.Value))));

    }
    else if (el.Type == "int")
    {
        return FVariable("", "string", new std::string(std::to_string(*reinterpret_cast<int*>(el.Value))));
    }

    return FVariable();
}

PyAnalyzer::FVariable PyAnalyzer::CallComplex(std::shared_ptr<SyntaxNode> Node)
{
    auto el = ExecExpr(Node->Children[0]);
    if (el.Type == "") return FVariable();
    if (el.Type == "array")
    {
        Errors.push_back(Error("Incorrect complex() arg type, cannot covert : " + el.Type + " at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
        return FVariable();
    }
    else if (el.Type == "string")
    {
        el = GetNumFromString(*reinterpret_cast<std::string*>(el.Value));
        if (el.Type == "") return FVariable();
    }

    if (el.Type == "Complex")
    {
        return el;
    }
    else if (el.Type == "float")
    {
        return FVariable("", "complex", new std::complex<float>(*reinterpret_cast<float*>(el.Value)));

    }
    else if (el.Type == "int")
    {
        return FVariable("", "complex", new std::complex<float>(*reinterpret_cast<int*>(el.Value)));
    }

    return FVariable();
}

std::pair<int, int> PyAnalyzer::ExecRange(std::shared_ptr<SyntaxNode> Node)
{
    int l, r;

    if (Node->Children.size() == 1)
    {
        l = 0;
        auto rval = CallInt(Node->Children[0]);
        if (rval.Type == "") return { -1, -1 };
        r = *reinterpret_cast<int*>(rval.Value);
    }
    else
    {
        auto lval = CallInt(Node->Children[0]);
        auto rval = CallInt(Node->Children[1]);
        if (rval.Type == "") return { -1, -1 };
        if (lval.Type == "") return { -1, -1 };
        l = *reinterpret_cast<int*>(lval.Value);
        r = *reinterpret_cast<int*>(rval.Value);
    }

    return { l, r };
}
