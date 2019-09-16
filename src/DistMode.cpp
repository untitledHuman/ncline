#include <cassert>
#include "DistMode.h"
#include "CMakeCommand.h"
#include "Settings.h"
#include "Configuration.h"
#include "Helpers.h"

namespace {

bool devDistEngineArg(std::string &cmakeArguments)
{
	cmakeArguments += " -D NCINE_OPTIONS_PRESETS=DevDist";
	return true;
}

bool devDistGameArg(std::string &cmakeArguments)
{
	cmakeArguments += " -D PACKAGE_OPTIONS_PRESETS=BinDist";
	return true;
}

bool ncineDirArg(std::string &cmakeArguments)
{
	std::string arguments;
	const bool argumentsAdded = config().engineDir(arguments);
	if (argumentsAdded)
		cmakeArguments += " -D nCine_DIR=" + arguments;

	return argumentsAdded;
}

void buildReleaseAndPackage(CMakeCommand &cmake, const char *buildDir)
{
	assert(buildDir);

	if (CMakeCommand::generatorIsMultiConfig())
	{
		cmake.buildConfig(buildDir, "release");
		cmake.build(buildDir, "release", "package");
	}
	else
	{
		cmake.build(buildDir);
		cmake.buildTarget(buildDir, "package");
	}
}

void distributeEngine(CMakeCommand &cmake, const Settings &settings)
{
	cmake.addAndroidNdkDirToPath();
	cmake.addDoxygenDirToPath();

	Helpers::info("Distribute the engine");

	std::string buildDir = Helpers::nCineSourceDir();
	Helpers::distDir(buildDir, settings);

	std::string arguments;
	devDistEngineArg(arguments);

	cmake.configure(Helpers::nCineSourceDir(), buildDir.data(), arguments.empty() ? nullptr : arguments.data());
	buildReleaseAndPackage(cmake, buildDir.data());
}

void distributeGame(CMakeCommand &cmake, const Settings &settings, const std::string &gameName)
{
	Helpers::info("Distribute the game: ", gameName.data());

	std::string buildDir = gameName;
	Helpers::distDir(buildDir, settings);

	std::string arguments;
	devDistGameArg(arguments);
	ncineDirArg(arguments);

	cmake.configure(gameName.data(), buildDir.data(), arguments.empty() ? nullptr : arguments.data());
	buildReleaseAndPackage(cmake, buildDir.data());
}

}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void DistMode::perform(CMakeCommand &cmake, const Settings &settings)
{
	assert(settings.mode() == Settings::Mode::DIST);
	assert(settings.target() != Settings::Target::LIBS);

	switch (settings.target())
	{
		case Settings::Target::LIBS:
			break;
		case Settings::Target::ENGINE:
			distributeEngine(cmake, settings);
			break;
		case Settings::Target::GAME:
		{
			std::string gameName;
			config().gameName(gameName);

			distributeGame(cmake, settings, gameName);
			break;
		}
	}
}