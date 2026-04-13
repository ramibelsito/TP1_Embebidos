#ifndef _USER_H_
#define _USER_H_

#include <stdbool.h>
#include <stdint.h>

// User Information Data Type
typedef struct user_t
{
	char id[8];
	char password[5];
	bool adminPermit;
} user_t;

extern user_t userDataset[];

// Starts initial userDataset
bool initUserSystem(void);

// Adds a user -> return null if successful
bool newUser(uint32_t id, uint32_t password);

// Remove a user -> returns null if successful
bool removeUser(uint32_t id);

// Chance password of active user -> returns null if successful
bool changePass(user_t * activeUser, uint32_t newPass);

#endif // _USER_H_


