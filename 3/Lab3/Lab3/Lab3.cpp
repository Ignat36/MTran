#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "Analyzer.h"

int main()
{
    //std::string FileName = "sort.py";
    //std::string FileName = "test.py";
    std::string FileName = "test_ok.py";

    std::ifstream FileStream; FileStream.open(FileName);

    std::string Line;
    std::vector<std::string> Code;
    while (std::getline(FileStream, Line))
    {
        Code.push_back(Line);
    }
    FileStream.close();

    PyAnalyzer Analyzer(Code);

    auto Errors = Analyzer.GetErrors();
    auto Tokens = Analyzer.GetTokens();

    for (auto i : Errors)
    {
        std::cout << FileName << " : " << i.Message << "\n";
        return 0;
    }
    

    Analyzer.PrintSyntaxTree();

    /*std::unordered_set<std::string> set;
    
    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Key Words                         \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::KeyWord && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Constants                         \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Number && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Literals                          \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Literal && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Variables                         \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Variable && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Operators                         \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Operator && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Built-in                          \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Function && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";

    set.clear();
    std::cout << "----------------------------------------------\n";
    std::cout << "            Delimeters                        \n";
    std::cout << "----------------------------------------------\n";
    for (auto i : Tokens)
    {
        if (i.TokenType == PyAnalyzer::ETokenType::Delimeter && !set.count(i.ValueName))
        {
            std::cout << "\t" << i.ValueName << "\n";
            set.insert(i.ValueName);
        }
    }
    std::cout << "----------------------------------------------\n\n\n";*/
    /*for (auto i : Tokens)
    {
        std::cout << i.ValueName << " | " << i.Description << " | at " << i.RowIndex << ":" << i.ColumnIndex << "\n";
    }*/
    
    return 0;
}