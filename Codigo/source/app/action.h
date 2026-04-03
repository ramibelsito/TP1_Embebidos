
#ifndef _ACTION_H_
#define _ACTION_H_




// FSM States

// Check id and returns true if a match is found
bool idCheck(void);

// Reads active user and checks password match. Returns true if successful
bool passCheck(void);

// Returns true and opens door for x seconds if user doesn't want to configure.
bool openCheck(void);

// Returns true if configuration ended
bool configCheck(void);

// NOT FSM States

// Returns true if admin session ended
bool adminCheck(void);

// Starts initial userDataset
bool initSystem(void);



#endif // _ACTION_H_
