/*
 * Santiago Balladares
 * Dissertation Project
 * Screen Space Subsurface Scattering (SSSSS)
 * Newcastle University
 * Computer Game Programming MSc
 * 2011 - 2012
 */
#pragma comment(lib, "Framework.lib")

#include "../Framework/window.h"
#include "Renderer.h"

int main()
{
	Window w("Screen Space Subsurface Scattering", 1900, 1024, false);
	//Window w("Screen Space Subsurface Scattering", 1920, 1080, true);

	if (!w.HasInitialised())
	{
		return -1;
	}
	
	srand((unsigned int)w.GetTimer()->GetMS() * 1000.0f);

	Renderer renderer(w);
	if (!renderer.HasInitialised())
	{
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(true);

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE))
	{
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();
	}

	return 0;
}