#include "Engine/System/BlueJadeApp.h"
#include <iostream>
#include <string>

using namespace std;

BlueJadeApp* g_pGameApp = nullptr;

int main(void) {

	g_pGameApp = new BlueJadeApp();

	if (!g_pGameApp->InitInstance()) {
		return 1;
	}

	g_pGameApp->MainLoop();

	g_pGameApp->CloseApp();

	return 0;
}