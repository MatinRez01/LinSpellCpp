#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace Tests
{
	TEST_CLASS(Tests)
	{
	public:
		
		TEST_METHOD(LookupLinear_NoMatchesFound)
		{
			LinSpell* spell = new LinSpell();
			string path = "../LinSpellCpp/frequency_dictionary_en_82_765.txt";
			if (!spell->LoadDictionary(path, "", 0, 1)) throw new exception;

			std::string input = "nonexistentword";
			Verbosity verbose = Verbosity::All;
			std::string language = "";
			int editDistanceMax = 2;

			std::vector<SuggestItem> result = spell->LookupLinear(input, verbose, language, editDistanceMax);

			Assert::IsTrue(result.empty());
		}
		TEST_METHOD(LookupLinearReturnsHallForHallo)
		{
			LinSpell* spell = new LinSpell();
			string path = "../LinSpellCpp/frequency_dictionary_en_82_765.txt";
			if (!spell->LoadDictionary(path, "", 0, 1)) throw new exception;

			std::vector<SuggestItem> result = spell->LookupLinear("hallo", Verbosity::Top, "", 2);

			Assert::AreEqual(std::string("hall"), result[0].term);
		}
	};
}
