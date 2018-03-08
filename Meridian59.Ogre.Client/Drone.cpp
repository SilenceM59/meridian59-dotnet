#include "stdafx.h"

namespace Meridian59
{
	namespace Ogre
	{
		using namespace System::Threading;

		// ########################################
		// Bot Initialize
		// ########################################
		void Drone::Init()
		{
			// Create a new bot instance
			Bot = gcnew Meridian59::Ogre::DroneBot();

			// Initialize the bot
			Bot->Init();
		}

		// ########################################
		// Stop the bot
		// ########################################
		void Drone::Stop()
		{
			Bot->StopEverything();
		}

		// ########################################
		// Game Tick
		// ########################################
		void Drone::Update()
		{
			// Gather user input and keep avatar & id's current.
			if (!Bot->Update())
			{
				return;
			}

			if (Bot->safe_logon && !Bot->safe_logged_on)
			{
				// Try to phase us when we first login
				Bot->PhaseNow(true);
				
				if (Bot->safe_phase_casted && !OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
				{
					if (Bot->is_safe_logon)
					{
						Bot->safe_logged_on = true;
					}
				}
				else {
					if (Bot->safe_phase_casted && OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
					{
						Bot->safe_logged_on = true;
					}
				}
			}

			if (Bot->phasing || Bot->auto_phasing)
			{
				Bot->is_safe_logon = false;
				Bot->Phase();
				return;
			}

			if (Bot->conveying || (Bot->auto_conveying && ! Bot->aggro))
			{
				if (std::difftime(std::time(nullptr), Bot->auto_convey))
				{
					Bot->auto_convey = std::time(nullptr);
				}

				Bot->Convey();
				return;
			}

			if (Bot->first_tick)
			{
				Bot->first_tick = false;
				ShowHelp();
			}

			// Seek & Destroy
			if (Bot->seek_destroy)
			{
				Bot->ticks_test = 0;
				Bot->SeekAndDestroy();
			}

			// Autofollow
			if (Bot->follow)
				Bot->AutoFollow();

			// Assist
			if (Bot->assist && Bot->assist_name != "")
				Bot->Assist();
		}

		void Drone::ShowHelp()
		{
			/*Bot->Log("-----------------------------------", true);
			Bot->Log("Drone has phased you until you're ready", true);
			Bot->Log("-----------------------------------", true);
			Bot->Log("/follow - Auto follow your current target.", true);
			Bot->Log("/assist \"Player Name\" [in quotes] - Become a minion.", true);
			Bot->Log("/seek - Seek out your own monsters and fight them.", true);
			Bot->Log("/stop - Stop all DroneBot script operations.", true);
			Bot->Log("-----------------------------------", true);
			Bot->Log("Use:  cast phase  to begin your journey.", true);*/
		}

		static Drone::Drone(void)
		{

		}
	};
};
