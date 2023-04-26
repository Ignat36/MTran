#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <complex>

#include "Analyzer.h"

int main()
{
    //std::string FileName = "test.py";
    std::string FileName = "sort.py";
    //std::string FileName = "test_ok.py";

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
    //std::cout << "Compiled successfully!";
    
    return 0;
}