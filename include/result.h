/**
\file   result.h
\author Andrew Baxter
\date   February 18, 2016

\brief Defines a series of generic error codes to be used by components of the game engine

\todo Finish documenting
*/

#ifndef BASILISK_RESULT_H
#define BASILISK_RESULT_H

#include <string>

namespace Basilisk
{
	/**
	Used to store a global error message giving specifics about what went wrong
	*/
	extern std::string errorMessage;

	/**

	*/
	enum class Result : int8_t
	{
		Failure = -10,
		IllegalArgument,
		OutOfMemory,

		EndOfStream,
		ParseError,
		FileNotFound,

		BrokenPipe,
		ConnectionRefused,
		ConnectionReset,

		DeviceLost,

		Success, // = 0

		NotReady,
		Timeout,
		Incomplete,
	};

	inline bool Succeeded(Result val) { return (static_cast<std::underlying_type<Result>::type>(val) >= 0); }
	inline bool Failed(Result val) { return (static_cast<std::underlying_type<Result>::type>(val) < 0); }

	inline const std::string &ResultToString(const Result &val)
	{
		const std::string lookup[14] = {
			"Failure",
			"Illegal Argument",
			"Out of Memory",

			"End of Stream",
			"Parsing Error",
			"File not Found",

			"Broken Pipe",
			"Connection Refused",
			"Connection Reset",

			"Device Lost"

			"Success",

			"Not Ready"
			"Timeout",
			"Incomplete",
		};
		return lookup[static_cast<std::underlying_type<Result>::type>(val) + 10];
	}
}

#endif