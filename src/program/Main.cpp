#include "graphic/GUI.hpp"
#include "program/Engine.hpp"
#include "program/MainArgs.hpp"
#include "util/Log.hpp"

#include "SDL3/SDL_gpu.h"

#include <memory>

#define SDL_MAIN_USE_CALLBACKS

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_messagebox.h"

using namespace tudov;

void RedirectLibraryPath(std::string_view path) noexcept
{
	// do nothing
}

void SDLLogOutputCallback(void *userdata, int category, SDL_LogPriority priority, const char *message) noexcept
{
	const char *cate = "";
	switch (category)
	{
	case SDL_LOG_CATEGORY_APPLICATION:
		cate = "Application";
		break;
	case SDL_LOG_CATEGORY_ERROR:
		cate = "Error";
		break;
	case SDL_LOG_CATEGORY_ASSERT:
		cate = "Assert";
		break;
	case SDL_LOG_CATEGORY_SYSTEM:
		cate = "System";
		break;
	case SDL_LOG_CATEGORY_AUDIO:
		cate = "Audio";
		break;
	case SDL_LOG_CATEGORY_VIDEO:
		cate = "Video";
		break;
	case SDL_LOG_CATEGORY_RENDER:
		cate = "Render";
		break;
	case SDL_LOG_CATEGORY_INPUT:
		cate = "Input";
		break;
	case SDL_LOG_CATEGORY_TEST:
		cate = "Test";
		break;
	case SDL_LOG_CATEGORY_GPU:
		cate = "Gpu";
		break;
	}

	auto &&log = Log::Get(std::string("@SDL.") + cate);
	switch (priority)
	{
	case SDL_LOG_PRIORITY_TRACE:
	case SDL_LOG_PRIORITY_VERBOSE:
		log->Trace("{}", message);
	case SDL_LOG_PRIORITY_DEBUG:
		log->Debug("{}", message);
		break;
	case SDL_LOG_PRIORITY_INFO:
		log->Info("{}", message);
		break;
	case SDL_LOG_PRIORITY_WARN:
		log->Warn("{}", message);
		break;
	case SDL_LOG_PRIORITY_ERROR:
		log->Error("{}", message);
	case SDL_LOG_PRIORITY_CRITICAL:
		log->Fatal("{}", message);
		break;
	default:
		break;
	}
}

static std::unique_ptr<Application> app;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	SDL_SetLogOutputFunction(SDLLogOutputCallback, nullptr);
	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_SENSOR | SDL_INIT_CAMERA))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		return SDL_APP_FAILURE;
	}

	for (auto index = 0; index < SDL_GetNumGPUDrivers(); ++index)
	{
		Log::Get("Main")->Info("GPU Driver {}:", index, SDL_GetGPUDriver(index));
	}

	app = std::make_unique<Engine>(MainArgs(argc, argv));
	app->Initialize();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	return app->Tick() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	app->Event(*event);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	if (result != SDL_APP_FAILURE)
	{
		app->Deinitialize();
		app = nullptr;
	}

	GUI::Quit();

	Log::GetInstance().Info("Application quit, result code: {}", std::int32_t(result));
	Log::Quit();
}

// int main(int argc, char **args)
// {
// 	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_SENSOR | SDL_INIT_CAMERA))
// 	{
// 		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
// 	}

// 	IMGUI_CHECKVERSION();
// 	ImGui::CreateContext();
// 	ImGui::StyleColorsDark();
// 	{
// 		ImGuiIO &io = ImGui::GetIO();
// 		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
// 		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
// 	}

// 	Engine(MainArgs{argc, args}).Run();

// 	ImGui::DestroyContext();

// 	SDL_Quit();

// 	Log::Exit();

// 	return 0;
// }
