/*
*  ===========================================================================
*	This file is part of CrystalEngine source code.
*   Copyright (C) 2013  Frank Köhnke
*
*   This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*  ===========================================================================
*/

#include "nowork/Log.h"
#include "nowork/Common.h"

#include <time.h>

std::mutex Log::m_Mutex;



std::ostream *Log::m_MessageStream = NULL;

void Log::PrintMsgHeader(std::ostream &ss, std::string category, std::string functionName, int line)
{
	ss << "[";
	GetCurrTime(&ss);
	ss << "]";
	if (!category.empty())
		ss << "[" << category << "] "; 
	ss << functionName;
	if (line != -1)
		ss << " on line " << line;
	ss << ": ";
}

void Log::Print(std::ostream &os, std::string functionName /* = "" */, int line /* = -1 */)
{
	std::ostringstream& s = dynamic_cast<std::ostringstream&>(os);
	if (s)
	{
		m_Mutex.lock();
		PrintMsgHeader(*m_MessageStream, functionName, "", line);
		*m_MessageStream << s.str() << std::endl;
		m_Mutex.unlock();
	}
}


void Log::Debug(std::ostream &os, std::string functionName, int line)
{
#ifdef _DEBUG
	Print(os, functionName, line);
#endif
}


void Log::Warning(std::ostream &message, std::string functionName /* = "" */, int line /* = -1 */)
{
	std::ostringstream& s = dynamic_cast<std::ostringstream&>(message);
	if (s)
	{
		m_Mutex.lock();
		SetTextColor(Color::Yellow);
		PrintMsgHeader(*m_MessageStream, "WARNING", functionName, line);
		*m_MessageStream << s.str() << std::endl;
		SetTextColor(Color::White);
		m_Mutex.unlock();
	}
}


void Log::Error(std::ostream &message, std::string functionName /* = "" */, int line /* = -1 */)
{
	std::ostringstream& s = dynamic_cast<std::ostringstream&>(message);
	if (s)
	{
		m_Mutex.lock();
		SetTextColor(Color::Red);
		PrintMsgHeader(*m_MessageStream, "ERROR", functionName, line);
		*m_MessageStream << s.str() << std::endl;
		SetTextColor(Color::White);
		m_Mutex.unlock();
	}
}

void Log::ScriptError(std::ostream &message, std::string functionName /*= ""*/)
{
	std::ostringstream& s = dynamic_cast<std::ostringstream&>(message);
	if (s)
	{
		m_Mutex.lock();
		SetTextColor(Color::Red);
		PrintMsgHeader(*m_MessageStream, "SCRIPT ERROR", functionName);
		*m_MessageStream << s.str() << std::endl;
		SetTextColor(Color::White);
		m_Mutex.unlock();
	}
}

void Log::GetCurrTime(std::ostream *targetStringStream)
{
	time_t t = time(0); // get time now
	struct tm now;	
	localtime_s(&now, &t);
	if(now.tm_hour < 10)
		*targetStringStream << 0;
	*targetStringStream << now.tm_hour << ':';
	if (now.tm_min < 10)
		*targetStringStream << 0;
	*targetStringStream << now.tm_min << ':';
	if (now.tm_sec < 10)
		*targetStringStream << 0;
	*targetStringStream << now.tm_sec;
}

void Log::SetTextColor(Color color)
{
#if defined(_WIN32) || defined(_WIN64)
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color)); //http://www.cplusplus.com/forum/beginner/5830/#msg25972 for more details
#else
	//TODO
#endif
}

