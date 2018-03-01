#include "stdafx.h"

namespace Meridian59 {
	namespace Ogre
	{
		void ControllerUI::GoList::Initialize()
		{
			// setup references to children from xml nodes
			Window = static_cast<CEGUI::FrameWindow*>(guiRoot->getChild(UI_NAME_GOLIST_WINDOW));
			List = static_cast<CEGUI::ItemListbox*>(Window->getChild(UI_NAME_GOLIST_LIST));
			GoNow = static_cast<CEGUI::PushButton*>(Window->getChild(UI_NAME_GOLIST_GONOW));

			// disable multiselect - you can't be everywhere at the same time.
			List->setMultiSelectEnabled(false);

			// subscribe go now button
			GoNow->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(UICallbacks::GoList::OnGoNowClicked));

			// subscribe close button
			Window->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber(UICallbacks::GoList::OnWindowClosed));

			// subscribe keyup
			Window->subscribeEvent(CEGUI::FrameWindow::EventKeyUp, CEGUI::Event::Subscriber(UICallbacks::GoList::OnWindowKeyUp));

			// add items to list
			LoadList();
		};

		void ControllerUI::GoList::Destroy()
		{
		};

		void ControllerUI::GoList::ApplyLanguage()
		{
		};

		void ControllerUI::GoList::ItemAdd(int Rid, String Name, String RoomName)
		{
			String NameToUse = Name;

			if (RoomName != "")
			{
				NameToUse = RoomName;
			}

			CEGUI::WindowManager* wndMgr = CEGUI::WindowManager::getSingletonPtr();

			// create widget (item)
			CEGUI::ItemEntry* widget = (CEGUI::ItemEntry*)wndMgr->createWindow(
				UI_WINDOWTYPE_OBJECTBASELISTBOXITEM);

			// set ID (RID)
			widget->setID(Rid);

			// get namelabel child
			CEGUI::Window* icon = widget->getChildAtIdx(UI_GOLIST_CHILDINDEX_ICON);
			CEGUI::Window* name = widget->getChildAtIdx(UI_GOLIST_CHILDINDEX_NAME);
			CEGUI::Window* amount = widget->getChildAtIdx(UI_GOLIST_CHILDINDEX_AMOUNT);

			// Hide the icon and amount boxes.
			// Not sure how they got there anyways - I guess I need to copy/paste less.
			icon->disable();
			icon->setVisible(false);
			amount->disable();
			amount->setVisible(false);

			// convert name
			const CEGUI::String& roomName = StringConvert::CLRToCEGUI(StringConvert::OgreToCLR(NameToUse));

			// set name
			name->setText(roomName);

			// subscribe click event
			name->subscribeEvent(
				CEGUI::Window::EventMouseDoubleClick,
				CEGUI::Event::Subscriber(UICallbacks::Welcome::OnItemDoubleClick));

			List->addItem(widget);

			// fix a bug with last item not selectable
			// when insertItem was used
			List->notifyScreenAreaChanged(true);
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool UICallbacks::GoList::OnItemDoubleClick(const CEGUI::EventArgs& e)
		{
			const CEGUI::WindowEventArgs& args = static_cast<const CEGUI::WindowEventArgs&>(e);
			CEGUI::ItemListbox* list = ControllerUI::GoList::List;
			CEGUI::ItemEntry* room = list->getFirstSelectedItem();

			if (room == nullptr)
			{
				return true;
			}

			OgreClient::Singleton->SendReqDM(DMCommandType::GoRoom, room->getID().ToString());

			ControllerUI::ActivateRoot();

			return true;
		};

		bool UICallbacks::GoList::OnGoNowClicked(const CEGUI::EventArgs& e)
		{
			const CEGUI::WindowEventArgs& args = static_cast<const CEGUI::WindowEventArgs&>(e);
			CEGUI::ItemListbox* list = ControllerUI::GoList::List;
			CEGUI::ItemEntry* room = list->getFirstSelectedItem();

			if (room == nullptr)
			{
				return true;
			}

			OgreClient::Singleton->SendReqDM(DMCommandType::GoRoom, room->getID().ToString());

			ControllerUI::ActivateRoot();

			return true;
		};

		bool UICallbacks::GoList::OnWindowKeyUp(const CEGUI::EventArgs& e)
		{
			const CEGUI::KeyEventArgs& args = static_cast<const CEGUI::KeyEventArgs&>(e);

			// close window on ESC
			if (args.scancode == CEGUI::Key::Escape)
			{
				// mark GUIroot active
				ControllerUI::ActivateRoot();
			}

			return UICallbacks::OnKeyUp(args);
		};

		bool UICallbacks::GoList::OnWindowClosed(const CEGUI::EventArgs& e)
		{
			// mark GUIroot active
			ControllerUI::ActivateRoot();

			return true;
		};

		void ControllerUI::GoList::LoadList()
		{
			/**
			 * This list was compiled using MS Excel and several other tools for gathering the information.
			 * You're welcome to append / modify it manually, or you can just look for the Excel file next to your Meridian59.sln.
			 */
			ControllerUI::GoList::ItemAdd(2110, "RID_KA0", "A dark, humid cavern");
			ControllerUI::GoList::ItemAdd(27, "RID_CAVE2", "A Deep, Dark, Spooky, Icky Cave");
			ControllerUI::GoList::ItemAdd(2010, "RID_KOC_SEWER1", "A foul place to wander");
			ControllerUI::GoList::ItemAdd(1501, "RID_DESERTPATH1", "A harrowing cleft");
			ControllerUI::GoList::ItemAdd(2113, "RID_KA3", "A notch in the jungle");
			ControllerUI::GoList::ItemAdd(110, "RID_ASSHQ", "A shadowy corner");
			ControllerUI::GoList::ItemAdd(2510, "RID_OC5B", "A spring of vitality");
			ControllerUI::GoList::ItemAdd(2509, "RID_OC1A", "A Tranquil Place");
			ControllerUI::GoList::ItemAdd(57, "RID_TOS_GREY", "Abandoned Building (Grey)");
			ControllerUI::GoList::ItemAdd(56, "RID_TOS_TAN", "Abandoned Building (Tan)");
			ControllerUI::GoList::ItemAdd(355, "RID_JAS_AB5", "Abandoned Building 1");
			ControllerUI::GoList::ItemAdd(356, "RID_JAS_AB6", "Abandoned Building 2");
			ControllerUI::GoList::ItemAdd(357, "RID_JAS_AB7", "Abandoned Building 3");
			ControllerUI::GoList::ItemAdd(358, "RID_JAS_AB8", "Abandoned Building 4");
			ControllerUI::GoList::ItemAdd(359, "RID_JAS_AB9", "Abandoned Building 5");
			ControllerUI::GoList::ItemAdd(361, "RID_JAS_AB11", "Abandoned Counting Room");
			ControllerUI::GoList::ItemAdd(76, "RID_TOS_OLD_TAVERN", "Abandoned Fountain Tavern");
			ControllerUI::GoList::ItemAdd(77, "RID_TOS_STORAGE", "Abandoned Storage Room");
			ControllerUI::GoList::ItemAdd(1511, "RID_DESERTRIVER2", "Abyssal Bore");
			ControllerUI::GoList::ItemAdd(105, "RID_BAR_HALL", "Adventurer's Hall of Barloque");
			ControllerUI::GoList::ItemAdd(372, "RID_JAS_HALL", "Adventurer's Hall of Jasper");
			ControllerUI::GoList::ItemAdd(2602, "RID_MAR_CRYPT3A", "Affirmation of the Forsaken");
			ControllerUI::GoList::ItemAdd(579, "RID_G9", "An ancient place, its origin forgotten");
			ControllerUI::GoList::ItemAdd(2501, "RID_OC02", "An Enigmatic Chamber");
			ControllerUI::GoList::ItemAdd(831, "RID_BRAX_BUILDING1", "Ancient Building 1");
			ControllerUI::GoList::ItemAdd(832, "RID_BRAX_BUILDING2", "Ancient Building 2");
			ControllerUI::GoList::ItemAdd(827, "RID_BRAX_GRAVEYARD", "Ancient Graveyard of Brax");
			ControllerUI::GoList::ItemAdd(2503, "RID_OC04", "Aqueducts of Merb");
			ControllerUI::GoList::ItemAdd(2154, "RID_KE4", "Avar Village");
			ControllerUI::GoList::ItemAdd(850, "RID_BOURGEOIS", "Banquet Hall of the Bourgeois Castle");
			ControllerUI::GoList::ItemAdd(951, "RID_DUKE2", "Blackstone Keep");
			ControllerUI::GoList::ItemAdd(851, "RID_CASTLE2B", "Bourgeois Castle");
			ControllerUI::GoList::ItemAdd(2508, "RID_OCPB", "Brog's Antechamber");
			ControllerUI::GoList::ItemAdd(106, "RID_BAR_INN", "Brownestone Inn");
			ControllerUI::GoList::ItemAdd(2123, "RID_KB3", "Camp of the Warrior Avars");
			ControllerUI::GoList::ItemAdd(38, "RID_CASTLE1", "Castle Victoria");
			ControllerUI::GoList::ItemAdd(857, "RID_FORGOTTEN_TOO", "Chamber of the Forgotten Heroes");
			ControllerUI::GoList::ItemAdd(854, "RID_MOCKERS", "Chamber of the Mocker");
			ControllerUI::GoList::ItemAdd(153, "RID_COR_INN", "Cibilo Creek Inn");
			ControllerUI::GoList::ItemAdd(2111, "RID_KA1", "Cliff Hive");
			ControllerUI::GoList::ItemAdd(1510, "RID_DESERTRIVER1", "Cliffbound River");
			ControllerUI::GoList::ItemAdd(1512, "RID_DESERTBRIDGE", "Collapsed Causeway");
			ControllerUI::GoList::ItemAdd(150, "RID_CORNOTH", "Cor Noth");
			ControllerUI::GoList::ItemAdd(828, "RID_BRAX", "Decaying City of Brax");
			ControllerUI::GoList::ItemAdd(556, "RID_E6", "Deep Forest of Farol");
			ControllerUI::GoList::ItemAdd(546, "RID_D6", "Deep in the Forest of Farol");
			ControllerUI::GoList::ItemAdd(2143, "RID_KD3", "Deep in the Jungle");
			ControllerUI::GoList::ItemAdd(534, "RID_C4", "Deep Woods of Ileria");
			ControllerUI::GoList::ItemAdd(354, "RID_JAS_AB4", "Deserted Eating House");
			ControllerUI::GoList::ItemAdd(541, "RID_D1", "Dirhadrim");
			ControllerUI::GoList::ItemAdd(360, "RID_JAS_AB10", "Disused Shop");
			ControllerUI::GoList::ItemAdd(526, "RID_B6", "Druid Hills");
			ControllerUI::GoList::ItemAdd(363, "RID_JAS_AB13", "Dusty Chamber");
			ControllerUI::GoList::ItemAdd(61, "RID_EAST_TOS", "East Ende");
			ControllerUI::GoList::ItemAdd(350, "RID_JAS_EAST", "East Jasper");
			ControllerUI::GoList::ItemAdd(564, "RID_F4", "East Merchant Way");
			ControllerUI::GoList::ItemAdd(554, "RID_E4", "East Merchant Way through Ilerian Woods");
			ControllerUI::GoList::ItemAdd(521, "RID_B1", "Elhadrim");
			ControllerUI::GoList::ItemAdd(307, "RID_RAZA_BAR", "Eric's Stout Spirits");
			ControllerUI::GoList::ItemAdd(52, "RID_TOS_INN", "Familiars");
			ControllerUI::GoList::ItemAdd(37, "RID_CASTLE1D", "Far'Nohl's Chambers");
			ControllerUI::GoList::ItemAdd(537, "RID_C7", "Faronath, home of the TreeFolk");
			ControllerUI::GoList::ItemAdd(54, "RID_TOS_BANK", "First Royal Bank of Tos");
			ControllerUI::GoList::ItemAdd(536, "RID_C6", "Forest of Farol");
			ControllerUI::GoList::ItemAdd(58, "RID_TOS_FORGOTTEN", "Forgotten Lover's Tryst");
			ControllerUI::GoList::ItemAdd(53, "RID_TOS_APOTH", "Frisconar's Mysticals");
			ControllerUI::GoList::ItemAdd(373, "RID_JAS_STORE", "General Store of Jasper");
			ControllerUI::GoList::ItemAdd(833, "RID_BRAX_ARENA", "Goad's Grinder");
			ControllerUI::GoList::ItemAdd(2507, "RID_OCPA", "Grog's Antechamber");
			ControllerUI::GoList::ItemAdd(59, "RID_TOS_FORGET", "Hall of the Forgotten Heroes");
			ControllerUI::GoList::ItemAdd(2122, "RID_KB2", "Headquarters for the Avar Warriors");
			ControllerUI::GoList::ItemAdd(2152, "RID_KE2", "Headquarters of the Noble Avars");
			ControllerUI::GoList::ItemAdd(9000, "RID_HISTORIC", "Historic Jasper");
			ControllerUI::GoList::ItemAdd(9001, "RID_OLD_MARION", "Historic Marion");
			ControllerUI::GoList::ItemAdd(9002, "RID_OBARLN", "Historic North Barloque");
			ControllerUI::GoList::ItemAdd(9003, "RID_OBARLS", "Historic South Barloque");
			ControllerUI::GoList::ItemAdd(2136, "RID_MAD_SCIENTIST_HUT", "Hut of the Mad Scientist");
			ControllerUI::GoList::ItemAdd(352, "RID_JAS_AB2", "Icehouse");
			ControllerUI::GoList::ItemAdd(715, "RID_GUILDH_MAX", "Inner Sanctum of Queen Venya'cyr");
			ControllerUI::GoList::ItemAdd(2114, "RID_KA4", "Into the Jungle");
			ControllerUI::GoList::ItemAdd(2125, "RID_KB5", "Into the Jungle");
			ControllerUI::GoList::ItemAdd(710, "RID_GUILDH10", "Jaarba's Abode");
			ControllerUI::GoList::ItemAdd(104, "RID_BAR_APOTH", "Joguer's Herbs and Roots");
			ControllerUI::GoList::ItemAdd(49, "RID_BADLAND2", "Kardde's Canyon");
			ControllerUI::GoList::ItemAdd(2000, "RID_KOC_NORTH", "Ko'catan, the Island Settlement");
			ControllerUI::GoList::ItemAdd(2009, "RID_KOC_SOUTH", "Ko'catan, the Island Settlement");
			ControllerUI::GoList::ItemAdd(707, "RID_GUILDH5", "Konima's Abandoned Dwelling");
			ControllerUI::GoList::ItemAdd(2505, "RID_OC06", "Konima's Ascension");
			ControllerUI::GoList::ItemAdd(205, "RID_MAR_HEALER_SHOP", "Lady Aftyn's Sanctuary");
			ControllerUI::GoList::ItemAdd(2141, "RID_KD1", "Lair of the Bone Priestess");
			ControllerUI::GoList::ItemAdd(2506, "RID_OCP1", "Lair of the Orc Pit Boss");
			ControllerUI::GoList::ItemAdd(568, "RID_F8", "Lake of Jala's Song");
			ControllerUI::GoList::ItemAdd(542, "RID_D2", "Lower Dirhadrim");
			ControllerUI::GoList::ItemAdd(522, "RID_B2", "Lower Elhadrim");
			ControllerUI::GoList::ItemAdd(856, "RID_CASTLE2E", "Madelia's Fine Peacockeries");
			ControllerUI::GoList::ItemAdd(593, "RID_I3", "Main gate of Barloque-- Royal City of Meridian");
			ControllerUI::GoList::ItemAdd(574, "RID_G4", "Main gate to Cor Noth");
			ControllerUI::GoList::ItemAdd(586, "RID_H6", "Main gate to the city of Tos");
			ControllerUI::GoList::ItemAdd(200, "RID_MARION", "Marion");
			ControllerUI::GoList::ItemAdd(47, "RID_CANYON2", "Martyr's Battleground");
			ControllerUI::GoList::ItemAdd(306, "RID_RAZA_CRYPT", "Mausoleum");
			ControllerUI::GoList::ItemAdd(701, "RID_GUILDH_MIN", "Mercenary Alley");
			ControllerUI::GoList::ItemAdd(902, "RID_UNIV", "Meridian University");
			ControllerUI::GoList::ItemAdd(101, "RID_BAR_NORTH", "North Barloque");
			ControllerUI::GoList::ItemAdd(511, "RID_A1", "North Quilicia Wood 1");
			ControllerUI::GoList::ItemAdd(512, "RID_A2", "North Quilicia Wood 2");
			ControllerUI::GoList::ItemAdd(513, "RID_A3", "North Quilicia Wood 3");
			ControllerUI::GoList::ItemAdd(514, "RID_A4", "North Quilicia Wood 4");
			ControllerUI::GoList::ItemAdd(2504, "RID_OC05", "Obfal's Sepulchre");
			ControllerUI::GoList::ItemAdd(903, "RID_GALLERY", "object gallery");
			ControllerUI::GoList::ItemAdd(566, "RID_F6", "Off the beaten path 1");
			ControllerUI::GoList::ItemAdd(567, "RID_F7", "Off the beaten path 2");
			ControllerUI::GoList::ItemAdd(114, "RID_BAR_VAULT", "Office of the Barloque Vaultman");
			ControllerUI::GoList::ItemAdd(115, "RID_BAR_COURT", "Office of the Justicar");
			ControllerUI::GoList::ItemAdd(362, "RID_JAS_AB12", "Old Barracks");
			ControllerUI::GoList::ItemAdd(351, "RID_JAS_AB1", "Old Granary ");
			ControllerUI::GoList::ItemAdd(116, "RID_BAR_JAIL", "Old Jailhouse");
			ControllerUI::GoList::ItemAdd(353, "RID_JAS_AB3", "Old Schoolhouse");
			ControllerUI::GoList::ItemAdd(2134, "RID_KC4", "Origin of the Kriipa");
			ControllerUI::GoList::ItemAdd(43, "RID_OOG", "Out of Grace with the Higher Powers");
			ControllerUI::GoList::ItemAdd(2, "RID_CASTLE1C", "Outside Castle Victoria");
			ControllerUI::GoList::ItemAdd(2115, "RID_KA5", "Outside the Guard Tower");
			ControllerUI::GoList::ItemAdd(2135, "RID_KC5", "Outside the Mad Scientist's Hut");
			ControllerUI::GoList::ItemAdd(583, "RID_H3", "Outskirts of Barloque");
			ControllerUI::GoList::ItemAdd(330, "RID_RAZA_FOREST", "Outskirts of Raza");
			ControllerUI::GoList::ItemAdd(596, "RID_I6", "Outskirts of Tos");
			ControllerUI::GoList::ItemAdd(2124, "RID_KB4", "Path to the Ruins");
			ControllerUI::GoList::ItemAdd(371, "RID_JAS_BAR", "Pietro's Wicked Brews");
			ControllerUI::GoList::ItemAdd(533, "RID_C3", "Quilicia Wood");
			ControllerUI::GoList::ItemAdd(374, "RID_JAS_SMITHY", "Quintor's Smithy");
			ControllerUI::GoList::ItemAdd(304, "RID_RAZA_APOTH", "Ravi's Magicks of Raza");
			ControllerUI::GoList::ItemAdd(300, "RID_RAZA", "Raza");
			ControllerUI::GoList::ItemAdd(2601, "RID_MAR_CRYPT2", "Resting place of Marion's ancestors");
			ControllerUI::GoList::ItemAdd(853, "RID_CASTLE2D", "Royal Library of the Bourgeois Castle");
			ControllerUI::GoList::ItemAdd(830, "RID_BRAX_CASTLE2", "Ruins of Castle Brax");
			ControllerUI::GoList::ItemAdd(2133, "RID_KC3", "Ruins of the Riija Temple");
			ControllerUI::GoList::ItemAdd(1502, "RID_DESERTCLIFFACCESS", "Sandbreak Cliffs 1");
			ControllerUI::GoList::ItemAdd(1503, "RID_DESERTCLIFFNOACCESS1", "Sandbreak Cliffs 2");
			ControllerUI::GoList::ItemAdd(1504, "RID_DESERTCLIFFNOACCESS2", "Sandbreak Cliffs 3");
			ControllerUI::GoList::ItemAdd(1505, "RID_DESERTCLIFFNOACCESS3", "Sandbreak Cliffs 4");
			ControllerUI::GoList::ItemAdd(515, "RID_A5", "Seafarer's Peak");
			ControllerUI::GoList::ItemAdd(75, "RID_TOS_SECRET_PASSAGE", "Secret Passage");
			ControllerUI::GoList::ItemAdd(1507, "RID_DESERTSHORE1", "Solitary Shoreline 1");
			ControllerUI::GoList::ItemAdd(1508, "RID_DESERTSHORE2", "Solitary Shoreline 2");
			ControllerUI::GoList::ItemAdd(1509, "RID_DESERTSHORE3", "Solitary Shoreline 3");
			ControllerUI::GoList::ItemAdd(151, "RID_COR_GROCER", "Solomon's Edibles");
			ControllerUI::GoList::ItemAdd(563, "RID_F3", "Source of the River Ille");
			ControllerUI::GoList::ItemAdd(102, "RID_BAR_SOUTH", "South Barloque");
			ControllerUI::GoList::ItemAdd(2142, "RID_KD2", "Territory of the Noble Avars");
			ControllerUI::GoList::ItemAdd(709, "RID_GUILDH9", "The Abandoned Warehouse");
			ControllerUI::GoList::ItemAdd(152, "RID_COR_HALL", "The Adventurer's Hall of Cor Noth");
			ControllerUI::GoList::ItemAdd(204, "RID_MAR_HALL", "The Adventurer's Hall of Marion");
			ControllerUI::GoList::ItemAdd(302, "RID_RAZA_HALL", "The Adventurer's Hall of Raza");
			ControllerUI::GoList::ItemAdd(72, "RID_TOS_HALL", "The Adventurer's Hall of Tos");
			ControllerUI::GoList::ItemAdd(2001, "RID_KOC_INN", "The Aerie Guest House");
			ControllerUI::GoList::ItemAdd(380, "RID_SEWER_KING", "The Ancient Smuggler's Crossing");
			ControllerUI::GoList::ItemAdd(40, "RID_THRONE1", "The Antechamber of Victoria Castle");
			ControllerUI::GoList::ItemAdd(2502, "RID_OC03", "The Archaic Hollows");
			ControllerUI::GoList::ItemAdd(60, "RID_TOS_ARENA", "The Arena of Kraanan");
			ControllerUI::GoList::ItemAdd(73, "RID_TOS_ARENA2", "The Arena of Kraanan");
			ControllerUI::GoList::ItemAdd(852, "RID_CASTLE2C", "The Assembly Chamber");
			ControllerUI::GoList::ItemAdd(45, "RID_BADLAND1", "The Badlands");
			ControllerUI::GoList::ItemAdd(615, "RID_K5", "The Badlands");
			ControllerUI::GoList::ItemAdd(2008, "RID_KOC_BAR", "The Barking Monster");
			ControllerUI::GoList::ItemAdd(32, "RID_CANYON1", "The Berdonne Canyons");
			ControllerUI::GoList::ItemAdd(103, "RID_BAR_BAR", "The Bhrama & Falcon");
			ControllerUI::GoList::ItemAdd(1506, "RID_DESERTDUNES", "The Black Desert");
			ControllerUI::GoList::ItemAdd(303, "RID_RAZA_SMITH", "The Blacksmith of Raza");
			ControllerUI::GoList::ItemAdd(714, "RID_GUILDH14", "The Bookmaker's Guild House");
			ControllerUI::GoList::ItemAdd(585, "RID_BOB", "The border of the Badlands");
			ControllerUI::GoList::ItemAdd(2100, "RID_BOWMAKER", "The Bowmaker's Hut");
			ControllerUI::GoList::ItemAdd(74, "RID_TOS_INN_CELLAR", "The Cellar");
			ControllerUI::GoList::ItemAdd(2102, "RID_JUNGLE_TRADING_POST_CELLAR", "The Cellar Rooms");
			ControllerUI::GoList::ItemAdd(952, "RID_DUKE3", "The Chambers of Duke Akardius");
			ControllerUI::GoList::ItemAdd(2131, "RID_CHASM", "The Chasm");
			ControllerUI::GoList::ItemAdd(2003, "RID_KOC_SMITHY", "The Chimney House");
			ControllerUI::GoList::ItemAdd(2144, "RID_KD4", "The Clearing of the Trading Post");
			ControllerUI::GoList::ItemAdd(2121, "RID_KB1", "The Cliffs");
			ControllerUI::GoList::ItemAdd(855, "RID_BAZMANS_ROOM", "The Councilors' Chamber");
			ControllerUI::GoList::ItemAdd(950, "RID_BLACKSTONE", "The Courtyard of Blackstone Keep");
			ControllerUI::GoList::ItemAdd(578, "RID_CRAGGED", "The Cragged Mountains");
			ControllerUI::GoList::ItemAdd(598, "RID_I8", "The Cragged Mountains");
			ControllerUI::GoList::ItemAdd(2006, "RID_KOC_TAILOR", "The Crooked Hem Garment Shop");
			ControllerUI::GoList::ItemAdd(71, "RID_TOS_CRYPT", "The Crypt");
			ControllerUI::GoList::ItemAdd(2600, "RID_MAR_CRYPT1", "The crypt in Marion");
			ControllerUI::GoList::ItemAdd(6, "RID_FOREST5", "The Deep Dark Woods of Marion");
			ControllerUI::GoList::ItemAdd(4, "RID_FOREST4", "The Deep Dark Woods of Tos");
			ControllerUI::GoList::ItemAdd(703, "RID_GUILDH3", "The Den of Shadows");
			ControllerUI::GoList::ItemAdd(750, "RID_ICE_CAVE1", "The Dreaded Caves of Ice");
			ControllerUI::GoList::ItemAdd(953, "RID_DUKE4", "The Duke's Feast Hall");
			ControllerUI::GoList::ItemAdd(954, "RID_DUKE5", "The Duke's Grand Ballroom");
			ControllerUI::GoList::ItemAdd(825, "RID_BRAX_MAZE", "The Dungeon of Castle Brax");
			ControllerUI::GoList::ItemAdd(2013, "RID_KOC_GUARDTOWER_EAST", "The East Tower");
			ControllerUI::GoList::ItemAdd(531, "RID_C1", "The Fey's Crossing");
			ControllerUI::GoList::ItemAdd(3, "RID_FIELD1", "The Fields");
			ControllerUI::GoList::ItemAdd(584, "RID_H4", "The Flatlands");
			ControllerUI::GoList::ItemAdd(555, "RID_E5", "The Forest Shrine");
			ControllerUI::GoList::ItemAdd(28, "RID_FOREST3", "The Forests of Meridian");
			ControllerUI::GoList::ItemAdd(24, "RID_FOREST1", "The Forests of Meridian 1");
			ControllerUI::GoList::ItemAdd(26, "RID_FOREST2", "The Forests of Meridian 2");
			ControllerUI::GoList::ItemAdd(51, "RID_TOS_SMITHY", "The Freelance Merchand and Menders Shop");
			ControllerUI::GoList::ItemAdd(308, "RID_RAZA_MUSEUM", "The Grand Museum of Raza");
			ControllerUI::GoList::ItemAdd(70, "RID_TOS_GRAVEYARD", "The Graveyard of Tos");
			ControllerUI::GoList::ItemAdd(552, "RID_E2", "The Great Ocean");
			ControllerUI::GoList::ItemAdd(700, "RID_GM_HALL", "The Guildmaster's Hall");
			ControllerUI::GoList::ItemAdd(702, "RID_GUILDH2", "The Hall of Explorers");
			ControllerUI::GoList::ItemAdd(156, "RID_COR_UNIV", "The Hall of Genealogy");
			ControllerUI::GoList::ItemAdd(2007, "RID_KOC_HALL", "The Hall of Heroes");
			ControllerUI::GoList::ItemAdd(2015, "RID_KOC_HALL_OF_HEROES_B", "The Hall of Heroes East");
			ControllerUI::GoList::ItemAdd(2014, "RID_KOC_HALL_OF_HEROES_A", "The Hall of Heroes West");
			ControllerUI::GoList::ItemAdd(713, "RID_GUILDH13", "The Hall of the Slaughtered Command");
			ControllerUI::GoList::ItemAdd(36, "RID_HERMHUT", "The Hermit's Hut");
			ControllerUI::GoList::ItemAdd(516, "RID_A6", "The Hills");
			ControllerUI::GoList::ItemAdd(305, "RID_RAZA_ELDER", "The Home of Roderic D'Stane");
			ControllerUI::GoList::ItemAdd(203, "RID_MAR_ELDER_HUT", "The home of the elder");
			ControllerUI::GoList::ItemAdd(712, "RID_GUILDH11", "The home of the Friends of the Forest");
			ControllerUI::GoList::ItemAdd(375, "RID_JAS_ELDER_HUT", "The Home of the Wise Man");
			ControllerUI::GoList::ItemAdd(2005, "RID_KOC_BANK", "The Hungry Vaults");
			ControllerUI::GoList::ItemAdd(301, "RID_RAZA_INN", "The Inn of Raza");
			ControllerUI::GoList::ItemAdd(711, "RID_GUILDH6", "The Ivory Chapel");
			ControllerUI::GoList::ItemAdd(2112, "RID_KA2", "The Jungle Cliffs");
			ControllerUI::GoList::ItemAdd(575, "RID_G5", "The King's Way 1");
			ControllerUI::GoList::ItemAdd(576, "RID_G6", "The King's Way 2");
			ControllerUI::GoList::ItemAdd(202, "RID_MAR_INN", "The Limping Toad Inn and Tavern");
			ControllerUI::GoList::ItemAdd(901, "RID_GODROOM", "The meeting place of the Gods");
			ControllerUI::GoList::ItemAdd(2004, "RID_KOC_STORE", "The Neglected Arsenal");
			ControllerUI::GoList::ItemAdd(706, "RID_GUILDH7", "The Old Dwarven Hall");
			ControllerUI::GoList::ItemAdd(107, "RID_BAR_PORT", "The Ports of Barloque");
			ControllerUI::GoList::ItemAdd(603, "RID_J3", "The Queen's Way");
			ControllerUI::GoList::ItemAdd(2101, "RID_JUNGLE_TRADING_POST", "The Remote Trading Post of Wulfgang zax'Ak");
			ControllerUI::GoList::ItemAdd(376, "RID_JAS_BANK", "The Royal Bank of Jasper");
			ControllerUI::GoList::ItemAdd(113, "RID_BAR_SMITHY", "The Royal Blacksmith of Barloque");
			ControllerUI::GoList::ItemAdd(562, "RID_F2", "The sandy shores of the Great Ocean");
			ControllerUI::GoList::ItemAdd(708, "RID_GUILDH8", "The Sewer Hideout");
			ControllerUI::GoList::ItemAdd(108, "RID_BAR_SEWER", "The Sewers of Barloque");
			ControllerUI::GoList::ItemAdd(111, "RID_BAR_SEWER2", "The Sewers of Barloque 2");
			ControllerUI::GoList::ItemAdd(112, "RID_BAR_SEWER3", "The Sewers of Barloque 3");
			ControllerUI::GoList::ItemAdd(377, "RID_JAS_SEWER1", "The Sewers of Jasper 1");
			ControllerUI::GoList::ItemAdd(378, "RID_JAS_SEWER2", "The Sewers of Jasper 2");
			ControllerUI::GoList::ItemAdd(379, "RID_JAS_SEWER3", "The Sewers of Jasper 3");
			ControllerUI::GoList::ItemAdd(78, "RID_TOS_SMUGGLERS_WAY", "The Smuggler's Way");
			ControllerUI::GoList::ItemAdd(109, "RID_BAR_MERCHANT", "The Sparkling Stone Shop");
			ControllerUI::GoList::ItemAdd(35, "RID_NEST1", "The Spider Nest");
			ControllerUI::GoList::ItemAdd(155, "RID_COR_TAILOR", "The Spindle and the Spinster");
			ControllerUI::GoList::ItemAdd(50, "RID_TOS", "The Streets of Tos");
			ControllerUI::GoList::ItemAdd(557, "RID_E7", "The Sweet Grass Prairies");
			ControllerUI::GoList::ItemAdd(801, "RID_TEMPKRA", "The Temple of Kraanan");
			ControllerUI::GoList::ItemAdd(802, "RID_TEMPLE_QOR", "The Temple of Qor");
			ControllerUI::GoList::ItemAdd(2151, "RID_KE1", "The Temple of Riija");
			ControllerUI::GoList::ItemAdd(48, "RID_TEMPLE", "The Temple of Shal'ille");
			ControllerUI::GoList::ItemAdd(41, "RID_THRONE2", "The Throne Room of Victoria Castle");
			ControllerUI::GoList::ItemAdd(597, "RID_I7", "The Twisted Wood");
			ControllerUI::GoList::ItemAdd(5, "RID_CAVE3", "The Underground Lake");
			ControllerUI::GoList::ItemAdd(1, "RID_UNDERWORLD", "The Underworld");
			ControllerUI::GoList::ItemAdd(532, "RID_C2", "The Vale of Sorrows");
			ControllerUI::GoList::ItemAdd(384, "RID_JAS_VAULT", "The Vault of Jasper");
			ControllerUI::GoList::ItemAdd(154, "RID_COR_MASTER", "The Weapon Master's Abode");
			ControllerUI::GoList::ItemAdd(704, "RID_GUILDH4", "The Wryn's Keep");
			ControllerUI::GoList::ItemAdd(2132, "RID_KC2", "Turf of the Warrior Avars");
			ControllerUI::GoList::ItemAdd(2002, "RID_KOC_APOTH", "Two Lizards Magic Shop");
			ControllerUI::GoList::ItemAdd(2500, "RID_OC01", "Ugol's Warren Entrance");
			ControllerUI::GoList::ItemAdd(599, "RID_I9", "Ukgoth, Holy Land of Trolls");
			ControllerUI::GoList::ItemAdd(589, "RID_H9", "Under the shadow of the Sentinel");
			ControllerUI::GoList::ItemAdd(42, "RID_DUNGEON", "Underbasement of Victoria");
			ControllerUI::GoList::ItemAdd(2012, "RID_KOC_SEWER2", "Up Chuk Creek without a paddle");
			ControllerUI::GoList::ItemAdd(39, "RID_CASTLE1B", "Upstairs in Castle Victoria");
			ControllerUI::GoList::ItemAdd(544, "RID_D4", "Valley of Ileria");
			ControllerUI::GoList::ItemAdd(364, "RID_JAS_AB14", "Warm Haven ");
			ControllerUI::GoList::ItemAdd(1500, "RID_DESERT", "Waylay Oasis");
			ControllerUI::GoList::ItemAdd(382, "RID_JAS_WEST", "West Jasper");
			ControllerUI::GoList::ItemAdd(545, "RID_D5", "West Merchant Way");
			ControllerUI::GoList::ItemAdd(535, "RID_C5", "West Merchant Way through Ilerian Woods");
			ControllerUI::GoList::ItemAdd(705, "RID_GUILDH12", "West Tower Guild Hall in Ko'catan");
			ControllerUI::GoList::ItemAdd(587, "RID_H7", "Western border of the Twisted Wood");
			ControllerUI::GoList::ItemAdd(331, "RID_FAROL_WEST", "Western Edge of the Forest of Farol");
			ControllerUI::GoList::ItemAdd(826, "RID_BRAX_CAVES", "Winding Caverns");
			ControllerUI::GoList::ItemAdd(829, "RID_BRAX_CASTLE1", "Within the Walls of Castle Brax");
			ControllerUI::GoList::ItemAdd(201, "RID_MAR_SMITHY", "Ye Olde Slasher Salesman");
			ControllerUI::GoList::ItemAdd(370, "RID_JAS_INN", "Yonder Inn of Jasper");
		}
	};
};
