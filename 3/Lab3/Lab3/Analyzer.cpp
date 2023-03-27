#include "Analyzer.h"

#include <iostream>
#include <fstream>

PyAnalyzer::PyAnalyzer(std::vector<std::string>& _Code) : Code(_Code)
{
    Analyze();
}

void PyAnalyzer::PrintSyntaxTree()
{
    SyntaxTree->Print();
}

void PyAnalyzer::PrintTablesForLab()
{
    std::unordered_set<std::string> un;

    std::string line(80, '-'); line += "\n";
    std::cout << line << "  Key words:\n" << line;
    for (auto& i : Tokens)
    {
        if (i.TokenType == ETokenType::KeyWord)
        {
            if (un.count(i.ValueName) == 0)
            {
                std::cout << "     " << i.ValueName << "\n";
                un.insert(i.ValueName);
            }
        }
    }
    std::cout << line; un.clear();

    std::cout << line << "  Constants:\n" << line;
    for (auto& i : Tokens)
    {
        if (i.TokenType == ETokenType::Number)
        {
            if (un.count(i.ValueName) == 0)
            {
                std::cout << "     " << i.ValueName << "\n";
                un.insert(i.ValueName);
            }
        }
    }
    std::cout << line; un.clear();

    std::cout << line << "  Literals:\n" << line;
    for (auto& i : Tokens)
    {
        if (i.TokenType == ETokenType::Literal)
        {
            if (un.count(i.ValueName) == 0)
            {
                std::cout << "     " << i.ValueName << "\n";
                un.insert(i.ValueName);
            }
        }
    }
    std::cout << line; un.clear();

    std::cout << line << "  Operators:\n" << line;
    for (auto& i : Tokens)
    {
        if (i.TokenType == ETokenType::Operator)
        {
            if (un.count(i.ValueName) == 0)
            {
                std::cout << "     " << i.ValueName << "\n";
                un.insert(i.ValueName);
            }
        }
    }
    std::cout << line; un.clear();

    std::cout << line << "  Variables:\n" << line;
    for (auto& i : Tokens)
    {
        if (i.TokenType == ETokenType::Variable)
        {
            if (un.count(i.ValueName) == 0)
            {
                std::cout << "     " << i.ValueName << "\n";
                un.insert(i.ValueName);
            }
        }
    }
    std::cout << line; un.clear();
}

int PyAnalyzer::Analyze()
{
    LexicalAnalisis();
    SyntaxAnalisis();
    SemanticAnalisis();
    return true;
}

int PyAnalyzer::LexicalAnalisis()
{
    std::string Token = "";

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
            if (c == ' ' || c == '\t') continue;
            MakeFalse = true;

            if (Delimiters.count(c))
            {
                Token = c;
                Tokens.push_back(FToken(Token, "delimeter", i, j, ETokenType::Delimeter));
                continue;
            }
            if ((c >= '0' && c <= '9') || c == '.')
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
            if (BuiltinTypes.count(Token))
            {
                KeywordsTable.Rows.push_back(FToken(Token, "build-in type", i, j));
                Tokens.push_back(FToken(Token, "build-in type", i, j, ETokenType::Type));
                continue;
            }
            if (Operators.count(Token))
            {
                OperatorsTable.Rows.push_back(FToken(Token, "operator", i, j));
                Tokens.push_back(FToken(Token, "operator", i, j, ETokenType::Operator));
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
    return 0;
}

int PyAnalyzer::SyntaxAnalisis()
{
    SyntaxTree = std::make_shared<SyntaxNode>(FToken("", "", 0, 0));
    SyntaxTree->Parent = SyntaxTree;
    std::shared_ptr<SyntaxNode> current = SyntaxTree;;
    int reset = 0;
    
    for (int i = 0; i < Tokens.size(); i++) {

        if (i && Tokens[i - 1].RowIndex != Tokens[i].RowIndex)
        {
            while (reset > 0) 
            {
                current = current->Parent.lock();
                reset--;
            }
        }

        if (Tokens[i].ValueName == "(") {
            // Create a new node and add it as a child of the current node
            std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);
            reset++;
            // Set the current node to be the new child
            current = current->Children.back();
        }
        else if (Tokens[i].ValueName == ")") {
            // Move back up to the parent node
            current = current->Parent.lock();
            reset--;

            std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);
        } 
        else if (Tokens[i].TokenType == ETokenType::Operator)
        {
            ProcessExpression(i, current, reset);
        }
        else if (Tokens[i].TokenType == ETokenType::KeyWord)
        {
            ProcessKeyWord(i, current, reset);
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

void PyAnalyzer::ReadFunctionSignature(int& x, int& y, bool& Flag)
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
    }

    y--;

    if (Flag)
    {
        Errors.push_back(Error(std::string("Incorrect function name : ") + std::to_string(x) + ":" + std::to_string(y)));
    }
    else
    {
        Functions.insert(Name);
        Tokens.push_back(FToken(Name, "function", x, y, ETokenType::Function));
    }
}

void PyAnalyzer::ReadForSignature(int& x, int& y, bool& Flag)
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
    }
    else
    {
        Variables.insert(Name);
        Tokens.push_back(FToken(Name, "variable", x, y, ETokenType::Variable));
    }
}

void PyAnalyzer::ProcessExpression(int& x, std::shared_ptr<SyntaxNode>& Node, int& reset)
{

    if (AssigmentOperators.count(Tokens[x].ValueName))
    {
        if (x - 1 >= 0 && Tokens[x - 1].TokenType == ETokenType::Variable)
        {
            Node = Node->Children.back(); reset++;
            std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
            Node->Children.push_back(Child);
            Node = Child; reset++;
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
        }
    }
    else
    {
        std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
        Node->Children.push_back(Child);
        Node = Child;
        reset++;
    }
}

void PyAnalyzer::ProcessKeyWord(int& x, std::shared_ptr<SyntaxNode>& Node, int& reset)
{
    std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
    Node->Children.push_back(Child);
    Node = Child;
    reset++;
    return;
}

void PyAnalyzer::SyntaxNode::Print(int Depth)
{
    std::cout << std::string(std::max(0, Depth-1), '\t') << Token.ValueName << std::endl;

    for (auto Child : Children) {
         Child->Print(Depth + 1);
    }
}
