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

#pragma once

#include "nowork/Common.h"
#include <mutex>

//Super advanced macro definitions (kinda like overloading parameters)

#define GET_SCRIPT_LOG_MACRO(_1,_2,NAME,...) NAME
#define GET_LOG_MACRO(_1,_2,_3,NAME,...) NAME

#define L_ERR_3(a,b,c) { Log::Error(std::ostringstream() << a,b,c); }
#define L_ERR_2(a,b) { Log::Error(std::ostringstream() << a,b); }
#define L_ERR_1(a) { Log::Error(std::ostringstream() << a); }

/**
* Prints an error message to the log.
* Parameters: Message stream, Filename(Optional), Line(Optional)
*/
#define LOG_ERROR(...) GET_LOG_MACRO(__VA_ARGS__, L_ERR_3, L_ERR_2, L_ERR_1)(__VA_ARGS__)

#define L_WRN_3(a,b,c) { Log::Warning(std::ostringstream() << a,b,c); }
#define L_WRN_2(a,b) { Log::Warning(std::ostringstream() << a,b); }
#define L_WRN_1(a) { Log::Warning(std::ostringstream() << a); }

/**
* Prints a warning message to the log. 
* Parameters: Message stream, Filename(Optional), Line(Optional)
*/
#define LOG_WARNING(...) GET_LOG_MACRO(__VA_ARGS__, L_WRN_3, L_WRN_2, L_WRN_1)(__VA_ARGS__)

#define L_DBG_3(a,b,c) { Log::Debug(std::ostringstream() << a,b,c); }
#define L_DBG_2(a,b) { Log::Debug(std::ostringstream() << a,b); }
#define L_DBG_1(a) { Log::Debug(std::ostringstream() << a); }

/**
* Prints a debug message to the log. Only printed when building in debug mode.
* Parameters: Message stream, Filename(Optional), Line(Optional)
*/
#define LOG_DEBUG(...) GET_LOG_MACRO(__VA_ARGS__, L_DBG_3, L_DBG_2, L_DBG_1)(__VA_ARGS__)

#define L_MSG_3(a,b,c) { Log::Print(std::ostringstream() << a,b,c); }
#define L_MSG_2(a,b) { Log::Print(std::ostringstream() << a,b); }
#define L_MSG_1(a) { Log::Print(std::ostringstream() << a); }

/**
* Prints a message to the log.
* Parameters: Message stream, Filename(Optional), Line(Optional)
*/
#define LOG_MESSAGE(...) GET_LOG_MACRO(__VA_ARGS__, L_MSG_3, L_MSG_2, L_MSG_1)(__VA_ARGS__)


#define L_SCRERR_2(a,b) { Log::ScriptError(std::ostringstream() << a,b); }
#define L_SCRERR_1(a) { Log::ScriptError(std::ostringstream() << a); }

#define LOG_SCRIPT_ERROR(...) GET_SCRIPT_LOG_MACRO(__VA_ARGS__, L_SCRERR_2, L_SCRERR_1)(__VA_ARGS__)

	
/**
* Outputs a message, warning or error to stdout
* also provides functionality to get the message stream to output it it i.e. in editor
* or ingame console. Might be need to extended.
*
* Logging is pretty time intensive. So dont wonder if performance decreases when used often (or in loops)
**/
class Log
{
	friend class NoWork;

public:

	enum class Color
	{
		Red = 0x0C,
		Yellow = 0x0E,
		White = 0x07
	};

	NOWORK_API Log();
	NOWORK_API ~Log();
		
	NOWORK_API static void Print(std::ostream &os, std::string functionName = "", int line = -1);
	NOWORK_API static void Debug(std::ostream &os, std::string functionName = "", int line = -1);
	NOWORK_API static void Warning(std::ostream &os, std::string functionName = "", int line = -1);
	NOWORK_API static void Error(std::ostream &message, std::string functionName = "", int line = -1);
	NOWORK_API static void ScriptError(std::ostream &message, std::string functionName = "");
	NOWORK_API static void Clear();
	NOWORK_API static void SetTextColor(Color color);

	NOWORK_API static std::ostream& GetMessageStream();
		
protected:

private:
	static void PrintMsgHeader(std::ostream &ss, std::string category, std::string functionName = "", int line = -1);
	static void GetCurrTime(std::ostream *targetStringStream);

	//Share this variable so it doesn't get instanciated per process
	static std::ostream *m_MessageStream;
	static std::mutex m_Mutex;
};

	
/*******************************************************************************
	Log inline implementation:
*******************************************************************************/


inline void Log::Clear()
{
	m_MessageStream->clear();
}

inline std::ostream& Log::GetMessageStream()
{
	return *m_MessageStream;
}

inline Log::Log()
{
#ifdef _DEBUG
	m_MessageStream = &std::cout;
#else
	m_MessageStream = &std::cout;
	//m_MessageStream = new std::stringstream;
#endif

	m_MessageStream->clear();
}

inline Log::~Log()
{
	Clear();

#ifndef _DEBUG
	//DelPtr(m_MessageStream);
#endif
}