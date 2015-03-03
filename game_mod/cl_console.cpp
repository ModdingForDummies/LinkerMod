#include "stdafx.h"

bool Con_HasActiveAutoComplete()
{
	//return conDrawInputGlob.matchIndex >= 0 && conDrawInputGlob.autoCompleteChoice[0];
	return (*(int *)0xC48A48 >= 0 && ((BYTE *)0xC48A08)[0]);
}

bool Con_CancelAutoComplete()
{
	if (Con_HasActiveAutoComplete())
	{
		*(int *)0xC48A48 = -1;
		((BYTE *)0xC48A08)[0] = 0;

		return true;
	}

	return false;
}

void Con_ToggleConsole()
{
	Field_Clear(g_consoleField);
	Con_CancelAutoComplete();

	g_consoleField->widthInPixels = 620;
	g_consoleField->charHeight = 16.0f;
	g_consoleField->fixedSize = true;

	*(bool *)0xC6924C = false;
	*(DWORD *)0x2910160 ^= 1;
}