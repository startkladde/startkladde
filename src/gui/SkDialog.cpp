#include "PasswordPermission.h"

#include "src/gui/dialogs.h"

PasswordPermission::PasswordPermission (QWidget *parent):
	passwordRequired (false),
	passwordOk (false),
	parent (parent)
{
}

PasswordPermission::PasswordPermission (bool passwordRequired, const QString &password, QWidget *parent):
	passwordRequired (passwordRequired), password (password),
	passwordOk (false),
	parent (parent)
{
}

PasswordPermission::~PasswordPermission ()
{
}

void PasswordPermission::requirePassword (const QString &password)
{
	this->passwordRequired=true;
	this->password=password;
}

bool PasswordPermission::permit (const QString &message)
{
	// If no password is required at all, the operation is permitted
	if (!passwordRequired) return true;

	// If the correct password has been entered before, the operation is
	// permitted
	if (passwordOk) return true;

	// We need to ask the user for the password
	if (verifyPassword (parent, password, message))
	{
		// The entered password was correct. Set passwordOk, so subsequent
		// invocations of this method will not ask for the password again.
		passwordOk=true;

		// The operation is permitted
		return true;
	}
	else
	{
		// The user canceled password entry (askForPassword keeps asking if the
		// user enters a wrong password). The operation is not permitted.
		return false;
	}
}
