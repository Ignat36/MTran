#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <complex>

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

    std::complex<float> a = 1;
    std::complex<float> b = 1.1;
    std::complex<float> c = 1.00900000009991;

    void* as = &a;
    void* bs = &b;
    void* cs = &c;
    
    std::complex<float> ar = *reinterpret_cast<std::complex<float>*>(as);
    std::complex<float> br = *reinterpret_cast<std::complex<float>*>(bs);
    std::complex<float> cr = *reinterpret_cast<std::complex<float>*>(cs);

    std::cout << ar + br + cr;

    //Analyzer.PrintSyntaxTree();
    
    return 0;
}