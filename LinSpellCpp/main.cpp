#include <iostream>
#include <chrono>
#include "LinSpell.h"
using namespace std;

void Benchmark(const std::string& pathh, int testNumber) {
    LinSpell* spell = new LinSpell();
    string path = "frequency_dictionary_en_82_765.txt";
    if (!spell->LoadDictionary(path, "", 0, 1)) throw new exception;

    int resultSum = 0;
    std::vector<std::string> testList(testNumber);

    // Load 1000 terms with random spelling errors
    std::ifstream file(pathh);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << pathh << std::endl;
        return;
    }

    std::string line;
    int i = 0;
    while (std::getline(file, line) && i < testNumber) {
        std::istringstream lineStream(line);
        std::string key;
        lineStream >> key;
        if (!key.empty()) {
            testList[i++] = key;
        }
    }
    file.close();

    auto start = std::chrono::high_resolution_clock::now();
    // Perform 'rounds' of lookups
    const int rounds = 10;
    for (int j = 0; j < rounds; ++j) {
        resultSum = 0;
        for (int k = 0; k < testNumber; ++k) {
            auto suggestions = spell->LookupLinear(testList[k], Verbosity::Top, "", 2);
            resultSum += suggestions.size();
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() / rounds;

    std::cout << resultSum << " results in " << duration << " ms" << std::endl;
}
void ConsoleTest() {
    LinSpell* spell = new LinSpell();
    string path = "frequency_dictionary_en_82_765.txt";
    if (!spell->LoadDictionary(path, "", 0, 1)) throw new exception;

    string input;
    while (true) {
        // Prompt the user for input
        std::cout << "Enter text: ";
        std::getline(std::cin, input);

        // Trim leading and trailing whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);

        // Break the loop if the input is empty
        if (input.empty()) {
            break;
        }
        auto start = std::chrono::high_resolution_clock::now();



        auto corrected = spell->LookupLinear(input, Verbosity::Top, "", 2);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;

        string correctedString = corrected.empty() ? "NO MATCH WAS FOUND" : corrected[0].term;
        std::cout << "done in " << elapsed.count() << "ms" << std::endl;

        cout << "your input was : " << input << endl;
        cout << "your corrected string is : " << correctedString << endl;

    }
}
int main()
{
    //Benchmark("./test/noisy_query_en_1000.txt", 1000);
    ConsoleTest();
}



