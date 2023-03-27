#include "LexicalAnalizer.h"

#include <iostream>
#include <fstream>

LexicalAnalizer::LexicalAnalizer(std::vector<std::string>& _Code) : Code(_Code)
{
    Analyze();
}

bool LexicalAnalizer::Analyze()
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
            if (c == ' ' || c =='\t') continue;
            MakeFalse = true;

            if (Delimiters.count(c))
            {
                Token = c;
                Tokens.push_back(Row(Token, "delimeter", i, j));
                continue;
            }
            if ((c >= '0' && c <= '9') || c == '.')
            {
                Token = ReadNumberConstant(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(Row(Token, "constant", i, j));
                continue;
            }
            if (c == '\"' || c == '\'')
            {
                Token = ReadLiteral(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(Row(Token, "literal", i, j));
                continue;
            }
            if (OperatorsCharacters.count(c))
            {
                Token = ReadOperator(i, j, ReturnFlag);
                if (!ReturnFlag) Tokens.push_back(Row(Token, "operator", i, j));
                continue;
            }


            Token = ReadWord(i, j, ReturnFlag);
            if (ReturnFlag) continue;


            if (Keywords.count(Token))
            {
                KeywordsTable.Rows.push_back(Row(Token, "key word", i, j));
                Tokens.push_back(Row(Token, "key word", i, j));

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
                FunctionsTable.Rows.push_back(Row(Token, "build-in function", i, j));
                Tokens.push_back(Row(Token, "build-in function", i, j));
                continue;
            }
            if (BuiltinTypes.count(Token)) 
            {
                KeywordsTable.Rows.push_back(Row(Token, "build-in type", i, j));
                Tokens.push_back(Row(Token, "build-in type", i, j));
                continue;
            }
            if (Operators.count(Token)) 
            {
                OperatorsTable.Rows.push_back(Row(Token, "operator", i, j));
                Tokens.push_back(Row(Token, "operator", i, j));
                continue; 
            }
            if (Variables.count(Token))
            {
                VariablesTable.Rows.push_back(Row(Token, "variable", i, j));
                Tokens.push_back(Row(Token, "variable", i, j));
                continue; 
            }
            if (Functions.count(Token))
            { 
                OperatorsTable.Rows.push_back(Row(Token, "function", i, j));
                Tokens.push_back(Row(Token, "function", i, j));
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
                Tokens.push_back(Row(Token, "variable", i, j));
                continue;
            }
        }
    }
    return true;;
}

std::string LexicalAnalizer::ReadNumberConstant(int& x, int& y, bool& Flag)
{
    std::string res = "";
    bool WasDot = false;
    bool WasComplexJ = false;
    bool BadToken = false;
    
    for (; y < Code[x].size(); y++)
    {
        char c = Code[x][y];
        
        if (Delimiters.count(c))
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

std::string LexicalAnalizer::ReadOperator(int& x, int& y, bool& Flag)
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

std::string LexicalAnalizer::ReadWord(int& x, int& y, bool& Flag)
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

std::string LexicalAnalizer::ReadLiteral(int& x, int& y, bool& Flag)
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

void LexicalAnalizer::ReadFunctionSignature(int& x, int& y, bool& Flag)
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
        Tokens.push_back(Row(Name, "function", x, y));
    }
}

void LexicalAnalizer::ReadForSignature(int& x, int& y, bool& Flag)
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
        Tokens.push_back(Row(Name, "variable", x, y));
    }
}
