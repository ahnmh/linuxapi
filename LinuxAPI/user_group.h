/*
 * user_group.h
 *
 *  Created on: Apr 7, 2016
 *      Author: root
 */

#ifndef USER_GROUP_H_
#define USER_GROUP_H_

#include <pwd.h>
#include <grp.h>

#include <unistd.h>
#include <crypt.h> // crypt() and crypt() requires -lcrypt library


void user_group();

#endif /* USER_GROUP_H_ */
