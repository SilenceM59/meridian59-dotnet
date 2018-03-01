#include "stdafx.h"

namespace Meridian59
{
	namespace Ogre
	{
		using namespace System::Threading;

		int attack_distance = 12000;
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

			if (ticks_tour <= 10 || ! OgreClient::Singleton->Data->RoomInformation->RoomID)
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
			if (!assist_name)
			{
				return;
			}

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

			// Auto-phase below XX HP
			if (!auto_phasing && HP_Percent <= 0.30)
			{
				if (!stop_test)
				{
					Log("HP: " + HP.ToString() + " / Max: " + HP_Max.ToString() + " / Percent: " + HP_Percent.ToString(), true);
					stop_test = true;
				}
				// Phase me, I am 20% of my health.
				ticks_phase = 0;
				auto_phasing = true;
				return;
			}

			if (assist_id && IsAssistValid())
			{
				assist_distance = AssistPlayer->GetDistanceSquared(Avatar->Position3D);

				if (assist_distance > 5000 && !Avatar->IsMoving)
				{
					FaceTarget(assist_id);

					V2 target_position;

					target_position.X = AssistPlayer->Position2D.X;
					target_position.Y = AssistPlayer->Position2D.Y;

					// Move towards the target
					int Speed = 50;

					if (OgreClient::Singleton->Data->VigorPoints < 10)
						Speed = 25;

					Avatar->StartMoveTo(target_position, Speed);

					// Save this, we can turn off invis on players..
					//OgreClient::Singleton->Data->RoomObjects->HasInvisibleRoomObject
				}
				else {
					// Auto-convey every 60 seconds
					if (!aggro && (!auto_phasing && !phasing) && account_type == AccountType::USER && std::difftime(std::time(nullptr), auto_convey) > 60)
					{
						Log("Sending regs to vault...", true);
						auto_conveying = true;
						return;
					}

					// We are close enough to attack
					int face_target_id = GetClosestAssistTarget(true);

					if (face_target_id)
					{
						FaceTarget(face_target_id);
						allow_fast_attack = true;
					}
				}
			}
		}

		int DroneBot::GetClosestAssistTarget(bool target_it)
		{
			target_id = 0;
			target_distance = 0;
			target_last_x = 0;
			target_last_y = 0;

			DataControllerOgre^ OgreData = OgreClient::Singleton->Data;

			RoomObject^ closest = nullptr;
			Real smallestdist = Real::MaxValue;

			aggro = false;

			for each(RoomObject^ obj in OgreData->RoomObjects)
			{
				// object itself doesn't count
				if (obj != nullptr && obj != AssistPlayer && obj != Avatar && obj->Flags->IsCreature && !obj->Name->Contains("soldier") && !obj->Name->Contains("army"))
				{
					Real dist = obj->GetDistanceSquared(AssistPlayer->Position3D);
					;
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
				}
			}

			if (closest != nullptr)
			{
				if (target_id != closest->ID)
				{
					target_changed = true;
					OgreClient::Singleton->LootAllStackables();
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

		void DroneBot::SeekAndDestroy()
		{
			ticks_seek++;

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

				if (cant_see_target)
				{
					target_distance += 15000;
				}

				// Auto-convey every 60 seconds
				if (!aggro && (!auto_phasing && !phasing) && account_type == AccountType::USER && std::difftime(std::time(nullptr), auto_convey) > 60)
				{
					Log("Sending regs to vault...", true);
					auto_conveying = true;

					return;
				}

				// Auto-phase below XX HP
				if (!auto_phasing && HP_Percent <= 0.30)
				{
					if (!stop_test)
					{
						Log("HP: " + HP.ToString() + " / Max: " + HP_Max.ToString() + " / Percent: " + HP_Percent.ToString(), true);
						stop_test = true;
					}
					// Phase me, I am 20% of my health.
					ticks_phase = 0;
					auto_phasing = true;
					return;
				}

				Running_SeekAndDestroy = true;

				seek_timer = std::time(nullptr);
				face_timer = std::time(nullptr);

				if (target_id)
				{
					DataControllerOgre^ OgreData = OgreClient::Singleton->Data;
					Meridian59::Data::Models::RoomObject^ target_room_obj = OgreData->RoomObjects->GetItemByID(target_id);
					FaceTarget(target_id);

					// Allows a non-tick restricted attack sender...
					// Should make attacking be more real-time and faster (as the client holding the attack key).
					allow_fast_attack = true;
				}
				else {
					return;
				}

				if (target_id && target_distance > attack_distance)
				{
					// Before moving to the target...
					if (!aggro)
					{
						if (HP_Percent < 0.9)
						{
							// Let's try to recover our hp before heading to another monster...
							return;
						}
					}

					// We need to move closer to the target
					V2 target_position;

					target_position.X = target_last_x;
					target_position.Y = target_last_y;

					if (cant_see_target)
					{
						cant_see_target = false;

						int sym_random = (int)Math::SymmetricRandom();

						if (sym_random >= 0)
						{
							target_position.X += Math::RangeRandom(-30, 20);
							target_position.Y += Math::RangeRandom(-20, 30);
						}
						if (sym_random < 0)
						{
							target_position.X -= Math::RangeRandom(-60, 40);
							target_position.Y -= Math::RangeRandom(-60, 10);
						}

						// Wait 5 ticks after the position adjustment.
						// Allows for adjustments to set in to the server.
						ticks_seek = -5;
					}
					else {
						target_position.X -= 30;
						target_position.Y -= 30;
					}

					// Setting the vigor speed is import because we're bypassing the collision TryMove system.
					// If you are running while out of vigor, the game will constantly kick back your char position.
					int Speed = (byte)MovementSpeed::Run;

					if (OgreClient::Singleton->Data->VigorPoints < 10)
						Speed = (byte)MovementSpeed::Walk;

					Avatar->StartMoveTo(target_position, Speed);
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
					if (InventoryObj->IsStackable && InventoryObj->Name != "shilling" && !InventoryObj->Name->Contains("Inky-cap"))
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
						if (VG <= 160)
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
					ticks_phase = PhaseNow(true);
				}
			}
		}

		int DroneBot::PhaseNow(bool phase)
		{
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
			// This will work but we need to replace assist with follow.
			return;

			if (!assist_name)
			{
				return;
			}

			if (assist_id && IsAssistValid())
			{
				assist_distance = AssistPlayer->GetDistanceSquared(Avatar->Position3D);

				if (assist_distance > 25000)
				{
					FaceTarget(assist_id);

					V2 target_position;

					target_position.X = AssistPlayer->Position2D.X;
					target_position.Y = AssistPlayer->Position2D.Y;

					// Move towards the target
					int Speed = (byte)MovementSpeed::Run;

					if (OgreClient::Singleton->Data->VigorPoints < 10)
						Speed = (byte)MovementSpeed::Walk;

					Avatar->StartMoveTo(target_position, Speed);

					//OgreClient::Singleton->Data->RoomObjects->HasInvisibleRoomObject
				}
				else {
					// We are close enough to attack
					GetClosestAssistTarget(true);
				}
			}
		}

		void DroneBot::FastAttack()
		{
			try {
				if (target_id)
				{
					Meridian59::Data::Models::RoomObject^ target_room_obj = OgreClient::Singleton->Data->RoomObjects->GetItemByID(target_id);

					if (target_room_obj != nullptr && target_id && target_distance && target_distance <= attack_distance && target_room_obj->IsTarget)
					{
						OgreClient::Singleton->SendReqAttackMessage();
					}
				}
			}
			catch (...)
			{

			}
		}

		void DroneBot::FaceTarget(int to_target_id)
		{
			double face_diff = std::difftime(std::time(nullptr), face_timer);

			/*if (face_diff < 2)
			{
				return;
			}*/

			face_timer = std::time(nullptr);

			// Face the target continuously..
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
			Real smallestdist = Real::MaxValue;
			Real comfortable_distance = 500000 * 15;

			aggro = false;

			for each(RoomObject^ obj in OgreData->RoomObjects)
			{
				// object itself doesn't count
				if (obj != nullptr && obj->Flags->IsCreature && !obj->Name->Contains("soldier") && !obj->Name->Contains("army"))
				{
					Real dist = obj->GetDistanceSquared(Avatar->Position3D);

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
				}
			}

			if (smallestdist > comfortable_distance)
			{
				closest = nullptr;
			}

			if (closest != nullptr)
			{
				if (target_id != closest->ID)
				{
					target_changed = true;
					OgreClient::Singleton->LootAllStackables();
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
			ServerString^ new_message = OgreClient::Singleton->Data->ChatMessages->LastAddedItem;

			if (new_message != nullptr)
			{
				last_message = new_message->ToString();

				if (last_message->Contains("This place of safety"))
				{
					is_safe_logon = true;
					OgreClient::Singleton->Data->ChatMessages->Remove(new_message);
				}

				if (last_message->Contains("You can't see") || last_message->Contains("You can't reach"))
				{
					cant_see_target = true;
					OgreClient::Singleton->Data->ChatMessages->Remove(new_message);
				}
			}
		}

		// While ogre performs live updates, each tick will perform an action
		bool DroneBot::Tick()
		{
			UpdateChatStates();

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

					account_type = OgreClient::Singleton->Data->AccountType;

					if (assist && assist_name)
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
				ok = true;
				seek_destroy = false;
				follow = true;
				Log("Auto-Follow ON.", true);
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
			if (input->Contains("/assist") && input->Contains("[") && input->Contains("]") && !assist)
			{
				// Wait for a valid player name in quotes.
				ok = true;

				// Stop all other automated actions
				StopEverything();

				assist = true;
				assist_name = input->Substring(input->IndexOf("[") + 1);
				assist_name = assist_name->Substring(0, assist_name->IndexOf("]"));

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

