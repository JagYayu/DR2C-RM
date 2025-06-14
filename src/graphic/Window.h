#pragma once

#include "IRenderer.h"

#include "SDL3/SDL_render.h"
#include "debug/DebugManager.h"
#include <SDL3/SDL.h>

#include <memory>

class SDL_Window;

namespace tudov
{
	class Engine;
	class SDLRenderer;
	class Texture;

	class Window
	{
		friend SDLRenderer;

	  private:
		SDL_Window *_window;
		UInt32 _prevTick;
		UInt32 _frame;
		float _framerate;
		float framerateTimer;

	  public:
		Engine &engine;
		DebugManager debugManager;
		std::unique_ptr<IRenderer> renderer;

		Window(Engine &engine) ;
		~Window() noexcept;

		SDL_Window *GetHandle();

		float GetFramerate() const noexcept;

		void Initialize();
		void Deinitialize() noexcept;
		void PoolEvents();
		void Render();
	};
} // namespace tudov
