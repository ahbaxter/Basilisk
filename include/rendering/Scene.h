/**
\file   scene.h
\author Andrew Baxter
\date February 17, 2016

Represents a persistent game level

\todo Utilize data-oriented design

*/

#ifndef BASILISK_SCENE_H
#define BASILISK_SCENE_H

#include "common.h"
#include <glm/glm/fwd.hpp>

namespace Basilisk
{
	class Scene
	{
	public:
		/**
		Create an empty scene
		*/
		Scene();

		~Scene();

		/**
		\brief Load a scene from a file
		Clears the old scene, if any

		\param[in] filename The file to read from
		*/
		Result Load(const std::string &filename);

		void Clear();

	private:

	};
}

#endif