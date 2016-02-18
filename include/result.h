/**
\file   result.h
\author Andrew Baxter
\date   February 17, 2016

\brief Defines a series of generic error codes to be used by components of the game engine

\todo Any use for an `InputException` branch?
*/

#ifndef BASILISK_RESULT_H
#define BASILISK_RESULT_H

#include <string>

namespace Basilisk
{	extern std::string errorMessage;

	enum class Result : uint8_t
	{
		Success = 0,
		Failure, //Avoid using. Can always be more specific.
		IllegalArgument,
		OutOfMemory,

		Timeout,
		Interrupted,

		EndOfStream,
		ParseError,

		BrokenPipe,
		ConnectionRefused,
		ConnectionReset,
	};

	inline bool Eval(const Result &val) { return (val != Result::Success); }

	inline const std::string &ResultToString(const Result &val)
	{
		const std::string lookup[11] = {
			"Success",
			"Failure",
			"Illegal Argument",
			"Out of Memory",

			"Timeout",
			"Interrupted",

			"End of Stream",
			"Parsing Error",


			"Broken Pipe",
			"Connection Refused",
			"Connection Reset"
		};
		return lookup[static_cast<std::underlying_type<Result>::type>(val)];
	}


	#define DECLARE_EXCEPTION(name, parent) \
	class name : public parent \
	{ \
	public: \
		name() : parent(#name) { } \
		name(const char *message) : parent(message) { } \
		const char *toString() { return (#name + std::string(": ") + what()).c_str(); } \
	};

	DECLARE_EXCEPTION(Exception, std::runtime_error);

		DECLARE_EXCEPTION(IOException, Exception);
			DECLARE_EXCEPTION(FileSystemException, IOException);
				DECLARE_EXCEPTION(FileNotFoundException, FileSystemException);
				DECLARE_EXCEPTION(FileExistsException, FileSystemException);
				DECLARE_EXCEPTION(FileFormatException, FileSystemException);
				DECLARE_EXCEPTION(IsADirectoryException, FileSystemException);
				DECLARE_EXCEPTION(NotADirectoryEcxeption, FileSystemException);

}

#endif