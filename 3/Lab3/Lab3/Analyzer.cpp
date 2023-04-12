#include "Analyzer.h"

#include <iostream>
#include <fstream>
#include <stack>

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
    LexicalAnalisis();
    checkSingleTokenDependensies();
    checkBrackets();
    SyntaxAnalisis();
    ReformatSyntaxTree();
    checkSyntaxTree();
    SemanticAnalisis();
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
            if (Variables.count(Token))
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
                Variables.insert(Token);
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
            if (FunctionsStack.size() && FunctionsStack.back()->Token.ValueName == "[]")
                FunctionsStack.pop_back();

            current = current->Parent.lock();
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

    for (; x < Code.size(); x++)
    {
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
        Variables.insert(Name);
        Tokens.push_back(FToken(Name, "variable", x, y, ETokenType::Variable));
    }

    return 0;
}

int PyAnalyzer::ProcessExpression(int& x, std::shared_ptr<SyntaxNode>& Node)
{

    if (AssigmentOperators.count(Tokens[x].ValueName))
    {
        if (x - 1 >= 0 && Tokens[x - 1].TokenType == ETokenType::Variable)
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
        }
    }
    else if (Node->Token.ValueName == "(")
    {
        Parent->Children[p_child_index] = BuildExpressionTree(Node);
        Node = Parent->Children[p_child_index];
    }

    if (!Node) return 0;

    for (int i = 0; i < Node->Children.size(); i++)
    {
        ReformatSyntaxNode(Node->Children[i], Node, i);
    }

    if (Node->Token.ValueName == "for")
    {
        if (Node->Children[0]->Token.ValueName != "in")
        {
            Errors.push_back(Error(" Incorrect for notation at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
            return 1;
        }
    }

    if (Node->Token.ValueName == "if" || Node->Token.ValueName == "while" || Node->Token.ValueName == "elif")
    {
        if (Node->Children[0]->Token.TokenType != ETokenType::Number
            && Node->Children[0]->Token.TokenType != ETokenType::Variable
            && Node->Children[0]->Token.TokenType != ETokenType::Function
            && Node->Children[0]->Token.TokenType != ETokenType::Operator
            )
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
        // something but probably leave for semantic analise
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
        Errors.push_back(Error("Unreached delimeter " + Node->Token.ValueName + " : at | " + std::to_string(Node->Token.RowIndex) + ":" + std::to_string(Node->Token.ColumnIndex)));
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

void PyAnalyzer::SyntaxNode::Print(int Depth)
{
    std::cout << " " << std::string(std::max(0, Depth - 1), '\t') << Token.ValueName << std::endl;

    for (auto Child : Children) {
         Child->Print(Depth + 1);
    }
}
