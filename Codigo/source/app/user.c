#include "app/user.h"
#include <stdbool.h>

user_t userDataset[] = {
    {.id = "12341234", .password = "1234", .adminPermit = false},
    {.id = "00000000", .password = "00000", .adminPermit = false},
    {.id = "12345678", .password = "42069", .adminPermit = false},
};

// Starts initial userDataset
bool initUserSystem() {
}

bool newUser(uint32_t id, uint32_t password) {
}

bool removeUser(uint32_t id) {
}

bool changePass(user_t* activeUser, uint32_t newPass) {
}
