/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once

#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

#include <memory.h>
#include <spdlog/spdlog.h>

enum class ELogSeverity
{
	Display = 0,
	Info,
	Warn,
	Error,
	Fatal
};

class VRIXIC_API Log
{
public:
	/**
	* Initializes both the core and client loggers 
	*/
	static void Init();

	/**
	* Logs a message on the core logger 
	* @param inServerity - the serverity of the log/message
	*/
	static void LogCoreMsg(ELogSeverity inSeverity...);

	/**
	* Logs a message on the client logger
	* @param inServerity - the serverity of the log/message
	*/
	static void LogClientMsg(ELogSeverity inSeverity...);

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
	{
		return CoreLogger;
	}

	inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
	{
		return ClientLogger;
	}

private:
	static std::shared_ptr<spdlog::logger> CoreLogger;
	static std::shared_ptr<spdlog::logger> ClientLogger;

};


// Macros for core logging 
// Generic - First param has to be ELogSeverity 
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
#define VE_CORE_LOG(severity, ...)						Log::LogCoreMsg(severity, __VA_ARGS__)
#define VE_CORE_LOG_DISPLAY(...)						Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_LOG_INFO(...)							Log::GetCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_LOG_WARN(...)							Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VE_CORE_LOG_ERROR(...)							Log::GetCoreLogger()->error(__VA_ARGS__)
#define VE_CORE_LOG_FATAL(...)							Log::GetCoreLogger()->critical(__VA_ARGS__)
#else
#define VE_CORE_LOG(severity, ...)						
#define VE_CORE_LOG_DISPLAY(...)						
#define VE_CORE_LOG_INFO(...)							
#define VE_CORE_LOG_WARN(...)							
#define VE_CORE_LOG_ERROR(...)							
#define VE_CORE_LOG_FATAL(...)							
#endif


// Macros for client logging
// Genric - First param has to be ELogSeverity 
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
#define VE_CLIENT_LOG(severity, ...)					Log::LogClientMsg(severity, __VA_ARGS__)
#define VE_CLIENT_LOG_DISPLAY(...)						Log::GetClientLogger()->trace(__VA_ARGS__)
#define VE_CLIENT_LOG_INFO(...)							Log::GetClientLogger()->info(__VA_ARGS__)
#define VE_CLIENT_LOG_WARN(...)							Log::GetClientLogger()->warn(__VA_ARGS__)
#define VE_CLIENT_LOG_ERROR(...)						Log::GetClientLogger()->error(__VA_ARGS__)
#define VE_CLIENT_LOG_FATAL(...)						Log::GetClientLogger()->critical(__VA_ARGS__)
#else
#define VE_CLIENT_LOG(severity, ...)
#define VE_CLIENT_LOG_DISPLAY(...)	
#define VE_CLIENT_LOG_INFO(...)		
#define VE_CLIENT_LOG_WARN(...)		
#define VE_CLIENT_LOG_ERROR(...)	
#define VE_CLIENT_LOG_FATAL(...)	
#endif


// Generic Log Macro defaults to client log 
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
#define VE_LOG(...) VE_CLIENT_LOG(__VA__ARGS__)
#else
#define VE_LOG(...) 
#endif