#include "app/user.h"
#include <stdbool.h>

user_t userDataset[] = {
    {.id = "12341234", .password = "1234", .adminPermit = false, .fullPass = false},
    {.id = "00000000", .password = "00000", .adminPermit = false, .fullPass = true},
    {.id = "12345678", .password = "42069", .adminPermit = false, .fullPass = true},
    {.id = "60612683", .password = "80085", .adminPermit = false, .fullPass = true},
};

#ifdef ADMIN
// Starts initial userDataset
bool initUserSystem() {

  return 0;
}

bool newUser(uint32_t id, uint32_t password) {
}

bool removeUser(uint32_t id) {
}
#endif // ADMIN
void changePass(user_t* activeUser, char pass[5]) {

  int8_t maxchars = activeUser->fullPass ? 5 : 4;

  for (int j = 0; j < maxchars; j++) {
    activeUser->password[j] = pass[j];
  }
}

bool searchId(char* targetId, int8_t* idx) {
  for (int i = 0; i < MAX_USERS; i++) {
    bool match = true;

    for (int j = 0; j < 8; j++) {
      if (userDataset[i].id[j] != targetId[j]) {
        match = false;
        break;
      }
    }

    if (match) {
      *idx = i;
      return true;
    }
  }

  *idx = -1;
  return false;
}

bool checkPass(int8_t* idx, char pass[5], bool checkFull) {

  user_t* user = &userDataset[*idx];

  if (checkFull != user->fullPass) {
    return false;
  }

  int8_t maxchars = user->fullPass ? 5 : 4;

  // 3. ComparaciÃ³n manual byte a byte
  for (int j = 0; j < maxchars; j++) {
    if (user->password[j] != pass[j]) {
      return false;
    }
  }

  return true;
}
