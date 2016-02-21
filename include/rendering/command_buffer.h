/**
\file   command_buffer.h
\author Andrew Baxter
\date   February 20, 2016

Encapsulates an API-generic command buffer

*/

#ifndef BASILISK_COMMAND_BUFFER_H
#define BASILISK_COMMAND_BUFFER_H

namespace Basilisk
{
	template<class Impl>
	class CommandList
	{
	public:
		inline const Impl &GetImplementation()
		{
			return static_cast<Impl&>(*this);
		}


	};

	class D3D12CommandList : public CommandList<D3D12CommandList>
	{
	public:

	};

}

#endif