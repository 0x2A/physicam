#pragma once

#include "NoWork/Common.h"


class FileSystem
{
public:

	NOWORK_API static std::string GetPath(const std::string& path);
	NOWORK_API static std::string GetFilename(const std::string& path);
	NOWORK_API static std::string LoadTextFile(const std::string& path);

};