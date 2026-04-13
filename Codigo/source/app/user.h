#ifndef _USER_H_
#define _USER_H_

#include <stdbool.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
//                Definitions

#define MAX_USERS 10


// User Information Data Type
typedef struct USER
{
	char id[8];
	char password[5];
	bool adminPermit;
	bool fullPass;
} user_t;

extern user_t userDataset[];

// Starts initial userDataset

#ifdef ADMIN
bool initUserSystem(void);
// Adds a user -> return null if successful
bool newUser(uint32_t id, uint32_t password);

// Remove a user -> returns null if successful
bool removeUser(uint32_t id);
#endif // ADMIN
// Chance password of active user -> returns null if successful
bool changePass(user_t * activeUser, char* newPass);

//searchs for an id, returns the index of the id, or -1 if it doesnt find it
bool searchId(char *targetId, int8_t *idx);

//checks if the password is correct for a given user, receiving the index
bool checkPass(int8_t *idx, char pass[5], bool checkFull);


#endif // _USER_H_

