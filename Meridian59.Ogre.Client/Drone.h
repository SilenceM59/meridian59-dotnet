#pragma once
#include "DroneBot.h"

namespace Meridian59 {

	namespace Ogre
	{
		public ref class Drone
		{

		public:
			void Init();
			void Update();
			void Stop();
			void ShowHelp();
			static Drone(void);
			DroneBot^ Bot;
		};
	};
};
