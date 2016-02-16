#pragma once
#include <stdexcept>
#include <string>


/**
\file   exceptions.h
\author Andrew Baxter
\date   February 16, 2016

\brief Defines a series of generic exceptions to be used by components of the game engine. Modeled after Python's exceptions.
Sets up the `DECLARE_EXCEPTION` macro to streamline the process
If this method proves to be oversimplified, I'll come back and renovate here

\todo Manually document the resulting classes with Doxygen markup
\todo Should `TimeoutException` really be a subclass of `IOException`?
\todo Any use for an `InputException` branch?
*/

namespace Basilisk
{
	#define DECLARE_EXCEPTION(name, parent) \
	class name : public parent \
	{ \
	public: \
		name() : parent(#name) { } \
		name(const char *message) : parent(message) { } \
		const char *toString() { return (#name + std::string(": ") + what()).c_str(); } \
	};

	DECLARE_EXCEPTION(Exception, std::runtime_error);
		DECLARE_EXCEPTION(ArrayIndexOutOfBoundsException, Exception);
		DECLARE_EXCEPTION(ArithmeticException, Exception);
			DECLARE_EXCEPTION(DivideByZeroException, ArithmeticException);
			DECLARE_EXCEPTION(OverflowException, ArithmeticException);
			DECLARE_EXCEPTION(FloatingPointException, ArithmeticException);

		DECLARE_EXCEPTION(IOException, Exception);
			DECLARE_EXCEPTION(EOFException, Exception);
			DECLARE_EXCEPTION(TimeoutException, Exception);
			DECLARE_EXCEPTION(InterruptedException, IOException);
			DECLARE_EXCEPTION(FileSystemException, IOException);
				DECLARE_EXCEPTION(FileNotFoundException, FileSystemException);
				DECLARE_EXCEPTION(FileExistsException, FileSystemException);
				DECLARE_EXCEPTION(FileFormatException, FileSystemException);
				DECLARE_EXCEPTION(IsADirectoryException, FileSystemException);
				DECLARE_EXCEPTION(NotADirectoryEcxeption, FileSystemException);

			DECLARE_EXCEPTION(NetworkException, IOException);
				DECLARE_EXCEPTION(ConnectionException, NetworkException);
					DECLARE_EXCEPTION(BrokenPipeException, ConnectionException);
					DECLARE_EXCEPTION(ConnectionAbortedException, ConnectionException);
					DECLARE_EXCEPTION(ConnectionRefusedException, ConnectionException);
					DECLARE_EXCEPTION(ConnectionResetException, ConnectionException);

		DECLARE_EXCEPTION(IllegalArgumentException, Exception);
		DECLARE_EXCEPTION(InvalidCastException, Exception);
		DECLARE_EXCEPTION(NullPointerException, Exception);
		DECLARE_EXCEPTION(OutOfMemoryException, Exception);
}