/*
 * user_group.c
 *
 *  Created on: Apr 7, 2016
 *      Author: root
 */

#include "user_group.h"
#include <stdio.h>
#include <pwd.h>
#include <grp.h>

#include <unistd.h>
#include <crypt.h> // crypt() and crypt() requires -lcrypt library

void user_group()
{
	struct passwd *pwd;
	pwd = getpwnam("ahnmh");

	printf("name=%s/", pwd->pw_name);
	printf("uid=%d/", pwd->pw_uid);
	printf("gid=%d/", pwd->pw_gid);
	printf("shell=%s/", pwd->pw_shell);
	printf("passwd=%s\n", pwd->pw_passwd);

	pwd = getpwuid(0);

	printf("name=%s/", pwd->pw_name);
	printf("uid=%d/", pwd->pw_uid);
	printf("gid=%d/", pwd->pw_gid);
	printf("shell=%s/", pwd->pw_shell);
	printf("passwd=%s\n", pwd->pw_passwd);

	// get all users from passwd
	while((pwd = getpwent()) != NULL) {
		printf("name=%s/", pwd->pw_name);
		printf("uid=%d/", pwd->pw_uid);
		printf("gid=%d/", pwd->pw_gid);
		printf("shell=%s/", pwd->pw_shell);
		printf("passwd=%s\n", pwd->pw_passwd);
	}
	endpwent();

/*
	if you want informations that included in /etc/shadow, use below function set
	getspnam, getspent, setspent, endspent
*/

/*	 crypt will return an encrypted password with 13 characters.*/
	char *encoded = crypt("1qaz2wsx", "mh"); // text passphrase, salt
	printf("encoded password=%s\n", encoded);


}
