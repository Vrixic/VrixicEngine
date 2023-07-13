/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdarg.h>
//#include <iostream>
//#include <fstream>

std::shared_ptr<spdlog::logger> Log::CoreLogger;
std::shared_ptr<spdlog::logger> Log::ClientLogger;

void Log::Init()
{
	// set the logger pattern 
	spdlog::set_pattern("%^[%T] %n: %v%$");

	// create the core engine logger 
	CoreLogger = spdlog::stdout_color_mt("VRIXIC");
	CoreLogger->set_level(spdlog::level::trace);

	// create a client logger 
	ClientLogger = spdlog::stdout_color_mt("APP");
	ClientLogger->set_level(spdlog::level::trace);
}

void Log::LogCoreMsg(ELogSeverity inSeverity...)
{
	va_list Args;
	va_start(Args, inSeverity);
	const char* Message = va_arg(Args, const char*);

	//std::ofstream FileHandle;
	//FileHandle.open("Log.txt", std::ios::app);
	//FileHandle << Message << "\n";
	//FileHandle.close();

	switch (inSeverity)
	{
	case ELogSeverity::Display:
		CoreLogger->trace(Message);
		break;
	case ELogSeverity::Info:
		CoreLogger->info(Message);
		break;
	case ELogSeverity::Warn:
		CoreLogger->warn(Message);
		break;
	case ELogSeverity::Error:
		CoreLogger->error(Message);
		break;
	case ELogSeverity::Fatal:
		CoreLogger->critical(Message);
		break;
	}
}

void Log::LogClientMsg(ELogSeverity inSeverity...)
{
	va_list Args;
	va_start(Args, inSeverity);
	const char* Message = va_arg(Args, const char*);

	switch (inSeverity)
	{
	case ELogSeverity::Display:
		ClientLogger->trace(Message);
		break;
	case ELogSeverity::Info:
		ClientLogger->info(Message);
		break;
	case ELogSeverity::Warn:
		ClientLogger->warn(Message);
		break;
	case ELogSeverity::Error:
		ClientLogger->error(Message);
		break;
	case ELogSeverity::Fatal:
		ClientLogger->critical(Message);
		break;
	}
}
