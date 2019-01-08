#include "common.hpp"
#include "random.hpp"
#include "asset.hpp"
#include "rtmidi.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "bridge.hpp"
#include "settings.hpp"
#include "engine/Engine.hpp"
#include "app/Scene.hpp"
#include "plugin.hpp"
#include "app.hpp"
#include "ui.hpp"

#include <unistd.h>
#include <osdialog.h>

#ifdef ARCH_WIN
	#include <Windows.h>
#endif

using namespace rack;


int main(int argc, char *argv[]) {
	#ifdef ARCH_WIN
		// Windows global mutex to prevent multiple instances
		// Handle will be closed by Windows when the process ends
		HANDLE instanceMutex = CreateMutex(NULL, true, APP_NAME.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Rack is already running. Multiple Rack instances are not supported.");
			exit(1);
		}
		(void) instanceMutex;
	#endif

	bool devMode = false;
	std::string patchFile;

	// Parse command line arguments
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "ds:u:")) != -1) {
		switch (c) {
			case 'd': {
				devMode = true;
			} break;
			case 's': {
				asset::systemDir = optarg;
			} break;
			case 'u': {
				asset::userDir = optarg;
			} break;
			default: break;
		}
	}
	if (optind < argc) {
		patchFile = argv[optind];
	}

	// Initialize environment
	random::init();
	asset::init(devMode);
	logger::init(devMode);

	// Log environment
	INFO("%s %s", APP_NAME.c_str(), APP_VERSION.c_str());
	if (devMode)
		INFO("Development mode");
	INFO("System directory: %s", asset::systemDir.c_str());
	INFO("User directory: %s", asset::userDir.c_str());

	midi::init();
	rtmidiInit();
	bridgeInit();
	keyboard::init();
	gamepad::init();
	ui::init();
	plugin::init(devMode);

	// Initialize app
	appInit();
	app()->scene->devMode = devMode;
	settings::load(asset::user("settings.json"));

	if (patchFile.empty()) {
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipLoadOnLaunch = settings::skipLoadOnLaunch;
		settings::skipLoadOnLaunch = true;
		settings::save(asset::user("settings.json"));
		settings::skipLoadOnLaunch = false;
		if (oldSkipLoadOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Clear your patch and start over?")) {
			app()->scene->rackWidget->lastPath = "";
		}
		else {
			// Load autosave
			std::string oldLastPath = app()->scene->rackWidget->lastPath;
			app()->scene->rackWidget->load(asset::user("autosave.vcv"));
			app()->scene->rackWidget->lastPath = oldLastPath;
		}
	}
	else {
		// Load patch
		app()->scene->rackWidget->load(patchFile);
		app()->scene->rackWidget->lastPath = patchFile;
	}

	app()->engine->start();
	app()->window->run();
	app()->engine->stop();

	// Destroy app
	app()->scene->rackWidget->save(asset::user("autosave.vcv"));
	settings::save(asset::user("settings.json"));
	appDestroy();

	// Destroy environment
	plugin::destroy();
	ui::destroy();
	bridgeDestroy();
	midi::destroy();
	logger::destroy();

	return 0;
}
