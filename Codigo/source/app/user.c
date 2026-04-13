#include "app/user.h"
#include <stdbool.h>
typedef struct user_t
{
	char id[8];
	char password[5];
    bool fullpass; //true if 5 characters, false if 4.
	bool adminPermit;
} user_t;


user_t userDataset[MAX_USERS] = 0;


// Starts initial userDataset
bool initUserSystem() {
    userDataset[0].id = 12341234;
    userDataset[0].password = 1234;
    userDataset[1].id = 60612683;
    userDataset[1].password = 80085;
    userDataset[2].id = 77777777;
    userDataset[2].password =42069;
    return

}
#ifdef ADMIN
bool newUser(uint32_t id, uint32_t password) {
}

bool removeUser(uint32_t id) {
}
#endif // ADMIN
bool changePass(user_t* activeUser, char pass[5])
{

    int8_t maxchars = user->fullpass ? 5 : 4;

    for (int j = 0; j < maxchars; j++)
    {
        activeUser->password[j] = pass[j];
    }
    return false
}

bool searchId(char *targetId, int8_t *idx)
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        bool match = true;


        for (int j = 0; j < 8; j++)
        {
            if (userDataset[i].id[j] != targetId[j])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            *idx = i;
            return true;
        }
    }

    *idx = -1;
    return false;
}




bool checkPass(int *idx, char pass[5], bool checkFull)
{

    user_t *user = &userDataset[*idx];


    if (checkFull != user->fullpass)
    {
        return false;
    }


    int8_t maxchars = user->fullpass ? 5 : 4;

    // 3. ComparaciÃ³n manual byte a byte
    for (int j = 0; j < maxchars; j++)
    {
        if (user->password[j] != pass[j])
        {
            return false;
        }
    }


    return true;
}
