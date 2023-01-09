#pragma once

// Stupid windows DLL exporting of classes!
#ifdef _WINDOWS
#	ifdef _USRDLL
#		define EXPORT __declspec(dllexport)
#	else
#		define EXPORT __declspec(dllimport)
#	endif
#else
#	define EXPORT
#endif