#include "stdafx.h"

namespace Meridian59 {
	namespace Ogre
	{
		void ControllerUI::ChangePassword::Initialize()
		{
			// setup references to children from xml nodes
			Window = static_cast<CEGUI::Window*>(guiRoot->getChild(UI_NAME_CHANGEPASSWORD_WINDOW));
			OldPassword = static_cast<CEGUI::Editbox*>(Window->getChild(UI_NAME_CHANGEPASSWORD_OLDPASSWORD));
			OldPasswordDesc = static_cast<CEGUI::Window*>(Window->getChild(UI_NAME_CHANGEPASSWORD_OLDPASSWORDDESC));
			NewPassword = static_cast<CEGUI::Editbox*>(Window->getChild(UI_NAME_CHANGEPASSWORD_NEWPASSWORD));
			NewPasswordDesc = static_cast<CEGUI::Window*>(Window->getChild(UI_NAME_CHANGEPASSWORD_NEWPASSWORDDESC));
			ConfirmPassword = static_cast<CEGUI::Editbox*>(Window->getChild(UI_NAME_CHANGEPASSWORD_CONFIRMPASSWORD));
			ConfirmPasswordDesc = static_cast<CEGUI::Window*>(Window->getChild(UI_NAME_CHANGEPASSWORD_CONFIRMPASSWORDDESC));

			Change = static_cast<CEGUI::PushButton*>(Window->getChild(UI_NAME_CHANGEPASSWORD_CHANGE));

			Window->setMousePassThroughEnabled(true);
			Window->setMouseInputPropagationEnabled(true);
			Window->setZOrderingEnabled(false);

			OgreClientConfig^ config = OgreClient::Singleton->Config;

			// subscribe return on passwordbox
			OldPassword->subscribeEvent(CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber(UICallbacks::ChangePassword::OnOldPasswordKeyUp));
			NewPassword->subscribeEvent(CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber(UICallbacks::ChangePassword::OnNewPasswordKeyUp));
			ConfirmPassword->subscribeEvent(CEGUI::Editbox::EventKeyUp, CEGUI::Event::Subscriber(UICallbacks::ChangePassword::OnConfirmPasswordKeyUp));

			// subscribe buttons
			Change->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(UICallbacks::ChangePassword::OnChangeClicked));

			// subscribe close button
			Window->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber(UICallbacks::OnWindowClosed));
		};

		void ControllerUI::ChangePassword::Destroy()
		{
		};

		void ControllerUI::ChangePassword::ApplyLanguage()
		{
			OldPasswordDesc->setText(GetLangLabel(LANGSTR::OLDPASSWORD));
			NewPasswordDesc->setText(GetLangLabel(LANGSTR::NEWPASSWORD));
			ConfirmPasswordDesc->setText(GetLangLabel(LANGSTR::CONFIRMPASSWORD));
			Change->setText(GetLangLabel(LANGSTR::CHANGE));
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool UICallbacks::ChangePassword::OnOldPasswordKeyUp(const CEGUI::EventArgs& e)
		{
			const CEGUI::KeyEventArgs& args = (const CEGUI::KeyEventArgs&)e;
			const CEGUI::Editbox* editbox = (const CEGUI::Editbox*)args.window;

			if (args.scancode == ::CEGUI::Key::Scan::Tab)
				ControllerUI::ChangePassword::NewPassword->activate();

			return true;
		};

		bool UICallbacks::ChangePassword::OnNewPasswordKeyUp(const CEGUI::EventArgs& e)
		{
			const CEGUI::KeyEventArgs& args = (const CEGUI::KeyEventArgs&)e;
			const CEGUI::Editbox* editbox = (const CEGUI::Editbox*)args.window;

			if (args.scancode == ::CEGUI::Key::Scan::Tab)
				ControllerUI::ChangePassword::ConfirmPassword->activate();

			return true;
		};

		bool UICallbacks::ChangePassword::OnConfirmPasswordKeyUp(const CEGUI::EventArgs& e)
		{
			const CEGUI::KeyEventArgs& args = (const CEGUI::KeyEventArgs&)e;
			const CEGUI::Editbox* editbox = (const CEGUI::Editbox*)args.window;

			if (args.scancode == ::CEGUI::Key::Scan::Return &&
				!ControllerUI::ChangePassword::Change->isDisabled())
			{
				// Copy of "OnChangeClicked"
			}

			return true;
		};

		bool UICallbacks::ChangePassword::OnChangeClicked(const CEGUI::EventArgs& e)
		{
			const CEGUI::WindowEventArgs& args = (const CEGUI::WindowEventArgs&)e;
			CEGUI::PushButton* btn = (CEGUI::PushButton*)args.window;

			//OgreClient::Singleton->SendUserCommand(nullptr);
			//ControllerUI::ChangePassword::Window->setEnabled(false);

			bool empty_field = false;
			bool old_password_match = false;
			bool new_password_match = false;

			if (StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::OldPassword->getText()) == "" || StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::NewPassword->getText()) == "" || StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::ConfirmPassword->getText()) == "")
			{
				empty_field = true;
			}

			if (StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::OldPassword->getText()) == OgreClient::Singleton->Config->SelectedConnectionInfo->Password)
			{
				old_password_match = true;
			}

			if (StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::NewPassword->getText()) == StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::ConfirmPassword->getText()))
			{
				new_password_match = true;
			}

			if (empty_field)
			{
				ControllerUI::ConfirmPopup::ShowOK("You must fill in all fields.", 0);

				return true;
			}

			if (!old_password_match)
			{
				ControllerUI::ConfirmPopup::ShowOK("Your current password is incorrect.", 0);

				return true;
			}

			if (!new_password_match)
			{
				ControllerUI::ConfirmPopup::ShowOK("Your new password and confirm password do not match.", 0);

				return true;
			}

			OgreClient::Singleton->SendReqChangePassword(StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::OldPassword->getText()), StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::NewPassword->getText()));
			
			// Update client with their new password
			OgreClient::Singleton->Config->SelectedConnectionInfo->Password = StringConvert::CEGUIToCLR(ControllerUI::ChangePassword::NewPassword->getText());

			// Let's close the window.
			ControllerUI::ToggleVisibility(ControllerUI::ChangePassword::Window);

			return true;
		};

	};
};
