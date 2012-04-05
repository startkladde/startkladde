#include "PasswordPermission.h"

/**
 * Creates a PasswordPermission, using the given PasswordCheck
 */
PasswordPermission::PasswordPermission (PasswordCheck &passwordCheck):
	passwordCheck (passwordCheck)
{

}

PasswordPermission::~PasswordPermission ()
{
}

/**
 * Sets whether the password is required
 *
 * @see permit
 */
void PasswordPermission::setPasswordRequired (bool required)
{
	passwordRequired=required;
}

bool PasswordPermission::getPasswordRequired ()
{
	return passwordRequired;
}

/**
 * Determines whether the operation is permitted
 *
 * If the password is not required (see setPasswordRequired), the operation is
 * always permitted. If the password is required, the operation is permitted if
 * the correct password is entered, has been entered before, as determined by
 * the associated PasswordCheck.
 *
 * @param message the message to display to the user in the password dialog
 * @return true if the operation is permitted, false otherwise
 */
bool PasswordPermission::permit (const QString &message)
{
	if (!passwordRequired)
		return true;

	return passwordCheck.query (message);
}
