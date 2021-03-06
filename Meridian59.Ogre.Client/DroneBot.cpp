#include "stdafx.h"

namespace Meridian59
{
	namespace Ogre
	{
		using namespace System::Threading;

		int attack_distance = 7000;
		float target_distance = 0;

		int ticks_seek = 0;
		int ticks_assist = 0;
		int ticks_convey = 0;
		int ticks_phase = 0;
		int ticks_since_face = 0;
		int ticks_since_target = 0;

		int allow_face_change = 0;
		int max_face_ticks = 50;
		int max_teleport_ticks = 100;

		bool stop_test = false;
		bool stop_test_message = false;

		void DroneBot::Tour()
		{
			ticks_tour++;

			if (ticks_tour <= 10 || !OgreClient::Singleton->Data->RoomInformation->RoomID)
			{
				return;
			}

			ticks_tour = 0;

			CEGUI::ItemListbox* list = ControllerUI::GoList::List;

			if (next_tour_index != last_tour_index)
			{
				last_room_name = OgreClient::Singleton->Data->RoomInformation->RoomName;
				last_room_roo_name = OgreClient::Singleton->Data->RoomInformation->RoomFile;

				::CEGUI::String roo_name = list->getItemFromIndex(last_tour_index)->getChildAtIdx(1)->getText();

				Log("Tour in progress: " + "ROOM|" + list->getItemFromIndex(last_tour_index)->getID().ToString() + "|" + OgreClient::Singleton->Data->RoomInformation->RoomName + "|" + StringConvert::CEGUIToCLR(roo_name), true);

				last_tour_index = next_tour_index;
			}

			if (last_tour_index > list->getItemCount() - 1)
			{
				SetChatInput("");
				StopEverything();
				Log("-------------------------------------", true);
				Log("How to use your Tour Data", true);
				Log("-------------------------------------", true);
				Log("Check your Meridian59.log file for all of the data output into chat during your tour.", true);
				Log("Paste the log into MS Excel and Text-To-Columns | and grab your data from columns B, C and D.", true);
				Log("Your updated list can be added to UIGoList.cpp.", true);
				Log("-------------------------------------", true);
				return;
			}

			OgreClient::Singleton->SendReqDM(DMCommandType::GoRoom, list->getItemFromIndex(next_tour_index)->getID().ToString());
			next_tour_index++;
		}

		void DroneBot::Assist()
		{
			if (!assist_id || !assist_name || Avatar->IsMoving)
			{
				// Do nothing while we're moving or if we are not assisting anyone...
				return;
			}

			ticks_assist++;

			if (ticks_assist < 0)
			{
				if (face_target_id)
				{
					// Face a specific ID
					ticks_face = 5;
					FaceTarget(face_target_id);
				}
				else if (aggro_id)
				{
					// Face an aggro ID
					ticks_face = 5;
					FaceTarget(aggro_id);
				}
				else if (target_id)
				{
					// Face a target ID
					ticks_face = 5;
					FaceTarget(target_id);
				}
			}

			if (ticks_assist <= 0)
			{
				// Sleep until ticks_assist is greater than zero.
				// Allows some laggy processes to wait for the server to catch up.
				return;
			}

			ticks_assist = 0;

			if (cant_see_target)
			{
				OgreClient::Singleton->SendReqMoveMessage(true);
				OgreClient::Singleton->SendReqTurnMessage(true);
				ticks_assist = -10;
			}
			
			if (phasing || auto_phasing || OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
			{
				// Paralyzed (probably phasing).
				allow_fast_attack = false;
				return;
			}

			// Auto-phase below XX HP
			if (HP_Percent <= 0.30)
			{
				allow_fast_attack = false;

				// Phase me, I am 20% of my health.
				ticks_phase = 0;
				auto_phasing = true;
				return;
			}

			bool handling_local_aggro = false;

			if (assist_id && IsAssistValid())
			{
				assist_distance = Avatar->GetDistanceSquared(AssistPlayer->Position3D);

				if (aggro && aggro_id)
				{
					Meridian59::Data::Models::RoomObject^ aggro_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(aggro_id);

					if (aggro_obj == nullptr)
					{
						aggro = false;
						aggro_id = 0;
					}
					else {

						landed_hit = false;

						// We will face the aggro_id next tick...
						face_target_id = aggro_id;

						aggro_distance = Avatar->GetDistanceSquared(aggro_obj->Position3D);

						aggro_assist_distance = AssistPlayer->GetDistanceSquared(aggro_obj->Position3D);

						if (aggro_distance < assist_leash_distance && assist_distance < assist_leash_distance)
						{
							handling_local_aggro = true;
							OgreClient::Singleton->Data->TargetID = aggro_id;
						}

						if (handling_local_aggro && aggro_distance > attack_distance)
						{
							V2 target_position;

							target_position.X = aggro_obj->Position2D.X;
							target_position.Y = aggro_obj->Position2D.Y;

							// Move towards the target
							int Speed = 50;

							if (OgreClient::Singleton->Data->VigorPoints < 10)
								Speed = 25;

							Avatar->StartMoveTo(target_position, Speed);

							ticks_assist = -8;
						}

						if (handling_local_aggro && aggro_distance < attack_distance)
						{
							// Attack the aggro...
							allow_fast_attack = true;
						}
					}
				}

				if (!handling_local_aggro)
				{
					if (assist_distance > attack_distance)
					{
						// Get closer to the person we're assisting...
						FaceTarget(assist_id);

						V2 target_position;

						target_position.X = AssistPlayer->Position2D.X;
						target_position.Y = AssistPlayer->Position2D.Y;

						// Move towards the target
						int Speed = 50;

						if (OgreClient::Singleton->Data->VigorPoints < 10)
							Speed = 25;

						Avatar->StartMoveTo(target_position, Speed);

						ticks_assist = -8;

						// Save this, we can turn off invis on players..
						//OgreClient::Singleton->Data->RoomObjects->HasInvisibleRoomObject
					}
					else
					{
						landed_hit = false;

						// Auto-convey every 60 seconds
						if (!aggro && (!auto_phasing && !phasing) && account_type == AccountType::USER && std::difftime(std::time(nullptr), auto_convey) > auto_convey_every_interval_seconds)
						{
							Log("Sending regs to vault...", true);
							auto_conveying = true;
							return;
						}

						// Attack anything next to the person we're assisting...
						face_target_id = GetClosestAssistTarget(true);

						if (face_target_id)
						{
							Meridian59::Data::Models::RoomObject^ target_room_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(target_id);

							if (target_room_obj != nullptr)
							{
								float target_distance = Avatar->GetDistanceSquared(target_room_obj->Position3D);

								// Attack a nearby target
								allow_fast_attack = true;
							}
							else {
								face_target_id = 0;
							}
						}
					}
				}
			}
		}

		int DroneBot::GetClosestAssistTarget(bool target_it)
		{
			if (AssistPlayer == nullptr)
			{
				return 0;
			}

			face_target_id = 0;
			target_id = 0;
			target_distance = 0;
			target_last_x = 0;
			target_last_y = 0;

			DataControllerOgre^ OgreData = OgreClient::Singleton->Data;

			RoomObject^ closest = nullptr;
			Real smallestdist = Real::MaxValue;

			RoomObject^ closest_group = nullptr;
			Real smallestdist_group = Real::MaxValue;

			RoomObject^ closest_aggro = nullptr;
			Real smallestdist_aggro = Real::MaxValue;

			aggro = false;
			aggro_id = 0;
			
			for each(RoomObject^ obj in OgreData->RoomObjects)
			{
				if (obj != nullptr && obj != AssistPlayer && obj != Avatar && obj->Flags->IsCreature)
				{
					if (!obj->Flags->IsMinimapAggroSelf && (obj->Name->Contains("soldier") || obj->Name->Contains("army")))
					{
						// Do not attack soldiers unless they're aggro to you.
						continue;
					}

					Real dist = AssistPlayer->GetDistanceSquared(obj->Position3D);

					if (dist <= smallestdist)
					{
						smallestdist = dist;
						closest = obj;
					}

					if (obj->Flags->IsMinimapAggroOther)
					{
						if (dist <= smallestdist_group)
						{
							smallestdist_group = dist;
							closest_group = obj;
						}
					}

					if (obj->Flags->IsMinimapAggroSelf)
					{
						if (dist <= smallestdist_aggro)
						{
							smallestdist_aggro = dist;
							closest_aggro = obj;
						}
					}
				}
			}

			if (closest_group != nullptr && smallestdist_group <= assist_leash_distance)
			{
				// Help with group monster first
				// Override our regular aggro with the group aggro.
				closest = closest_group;
				smallestdist = smallestdist_group;
				aggro = true;
				aggro_id = closest->ID;
				aggro_distance = Avatar->GetDistanceSquared(closest->Position3D);
			}
			else {
				if (closest_aggro != nullptr && smallestdist_aggro <= assist_leash_distance)
				{
					// If the group monster is farther, try to work on my aggro.
					aggro = true;
					closest = closest_aggro;
					smallestdist = smallestdist_aggro;
					aggro_id = closest->ID;
					aggro_distance = Avatar->GetDistanceSquared(closest->Position3D);
				}
			}

			if (closest != nullptr)
			{
				if (target_id != closest->ID)
				{
					target_changed = true;
					/*if (!IsDM)
					{
						OgreClient::Singleton->LootAllStackables();
					}*/
				}
				target_id = closest->ID;
			}
			else {
				target_id = 0;
			}

			if (target_id)
			{
				if (target_it && OgreData->TargetID != target_id)
				{
					OgreData->TargetID = target_id;
					landed_hit = false;
				}

				// Distance to the target
				target_distance = smallestdist;
				target_last_x = closest->Position2D.X;
				target_last_y = closest->Position2D.Y;
			}

			return target_id;
		}

		bool DroneBot::IsAssistValid()
		{
			if (!assist_id || !assist_name || AssistPlayer == nullptr || assist_id != AssistPlayer->ID)
			{
				return false;
			}

			return true;
		}

		bool DroneBot::IsFollowValid()
		{
			if (!follow_id || !follow_name || FollowPlayer == nullptr || follow_id != FollowPlayer->ID)
			{
				return false;
			}

			return true;
		}

		void DroneBot::SeekAndDestroy()
		{
			Meridian59::Data::Models::RoomObject^ target_room_obj = nullptr;

			ticks_seek++;

			if (ticks_seek == -5)
			{
				if (target_id)
				{
					ticks_face = 5;
					FaceTarget(target_id);
				}
			}

			if (ticks_seek <= 0)
			{
				// Sleep until ticks_seek is greater than zero.
				// Allows some laggy processes to wait for the server to catch up.
				return;
			}

			ticks_seek = 0;

			try {

				if (OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
				{
					// Paralyzed (probably phasing).
					allow_fast_attack = false;
					return;
				}

				if (phasing || auto_phasing)
				{
					// Prevent doing any actions while phasing...
					return;
				}

				GetClosestTarget(true);

				// Auto-convey every 60 seconds
				if (!aggro && (!auto_phasing && !phasing) && account_type == AccountType::USER && std::difftime(std::time(nullptr), auto_convey) > auto_convey_every_interval_seconds)
				{
					Log("Sending regs to vault...", true);
					auto_conveying = true;

					return;
				}

				// Auto-phase below XX HP
				if (!auto_phasing && HP_Percent <= 0.30)
				{
					StopEverything();
					allow_fast_attack = false;
					// Phase me, I am low HP.
					ticks_phase = 0;
					auto_phasing = true;
					return;
				}

				Running_SeekAndDestroy = true;

				if (target_id)
				{
					DataControllerOgre^ OgreData = OgreClient::Singleton->Data;
					target_room_obj = OgreData->RoomObjects->GetItemByID(target_id);
					ticks_face = 5;
					FaceTarget(target_id);

					// Before moving to the target...
					if (!aggro)
					{
						if (HP_Percent < 0.9)
						{
							allow_fast_attack = false;
							// Let's try to recover our hp before heading to another monster...
							return;
						}
					}

					// Allows a non-tick restricted attack sender...
					// Should make attacking be more real-time and faster (as the client holding the attack key).
					allow_fast_attack = true;
				}
				else {
					return;
				}

				if (cant_see_target && target_distance <= attack_distance)
				{
					if (CheckSkipTarget())
					{
						IgnoredTargets.Add(target_id);
						ignored_targets_timer = std::time(nullptr);
						return;
					}
				}
				else {
					if (target_distance > attack_distance)
					{
						// No bad timer until we are already at the monster.
						bad_target_timer = NULL;
					}
				}

				if (cant_see_target)
				{
					OgreClient::Singleton->SendReqMoveMessage(true);
					OgreClient::Singleton->SendReqTurnMessage(true);

					target_distance += 30000;
				}


				if (target_id && (target_distance > attack_distance || cant_see_target))
				{
					// We need to move closer to the target
					V2 target_position;

					target_position.X = target_last_x;
					target_position.Y = target_last_y;

					int sym_random = (int)Math::SymmetricRandom();

					if (cant_see_target)
					{
						cant_see_target = false;

						if (sym_random >= 0)
						{
							target_position.X += Math::RangeRandom(-70, 70);
							target_position.Y += Math::RangeRandom(-70, 70);
						}
						if (sym_random < 0)
						{
							target_position.X -= Math::RangeRandom(-100, 100);
							target_position.Y -= Math::RangeRandom(-100, 100);
						}

						// Wait 5 ticks after the position adjustment.
						// Allows for adjustments to set in to the server.
						ticks_seek = -10;
					}
					else {
						target_position.X -= Math::RangeRandom(-100, 100);
						target_position.Y += Math::RangeRandom(-100, 100);
					}

					// Setting the vigor speed is import because we're bypassing the collision TryMove system.
					// If you are running while out of vigor, the game will constantly kick back your char position.
					int Speed = (byte)MovementSpeed::Run;

					if (OgreClient::Singleton->Data->VigorPoints < 10)
						Speed = (byte)MovementSpeed::Walk;

					Avatar->StartMoveTo(target_position, Speed);
				}
				else {
					landed_hit = false;
				}
			}
			catch (...) {

				target_id = 0;
				target_distance = 0;
				target_last_x = 0;
				target_last_y = 0;

				//Sleep(1000);

				Running_SeekAndDestroy = false;
				return;
			}

			Running_SeekAndDestroy = false;

			//Log("Seek & Destroy: Thread has finished.", true);
			//stop_test = false;
		}

		void DroneBot::Convey()
		{
			if (auto_conveying)
			{
				// Check for aggro while we're conveying...
				GetClosestTarget(false);
			}

			if (auto_conveying && aggro)
			{
				// Prevent conveying while you have aggro.
				ticks_convey = 100;
				return;
			}

			ticks_convey++;

			if (ticks_convey >= 20)
			{
				ticks_convey = 0;
			}
			else if (ticks_convey > 1) {
				// Wait ticks for each conveyance but allow first tick to happen.
				return;
			}

			bool sent_something = false;
			bool select_something = false;

			if (Avatar != nullptr)
			{
				SpellObject^ conveySpell = OgreClient::Singleton->Data->SpellObjects->GetItemByName("Conveyance", false);

				if (conveySpell == nullptr)
				{
					conveying = false;
					return;
				}

				InventoryObjectList^ InventoryList = OgreClient::Singleton->Data->InventoryObjects;

				for each(InventoryObject^ InventoryObj in InventoryList)
				{
					if (InventoryObj->IsStackable && InventoryObj->Name != "shilling" && !InventoryObj->Name->Contains("Inky-cap") && !InventoryObj->Name->Contains("red mush"))
					{
						if (OgreClient::Singleton->Data->TargetID == InventoryObj->ID)
						{
							if (OgreClient::Singleton->GameTick->CanReqCast())
							{
								OgreClient::Singleton->SendReqCastMessage(conveySpell);
								sent_something = true;
								break;
							}
						}
						else {
							Log("Sending " + InventoryObj->Count.ToString() + " " + InventoryObj->Name + " to your vault.", true);
							OgreClient::Singleton->Data->TargetID = InventoryObj->ID;
							select_something = true;
							break;
						}
					}
					if (InventoryObj->Name->Contains("Inky-cap") && std::difftime(std::time(nullptr), last_eat) > 10)
					{
						if (VG <= 180)
						{
							last_eat = std::time(nullptr);
							OgreClient::Singleton->UseUnuseApply(InventoryObj);
						}
					}
				}
			}
			else {
				// No avatar - no convey!
				conveying = false;
				return;
			}

			if (sent_something || select_something)
			{
				// Wait for next tick.
				return;
			}

			// Nothing left to send.
			ticks_convey = 0;
			conveying = false;
			auto_conveying = false;
			auto_convey = std::time(nullptr);
		}

		void DroneBot::Phase()
		{
			if (is_safe_logon)
			{
				phasing = false;
				auto_phasing = false;

				return;
			}

			if (IsDM)
			{
				safe_logged_on = true;
			}

			bool try_phasing = false;

			ticks_phase++;

			if (ticks_phase >= 30)
			{
				ticks_phase = 1;
			}

			if (ticks_phase == 1)
			{
				try_phasing = true;
			}

			if ((phasing || auto_phasing) && try_phasing)
			{
				if (Avatar != nullptr && !Avatar->Flags->IsPhasing && !OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
				{
					if (IsDM)
					{
						OgreClient::Singleton->ExecChatCommand("dm immortal");
					}

					ticks_phase = PhaseNow(true);
				}
			}
		}

		int DroneBot::PhaseNow(bool phase)
		{
			if (IsDM)
			{
				safe_phase_casted = true;

				return -500;
			}

			bool is_phased = false;

			if (OgreClient::Singleton->Data->Effects->Paralyze->IsActive)
			{
				is_phased = true;
			}

			SpellObject^ spell = OgreClient::Singleton->Data->SpellObjects->GetItemByName("Phase", false);

			if (spell == nullptr)
			{
				return -500;
			}

			if (phase && !is_phased)
			{
				// Phase the player
				if (OgreClient::Singleton->GameTick->CanReqCast())
				{
					OgreClient::Singleton->SendReqCastMessage(spell);

					safe_phase_casted = true;

					return -1000;
				}
			}
			else if (!phase && is_phased) {
				// UnPhase the player
				if (OgreClient::Singleton->GameTick->CanReqCast())
				{
					OgreClient::Singleton->SendReqCastMessage(spell);
					return -1000;
				}
			}

			return -25;
		}

		void DroneBot::AutoFollow()
		{
			if (!follow_name)
			{
				return;
			}

			if (follow_id && IsFollowValid())
			{
				follow_distance = FollowPlayer->GetDistanceSquared(Avatar->Position3D);

				if (follow_distance > attack_distance)
				{
					FaceTarget(follow_id);

					V2 target_position;

					target_position.X = FollowPlayer->Position2D.X;
					target_position.Y = FollowPlayer->Position2D.Y;

					// Move towards the target
					int Speed = (byte)MovementSpeed::Run;

					if (OgreClient::Singleton->Data->VigorPoints < 10)
						Speed = (byte)MovementSpeed::Walk;

					Avatar->StartMoveTo(target_position, Speed);
				}
			}
		}

		void DroneBot::FastAttack()
		{
			try {
				if (target_id)
				{
					Meridian59::Data::Models::RoomObject^ target_room_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(target_id);

					if (target_room_obj != nullptr && target_room_obj->IsTarget)
					{
						OgreClient::Singleton->SendReqAttackMessage();
					}
				}
			}
			catch (...)
			{

			}
		}

		bool DroneBot::CheckSkipTarget()
		{
			if (Avatar == nullptr || Avatar->IsMoving)
			{
				return false;
			}

			if (bad_target_timer == NULL)
			{
				bad_target_timer = std::time(nullptr);

				return false;
			}

			if (target_id)
			{
				int bad_target_diff = std::difftime(std::time(nullptr), bad_target_timer);

				if (bad_target_diff > 5)
				{
					Log("Target is unreachable after " + bad_target_diff.ToString() + " seconds...", true);
					bad_target_timer = NULL;
					return true;
				}
			}

			return false;
		}

		void DroneBot::FaceTarget(int to_target_id)
		{
			ticks_face++;

			if (ticks_face > 2)
			{
				ticks_face = 0;
			}
			else {
				return;
			}

			if (to_target_id)
			{
				DataControllerOgre^ OgreData = OgreClient::Singleton->Data;

				Meridian59::Data::Models::RoomObject^ target_room_obj = OgreData->RoomObjects->GetItemByID(to_target_id);

				if (Avatar != nullptr && to_target_id && target_room_obj != nullptr)
				{
					Avatar->Angle = MathUtil::BinaryAngleToRadian(Avatar->GetAngleTo(target_room_obj));
					OgreClient::Singleton->SendReqTurnMessage(true);
				}
			}
		}

		int DroneBot::GetClosestTarget(bool target_it)
		{
			target_id = 0;
			target_distance = 0;
			target_last_x = 0;
			target_last_y = 0;

			DataControllerOgre^ OgreData = OgreClient::Singleton->Data;

			RoomObject^ closest = nullptr;
			RoomObject^ closest_aggro = nullptr;
			Real smallestdist = Real::MaxValue;
			Real smallestdist_aggro = Real::MaxValue;
			Real comfortable_distance = 500000 * 15;

			aggro = false;

			if (std::difftime(std::time(nullptr), ignored_targets_timer) > 20)
			{
				IgnoredTargets.Clear();
				ignored_targets_timer = NULL;
			}

			for each(RoomObject^ obj in OgreData->RoomObjects)
			{
				// object itself doesn't count
				if (obj != nullptr && obj->Flags->IsCreature && !IgnoredTargets.Contains(obj->ID) && !obj->Name->Contains("soldier") && !obj->Name->Contains("army"))
				{
					Real dist = obj->GetDistanceSquared(Avatar->Position3D);

					if (obj->Flags->IsMinimapAggroSelf)
					{
						if (dist <= smallestdist_aggro)
						{
							smallestdist_aggro = dist;
							closest_aggro = obj;
						}
					}

					if (dist <= smallestdist)
					{
						smallestdist = dist;
						closest = obj;
					}

					// Check if this object is aggro on avatar
					if (!aggro)
					{
						aggro = obj->Flags->IsMinimapAggroSelf;
					}

					if (aggro && closest_aggro != nullptr)
					{
						// Get aggro distance
						aggro_distance = Avatar->GetDistanceSquared(closest_aggro->Position3D);
						aggro_id = closest_aggro->ID;
					}
					else {
						aggro = false;
						aggro_id = 0;
						aggro_distance = 0;
					}
				}
			}

			if (closest == nullptr)
			{
				IgnoredTargets.Clear();
				ignored_targets_timer = NULL;
			}

			if (smallestdist > comfortable_distance)
			{
				closest = nullptr;
			}

			if (smallestdist_aggro > comfortable_distance)
			{
				closest_aggro = nullptr;
			}

			if (closest_aggro != nullptr)
			{
				closest = closest_aggro;
				smallestdist = smallestdist_aggro;
			}

			if (closest != nullptr)
			{
				if (target_id != closest->ID)
				{
					target_changed = true;

					/*if (!IsDM)
					{
						OgreClient::Singleton->LootAllStackables();
					}*/
				}
				target_id = closest->ID;
			}
			else {
				target_id = 0;
			}

			if (target_id)
			{
				if (target_it && OgreData->TargetID != target_id)
				{
					if (!OgreData->TargetID)
					{
						// No Target or Target Killed
						IgnoredTargets.Clear();
						ignored_targets_timer = NULL;
					}

					OgreData->TargetID = target_id;
					cant_see_chat_tick = 0;
				}

				// Distance to the target
				target_distance = smallestdist;
				target_last_x = closest->Position2D.X;
				target_last_y = closest->Position2D.Y;
			}

			return target_id;
		}

		static DroneBot::DroneBot(void)
		{
			//VisitedTargets = gcnew List<RoomObject^>();
		};

		// The bot has been loaded into Ogre
		void DroneBot::Init()
		{
			Window = static_cast<CEGUI::FrameWindow*>(ControllerUI::GUIRoot->getChild(UI_NAME_CHAT_WINDOW));
			Input = static_cast<CEGUI::Editbox*>(Window->getChild(UI_NAME_CHAT_INPUT));
			Text = static_cast<CEGUI::Window*>(Window->getChild(UI_NAME_CHAT_TEXT));

			Log("Loaded into Ogre3D Client successfully.", false);
		}

		void DroneBot::UpdateAllowTicks()
		{
			if (seek_destroy)
			{
				allow_face_change++;
			}
		}

		void DroneBot::UpdateChatStates()
		{
			if (Avatar == nullptr)
			{
				return;
			}

			ServerString^ new_message = OgreClient::Singleton->Data->ChatMessages->LastAddedItem;

			if (new_message != nullptr)
			{
				last_message = new_message->ToString();

				if (last_message->Contains("This place of safety") || last_message->Contains("recently attacked a player"))
				{
					is_safe_logon = true;
					OgreClient::Singleton->Data->ChatMessages->Remove(new_message);
				}

				// Only mind "Can't see" and "Can't reach" messages if you haven't landed a hit.
				if (!landed_hit)
				{
					if (last_message->Contains("You can't see") || last_message->Contains("You can't reach") || last_message->Contains("is too far away to"))
					{
						ticks_face = 5;
						cant_see_target = true;

						OgreClient::Singleton->Data->ChatMessages->Remove(new_message);

						cant_see_chat_tick++;

						if (cant_see_chat_tick > 0)
						{
							cant_see_target = true;
							cant_see_chat_tick = -5;
						}

						// This may break everything...
						landed_hit = false;
					}
				}
			}
		}

		void DroneBot::CheckLandedHit()
		{
			if (Avatar == nullptr)
			{
				return;
			}

			if (Avatar->Animation->AnimationType == AnimationType::ONCE && Avatar->Animation->CurrentGroup == 2)
			{
				// Avatar attacking.
				landed_hit = true;
				last_hit_timer = std::time(nullptr);
				bad_target_timer = NULL;
				cant_see_target = false;
			}
		}

		// While ogre performs live updates, each tick will perform an action
		bool DroneBot::Tick()
		{
			UpdateChatStates();

			CheckLandedHit();

			if (allow_fast_attack)
			{
				FastAttack();
			}

			ticks_tick++;

			// Forces the bot to take 1 Tick and give Ogre 1 Tick
			// Split the ticks man!
			if (ticks_tick > 2)
			{
				ticks_tick = 0;
			}
			else {
				return false;
			}

			UpdateAllowTicks();

			try {

				if (room_changed || game_saved || avatar_id != OgreClient::Singleton->Data->AvatarID)
				{
					Avatar = OgreClient::Singleton->Data->AvatarObject;

					if (Avatar == nullptr)
					{
						return false;
					}

					IsDM = false;

					account_type = OgreClient::Singleton->Data->AccountType;

					if (account_type == AccountType::ADMIN || account_type == AccountType::DM)
					{
						IsDM = true;
					}

					// Reset follow and assist ID's when the room changes.
					follow_id = 0;
					assist_id = 0;
					AssistPlayer = nullptr;
					FollowPlayer = nullptr;
				}

				if (assist && assist_name && !assist_id)
				{
					AssistPlayer = OgreClient::Singleton->Data->RoomObjects->GetItemByName(assist_name, false);

					if (AssistPlayer == nullptr || !AssistPlayer->Flags->IsPlayer)
					{
						AssistPlayer = nullptr;
						assist_id = 0;
					}
					else {
						assist_id = AssistPlayer->ID;
					}
				}

				if (follow && follow_name)
				{
					FollowPlayer = OgreClient::Singleton->Data->RoomObjects->GetItemByName(follow_name, false);

					if (FollowPlayer == nullptr || !FollowPlayer->Flags->IsPlayer)
					{
						FollowPlayer = nullptr;
						follow_id = 0;
					}
					else {
						follow_id = FollowPlayer->ID;
					}
				}

				if (last_room_name == "")
				{
					last_room_name = OgreClient::Singleton->CurrentRoom->Filename;
				}
				else if (last_room_name != OgreClient::Singleton->CurrentRoom->Filename) {
					room_changed = true;
				}

				if (Avatar == nullptr || !Avatar->ID)
				{
					return false;
				}

				avatar_id = Avatar->ID;

				if (avatar_id)
				{
					HP = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(1)->ValueCurrent;
					HP_Max = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(1)->ValueMaximum;
					HP_Percent = (float)((float)HP / (float)HP_Max);

					MP = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(2)->ValueCurrent;
					MP_Max = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(2)->ValueMaximum;
					MP_Percent = (float)((float)MP / (float)MP_Max);

					VG = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(3)->ValueCurrent;
					VG_Max = OgreClient::Singleton->Data->AvatarCondition->GetItemByNum(3)->ValueMaximum;
					VG_Percent = (float)((float)VG / (float)VG_Max);
				}
				;
			}
			catch (...) {
				return false;
			}

			return true;
		}

		// Ogre is performing live updates
		bool DroneBot::Update()
		{
			UserInput();

			return Tick();
		}

		// Collect the user input using the chat box
		void DroneBot::UserInput()
		{
			bool ok = false;
			System::String^ input = StringConvert::CEGUIToCLR(Input->getText());

			if (input == "")
			{
				return;
			}

			// Stop everything
			if (input == "/stop")
			{
				ok = true;
				StopEverything();
			}

			// Convey everything			
			if (input == "/convey" && !follow)
			{
				ok = true;
				conveying = true;
				ticks_convey = 0;
				Log("Conveying NOW.", true);
			}
			else if (input == "/convey") {
				ok = true;
				conveying = false;
				ticks_convey = 0;
				Log("Conveying ABORTED.", true);
			}

			// Auto Follow			
			if (input == "/follow" && !follow)
			{
				follow_name = "";
				follow_id = NULL;
				SetChatInput("");

				if (OgreClient::Singleton->Data->TargetObject == nullptr)
				{
					Log("You must first target a player to follow.", true);
					return;
				}

				target_id = OgreClient::Singleton->Data->TargetID;

				if (!target_id || !OgreClient::Singleton->Data->TargetObject->Flags->IsPlayer)
				{
					Log("You must first target a player to follow.", true);
					return;
				}

				Meridian59::Data::Models::RoomObject^ target_room_obj = nullptr;

				target_room_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(target_id);

				follow_name = target_room_obj->Name;
				follow_id = target_room_obj->ID;

				ok = true;
				seek_destroy = false;
				follow = true;
				Log("Auto following " + follow_name + ".", true);
			}
			else if (input == "/follow") {
				ok = true;
				follow = false;
				Log("Auto-Follow OFF.", true);
			}

			if (input == "/tour")
			{
				Tour();
			}

			// Assist 
			if (input->Contains("/assist") && input->Contains("[") && input->Contains("]"))
			{
				// Wait for a valid player name in quotes.
				ok = true;

				// Stop all other automated actions
				StopEverything();

				assist = true;
				assist_name = input->Substring(input->IndexOf("[") + 1);
				assist_name = assist_name->Substring(0, assist_name->IndexOf("]"));

				SetChatInput("");

				if (assist_name == "" || assist_name == "target" || assist_name == "Target")
				{
					if (OgreClient::Singleton->Data->TargetObject == nullptr)
					{
						Log("You must first target a player to assist.", true);
						return;
					}

					target_id = OgreClient::Singleton->Data->TargetID;

					if (!target_id || !OgreClient::Singleton->Data->TargetObject->Flags->IsPlayer)
					{
						Log("You must first target a player to assist.", true);
						return;
					}

					Meridian59::Data::Models::RoomObject^ target_room_obj = nullptr;

					target_room_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(target_id);

					assist_name = target_room_obj->Name;
				}

				Log("Assisting " + assist_name, true);
				Log("When " + assist_name + " is on the same map as you, the fun will begin.", true);
				Log("To stop assisting just type: /stop", true);
			}
			else if (input == "/assist off") {
				ok = true;
				assist = false;
				Log("Assist OFF.", true);
			}

			// Seek & Destroy (self builder) 
			if (input == "/seek" && !seek_destroy)
			{
				ok = true;
				StopEverything();
				seek_destroy = true;
				Log("Seek & Destroy ON.", true);
			}
			else if (input == "/seek") {
				ok = true;
				seek_destroy = false;
				Log("Seek & Destroy OFF.", true);
			}

			if (ok)
			{
				// Erase the text entry before the user can hit enter.
				SetChatInput("");
			}
		}

		// Stops everything
		void DroneBot::StopEverything()
		{
			follow = false;
			assist = false;
			seek_destroy = false;
			conveying = false;
			phasing = false;
			auto_conveying = false;
			auto_phasing = false;
			allow_fast_attack = false;
			allow_face_change = false;
		}

		// Sets a string to the chat input box
		void DroneBot::SetChatInput(CEGUI::String message)
		{
			Input->setText(message);

			if (Input->isActive())
			{
				Input->deactivate();
			}
		}

		// Saves a message to log and optionally sends it to the Chat Display
		void DroneBot::Log(System::String^ message, bool toChat)
		{
			Logger::Log(Logger::MODULENAME, Enums::LogType::Info, "DroneBot: " + message);

			if (toChat)
			{
				AppendChat(message);
			}
		}

		// Appends a message in Bold Black to the Chat Display
		void DroneBot::AppendChat(System::String^ message)
		{
			if (Window != nullptr)
			{
				CLRString^ str = CLRString::Empty;
				str += UI_TAGFONT_BOLD + UI_TAGCOLOR_BLACK + message + "\n";
				Text->appendText(StringConvert::CLRToCEGUI(str));
			}
		}

		void DroneBot::LogPosition()
		{
			try {

				// Disabled - Used for debugging.
				return;

				if (avatar_id)
				{
					bool change = false;

					if (last_x != Avatar->Position3D.X)
					{
						last_x = Avatar->Position3D.X;
						change = true;
					}
					if (last_y != Avatar->Position3D.Y)
					{
						last_y = Avatar->Position3D.Y;
						change = true;
					}
					if (last_z != Avatar->Position3D.Z)
					{
						last_z = Avatar->Position3D.Z;
						change = true;
					}
				}
			}
			catch (...) {
			}
		}

	};
};

