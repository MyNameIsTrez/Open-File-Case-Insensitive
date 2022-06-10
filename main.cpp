#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <cstring>
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
			if (StringsEqualCaseInsensitive(filesystemEntryPath.filename(), (*filePathIterator)))
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
				return fopen((inspectedPath / filePath.filename()).c_str(), mode.c_str());
			}

			// Some directory in filePath doesn't exist, so the file can't be created.
			return nullptr;
		}
	}

	// If the file exists, open it.
	return fopen(inspectedPath.c_str(), mode.c_str());
}

void TestFileCaseInsensitivityCreationReadingWritingAppending(void)
{
	FILE *filePointer1 = OpenFileCaseInsensitive("f1", "w");
	fwrite("foo", 1, 3, filePointer1);
	fclose(filePointer1);

	char buffer[42];
	memset(buffer, 0, 42);

	// a+ means 1. read and write 2. without truncating 3. starting at the end of the file.
	FILE *filePointer2 = OpenFileCaseInsensitive("F1", "a+");
	fwrite("bar", 1, 3, filePointer2);

	fseek(filePointer2, 0, SEEK_SET);
	std::string result = fgets(buffer, 42, filePointer2);

	std::string expected = "foobar";
	assert(result == expected);
}

void TestDirectoryDoesntExist(void)
{
	FILE *filePointer = OpenFileCaseInsensitive(std::filesystem::path("a1") / "f1", "w");
	assert(filePointer == nullptr);
}

void TestDirectoryCaseInsensitivity(void)
{
	std::filesystem::create_directories(std::filesystem::path(System::GetWorkingDirectory()) / "a1");

	FILE *filePointer1 = OpenFileCaseInsensitive(std::filesystem::path("a1") / "f1", "w");
	fwrite("foo", 1, 3, filePointer1);
	fclose(filePointer1);

	char buffer[42];
	memset(buffer, 0, 42);

	FILE *filePointer2 = OpenFileCaseInsensitive(std::filesystem::path("A1") / "f1", "a+");
	fwrite("bar", 1, 3, filePointer2);

	fseek(filePointer2, 0, SEEK_SET);
	std::string result = fgets(buffer, 42, filePointer2);

	std::string expected = "foobar";
	assert(result == expected);
}

int main(void)
{
	std::filesystem::remove_all(System::GetWorkingDirectory());

	std::filesystem::create_directories(System::GetWorkingDirectory());

	TestFileCaseInsensitivityCreationReadingWritingAppending();
	TestDirectoryDoesntExist();
	TestDirectoryCaseInsensitivity();

	std::filesystem::remove_all(System::GetWorkingDirectory());

	return (0);
}
