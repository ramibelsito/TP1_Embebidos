#include "app/action.h"
#include "hal/wheel.h"
#include "app/user.h"


bool idCheck()
{

}

bool passCheck()
{

}

bool openCheck()
{

}

bool configCheck()
{

}

bool adminCheck()
{

}

bool initSystem()
{
	initUserSystem();
}

// Concatenates number if wipe is null, returns 0 if enter was not pressed
uint32_t inputComposit(bool wipe);




uint32_t inputComposit(bool wipe)
{
	static uint32_t result;
	uint32_t inputType = readWheel();
	if (wipe)
	{
		result = 0;
	}
	switch (inputType)
	{
		case RIGHTTURN:
			break;
		case LEFTTURN:
			break;
		case CLICK:
			break;
		case DOUBLECLICK:
			return result;
			break;
		case CLICKHOLD:
			break;
		default:
			break;
	}
	return 0;
}
