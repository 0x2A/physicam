#include "NoWork/FileSystem.h"
#include "NoWork/Log.h"
#include <fstream>
#include <streambuf>

std::string FileSystem::GetPath(const std::string& path)
{
	std::string fName(path);
	size_t pos = fName.rfind("/");
	if (pos == std::string::npos)
	{
		pos = fName.rfind("\\");
		if (pos == std::string::npos)
			return fName;
	}

	if (pos == 0)    // / is at the front.
		return fName;

	return fName.substr(0, pos);
}

std::string FileSystem::GetFilename(const std::string& path)
{
	std::string fName(path);
	size_t pos = fName.rfind("/");
	if (pos == std::string::npos)
	{
		pos = fName.rfind("\\");
		if (pos == std::string::npos)
			return fName;
	}

	if (pos == 0)    // / is at the front.
		return fName;

	return fName.substr(pos + 1, fName.size() - (pos + 1));
}

std::string FileSystem::LoadTextFile(const std::string& path)
{
	std::ifstream t(path, std::ios::in);
	if (!t.is_open())
	{
		std::stringstream ss;
		ss << "Unable to open file '" << path << "': " << std::strerror(errno);
		throw std::exception(ss.str().c_str());
	}

	std::string src((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	t.close();
	return src;
}

