#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "LexicalAnalizer.h"

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

    LexicalAnalizer Analyzer(Code);

    auto Errors = Analyzer.GetErrors();
    auto Tokens = Analyzer.GetTokens();

    for (auto i : Errors)
    {
        std::cout << FileName << " : " << i.Message << "\n";
    }

    for (auto i : Tokens)
    {
        std::cout << i.ValueName << " | " << i.Description << " | at " << i.RowIndex << ":" << i.ColumnIndex << "\n";
    }

    FileStream.close();
    return 0;
}