#ifndef PASSWORDPERMISSION_H_
#define PASSWORDPERMISSION_H_

#include "src/gui/PasswordCheck.h"

/**
 * Uses a PasswordCheck to query a password from the user if requried
 *
 * This is separate from PasswordCheck so several actions (for which the need
 * for a password is configured individually) can share the same password, and
 * the password only has to be entered once.
 */
class PasswordPermission
{
	public:
		PasswordPermission (PasswordCheck &passwordCheck);
		virtual ~PasswordPermission ();

		void setPasswordRequired (bool required);
		bool getPasswordRequired ();
		bool permit (const QString &message);

	private:
		PasswordCheck &passwordCheck;
		bool passwordRequired;
};

#endif
