#ifndef PASSWORDUNLOCKER_H_
#define PASSWORDUNLOCKER_H_

#include <QString>

class QWidget;

/**
 * Manages password entry for a given action
 *
 * This class manages the permission to perform an operation. The user has to
 * enter a password in order to get the permission. After the correct password
 * has been entered, the user is not asked for the password again.
 *
 * This class can also be configured to unconditionally permit the operation
 * without requiring a password.
 */
class PasswordPermission
{
	public:
		PasswordPermission (QWidget *parent);
		PasswordPermission (bool passwordRequired, const QString &password, QWidget *parent);
		virtual ~PasswordPermission ();

		void requirePassword (const QString &password);

		bool permit (const QString &message);

	private:
		bool passwordRequired;
		QString password;
		bool passwordOk;

		QWidget *parent;
};

#endif
