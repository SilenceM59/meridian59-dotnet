#pragma once

namespace Meridian59 {

	namespace Ogre
	{
		// ########################################
		// DroneBot Class
		// ########################################
		public ref class DroneBot
		{

		private:

			//static System::Collections::Generic::List<Meridian59::Data::Models::RoomObject^>^ VisitedTargets;
			static System::Collections::Generic::List<System::String^> IgnoredTargets;

		public:

			// ########################################
			// Name of the Player to assist
			// ########################################
			int assist_id;
			float assist_distance;
			System::String^ assist_name;
			Meridian59::Data::Models::RoomObject^ AssistPlayer;

			// ########################################
			// You will login phased automatically
			// ########################################
			bool safe_logon = true;
			bool safe_logged_on = false;
			bool safe_phase_casted = false;

			// ########################################
			// Timers
			// ########################################
			time_t seek_timer;
			time_t face_timer;
			time_t auto_convey;
			time_t last_eat;
			time_t last_loot;

			// ########################################
			// Ticks
			// ########################################
			int ticks_convey;
			int ticks_test;
			int ticks_tick;

			// ########################################
			// Health, Mana and Vigor
			// ########################################
			int HP;
			int MP;
			int VG;
			int HP_Max;
			int MP_Max;
			int VG_Max;
			float VG_Percent;
			float HP_Percent;
			float MP_Percent;

			// ########################################
			// Tour
			// ########################################
			int ticks_tour;
			int last_room_id;
			int last_tour_index = 0;
			int next_tour_index = 0;
			System::String^ last_room_roo_name;

			// ########################################
			// Threads Running
			// ########################################
			bool Running_SeekAndDestroy = false;

			// ########################################
			// Chat Window
			// ########################################
			static CEGUI::Window* Window;

			// ########################################
			// Chat Text
			// ########################################
			static CEGUI::Window* Text;

			// ########################################
			// Chat Input
			// ########################################
			static CEGUI::Editbox* Input;

			// ########################################
			// Your Character
			// ########################################
			Meridian59::Data::Models::RoomObject^ Avatar;
			AccountType account_type;
			int avatar_id;
			float last_x;
			float last_y;
			float last_z;

			// ########################################
			// Target
			// ########################################
			Meridian59::Data::Models::RoomObject^ Target;
			int target_id;
			int target_id_last;
			bool target_changed;
			float target_last_x;
			float target_last_y;
			float target_last_z;

			// ########################################
			// General Numeric
			// ########################################
			double sleepUntil;
			double currentTick;

			// ########################################
			// Aggro
			// ########################################
			int aggro_count;
			bool aggro;

			// ########################################
			// Statistics
			// ########################################
			int imps;
			int toughers;

			// ########################################
			// Booleans
			// ########################################
			bool allow_fast_attack = false;
			bool conveying = false;
			bool phasing = false;
			bool auto_phasing = false;
			bool auto_conveying = false;
			bool room_changed = false;
			bool game_saved = false;
			bool first_tick = true;
			bool try_on_next_tick = false;
			bool follow = false;
			bool assist = false;
			bool seek_destroy = false;
			bool dead = false;
			bool engaged = false;
			bool low = false;
			bool tired = false;
			bool stuck = false;
			bool is_safe_logon = false;

			// ########################################
			// Chat states
			// ########################################
			bool cant_see_target = false;
			System::String^ last_message;
			System::String^ last_room_name;

			// ########################################
			// Methods
			// ########################################
			void Init();
			bool Tick();
			bool Update();
			void UserInput();
			void UpdateChatStates();
			void Log(System::String^ message, bool toChat);
			void SetChatInput(CEGUI::String message);
			void AppendChat(System::String^ message);
			void StopEverything();
			void AutoFollow();
			void Assist();
			void SeekAndDestroy();
			void LogPosition();
			int GetClosestTarget(bool target_it);
			void UpdateAllowTicks();
			void FaceTarget(int to_target_id);
			void Convey();
			void Phase();
			void FastAttack();
			int PhaseNow(bool phase);
			bool IsAssistValid();
			int GetClosestAssistTarget(bool target_it);
			void Tour();

			// ########################################
			// __construct
			// ########################################
			static DroneBot(void);

		};
	};
};
