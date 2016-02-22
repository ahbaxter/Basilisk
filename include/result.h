/**
\file   result.h
\author Andrew Baxter
\date   February 21, 2016

Defines a series of all-purpose error (or success) codes to be used by components of the game engine

\todo Properly capitalize APIError to ApiError

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
	All-pupose error (or success) codes to be used by components of the Basilisk game engine

	\todo Is `DeviceLost` already covered in `APIFailure`? I figure it has its merits.
	*/
	enum class Result : int8_t
	{
		ApiError = -11,
		IllegalArgument,
		IllegalState,
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

	/**
	Checks if a Basilisk result succeeded

	\param[in] val The result to check
	\return `true` if the result indicates success; `false` otherwise
	*/
	inline bool Succeeded(Result val) {
		return (static_cast<std::underlying_type<Result>::type>(val) >= 0);
	}
	/**
	Checks if a Basilisk result failed

	\param[in] val The result to check
	\return `true` if the result indicates failure; `false` otherwise
	*/
	inline bool Failed(Result val) {
		return (static_cast<std::underlying_type<Result>::type>(val) < 0);
	}
	
	/**
	Translates a Basilisk result into English

	\param[in] val The result to translate
	\return English
	*/
	inline const std::string &ResultToString(const Result &val)
	{
		const std::string lookup[15] = {
			"API Failure",
			"Illegal Argument",
			"Illegal State",
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