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
    checkSingleTokenDependensies();
    checkBrackets();
    SyntaxAnalisis(); 
    ReformatSyntaxTree();
    checkSyntaxTree();
    SemanticAnalisis();
    return true;
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
    std::shared_ptr<SyntaxNode> current = SyntaxTree;;
    int deepth = 0;
    
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
                Errors.push_back(Error("incorrect code deepth change at line " + std::to_string(i)));
            }

            if (diff == -1 && Tokens[i-1].ValueName != ":")
            {
                Errors.push_back(Error("Unexpected indent at line " + std::to_string(i)));
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
            // Create a new node and add it as a child of the current node
            /*std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);*/
            // Set the current node to be the new child
            current = current->Children.back();
        }
        else if (Tokens[i].ValueName == ")") {
            // Move back up to the parent node
            current = current->Parent.lock();

            /*std::shared_ptr<SyntaxNode> child = std::make_shared<SyntaxNode>(Tokens[i], current);
            current->Children.push_back(child);*/
        } 
        else if (Tokens[i].TokenType == ETokenType::Operator)
        {
            ProcessExpression(i, current);
        }
        else if (Tokens[i].TokenType == ETokenType::KeyWord)
        {
            ProcessKeyWord(i, current);
        }
        else if (Tokens[i].ValueName == ",")
        {
            // Check for smthing enumeration
            continue;
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

void PyAnalyzer::ProcessExpression(int& x, std::shared_ptr<SyntaxNode>& Node)
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
        }
    }
    else
    {
        Node = Node->Children.back();
        std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
        Node->Children.push_back(Child);
        Node = Child;
    }
}

void PyAnalyzer::ProcessKeyWord(int& x, std::shared_ptr<SyntaxNode>& Node)
{
    std::shared_ptr<SyntaxNode> Child = std::make_shared<SyntaxNode>(Tokens[x], Node);
    Node->Children.push_back(Child);
    Node = Child;
    return;
}

void PyAnalyzer::ReformatSyntaxTree()
{
    for (int i = 0; i < SyntaxTree->Children.size(); i++)
    {
        ReformatSyntaxNode(SyntaxTree->Children[i], SyntaxTree, i);
    }
}

void PyAnalyzer::ReformatSyntaxNode(std::shared_ptr<SyntaxNode> Node, std::shared_ptr<SyntaxNode> Parent, int p_child_index)
{
    if (Node->Token.TokenType == ETokenType::Variable || Node->Token.TokenType == ETokenType::Literal || Node->Token.TokenType == ETokenType::Number)
    {
        if (Node->Children.size())
        {
            Parent->Children[p_child_index] = Node->Children.back();
            Node->Children.back()->Parent = Parent;
            Node->Parent = Node->Children.back();
            Node->Children.back()->Children.insert(Node->Children.back()->Children.begin(), Node);
            Node->Children.clear();
            Node = Parent->Children[p_child_index];
        }
    }

    for (int i = 0; i < Node->Children.size(); i++)
    {
        ReformatSyntaxNode(Node->Children[i], Node, i);
    }
}

void PyAnalyzer::checkSingleTokenDependensies()
{
    for (int i = 0; i < Tokens.size() - 1; i++)
    {

        if (Tokens[i + 1].RowIndex != Tokens[i].RowIndex)
        {
            if (Tokens[i + 1].TokenType == ETokenType::Operator)
            {
                Errors.push_back(Error(" found operator in the beginning of file : " + Tokens[i+1].ValueName + std::string(" | at ") + std::to_string(Tokens[i+1].RowIndex) + ":" + std::to_string(Tokens[i+1].ColumnIndex)));
            }
        }

        if (Tokens[i].ValueName == ":")
        {
            if (Tokens[i + 1].RowIndex == Tokens[i].RowIndex)
            {
                Errors.push_back(Error(" colon ':' must be followed by new string with additional tab, but found : " + Tokens[i].ValueName + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
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
            }
            break;
        case ETokenType::Function:
            if (Tokens[i + 1].ValueName != "(")
            {
                Errors.push_back(Error("Expected '(' symbol, but found : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
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
                }
                break;
            }

            if (Tokens[i].ValueName == "continue" || Tokens[i].ValueName == "break")
            {
                if (Tokens[i + 1].RowIndex == Tokens[i].RowIndex)
                {
                    Errors.push_back(Error(" colon ':' must be followed by new string with additional tab, but found : " + Tokens[i].ValueName + std::string(" | at ") + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
                }
            }

            break;

        case ETokenType::Operator:
            if (Tokens[i].RowIndex != Tokens[i + 1].RowIndex)
            {
                Errors.push_back(Error("Unexpected operator in the end of line : " + Tokens[i].ValueName + " | at " + std::to_string(Tokens[i].RowIndex) + ":" + std::to_string(Tokens[i].ColumnIndex)));
            }
            if (Tokens[i + 1].ValueName != "(" &&
                 (Tokens[i + 1].TokenType == ETokenType::KeyWord
                || Tokens[i + 1].TokenType == ETokenType::Operator
                || Tokens[i + 1].TokenType == ETokenType::Delimeter)
                )
            {
                Errors.push_back(Error("Unexpected token : " + Tokens[i + 1].ValueName + " | at " + std::to_string(Tokens[i + 1].RowIndex) + ":" + std::to_string(Tokens[i + 1].ColumnIndex)));
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
    }
}

void PyAnalyzer::checkBrackets()
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
                    break;
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
                    break;
                }
            }
        }
    }
}

void PyAnalyzer::checkSyntaxTree()
{
}

void PyAnalyzer::SyntaxNode::Print(int Depth)
{
    std::cout << std::string(std::max(0, Depth-1), '\t') << Token.ValueName << std::endl;

    for (auto Child : Children) {
         Child->Print(Depth + 1);
    }
}
