#pragma once

#include <cmath>
#include <unordered_set>

struct ImFont;
struct SDL_Renderer;

namespace tudov
{
	class GUI
	{
	  private:
		inline static bool _isOn = false;
		inline static std::float_t _scale = 1;
		inline static std::unordered_set<SDL_Renderer *> _sdlRenderers{};

	  public:
		explicit GUI() noexcept = delete;

		static void TryInit() noexcept;
		static void Quit() noexcept;

		static bool Attach(SDL_Renderer *sdlRenderer) noexcept;
		static bool Detach(SDL_Renderer *sdlRenderer) noexcept;

		static std::float_t GetScale() noexcept;
		static void SetScale(std::float_t scale) noexcept;

		static ImFont &GetSmallFont() noexcept;
		static ImFont &GetMediumFont() noexcept;
		static ImFont &GetLargeFont() noexcept;
	};
} // namespace tudov
