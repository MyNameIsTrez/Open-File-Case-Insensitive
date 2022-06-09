#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <cassert>

class System
{
public:
	static const std::string &GetWorkingDirectory() { return s_WorkingDirectory; }

private:
	static std::string s_WorkingDirectory;
};

std::string System::s_WorkingDirectory = "cccp";

bool StringsEqualCaseInsensitive(const std::string &a, const std::string &b)
{
	return std::equal(a.begin(), a.end(),
					  b.begin(), b.end(),
					  [](char a, char b)
					  { return std::tolower(a) == std::tolower(b); });
}

FILE *OpenFileCaseInsensitive(const std::filesystem::path filePath, const std::string &mode)
{
	std::filesystem::path inspectedPath{System::GetWorkingDirectory()};

	for (std::filesystem::path::const_iterator filePathIterator = filePath.begin(); filePathIterator != filePath.end(); ++filePathIterator)
	{
		bool pathPartExists = false;

		for (const std::filesystem::path &filesystemEntryPath : std::filesystem::directory_iterator{inspectedPath})
		{
			if (StringsEqualCaseInsensitive(filesystemEntryPath.filename().string(), (*filePathIterator).string()))
			{
				inspectedPath = filesystemEntryPath;
				pathPartExists = true;
				break;
			}
		}

		if (!pathPartExists)
		{
			// If this is the last part, then all directories in filePath exist, but the file doesn't.
			if (std::next(filePathIterator) == filePath.end())
			{
				return fopen((inspectedPath / filePath.filename()).string().c_str(), mode.c_str());
			}

			// Some directory in filePath doesn't exist, so the file can't be created.
			return nullptr;
		}
	}

	// If the file exists, open it.
	return fopen(inspectedPath.string().c_str(), mode.c_str());
}

void TestCaseInsensitivity()
{
	FILE *a = OpenFileCaseInsensitive("f1", "w");
	fwrite("foo", 1, 3, a);
	fclose(a);

	char buffer[42];
	FILE *b = OpenFileCaseInsensitive("F1", "r");
	fgets(buffer, 42, b);
	assert(buffer == "foo");
}

int main()
{
	std::filesystem::path cc_game_path{System::GetWorkingDirectory()};
	std::filesystem::create_directories(cc_game_path);

	TestCaseInsensitivity();

	// std::filesystem::create_directories(cc_game_path / "a1" / "b1");
	// std::filesystem::create_directories(cc_game_path / "a2");

	std::filesystem::remove_all(cc_game_path);

	return (0);
}
