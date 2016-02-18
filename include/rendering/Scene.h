/**
\file   scene.h
\author Andrew Baxter
\date February 17, 2016

Represents a persistent game level

\todo Utilize data-oriented design

*/

#ifndef BASILISK_SCENE_H
#define BASILISK_SCENE_H

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
		/**
		Load a scene from a file
		
		\param[in] filename The file to read from
		\throws 
		*/

		~Scene();
	private:

	};
}

#endif