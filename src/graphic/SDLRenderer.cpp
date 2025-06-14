#include "SDLRenderer.h"

#include "SDLTexture.h"
#include "Window.h"
#include "program/Engine.h"

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>

using namespace tudov;

SDLRenderer::SDLRenderer(Window &window)
    : IRenderer(window),
      _log(Log::Get("SDLRenderer"))
{
}

SDL_Renderer *SDLRenderer::GetRaw() noexcept
{
	return _renderer;
}

const SDL_Renderer *SDLRenderer::GetRaw() const noexcept
{
	return _renderer;
}

void SDLRenderer::Initialize() noexcept
{
	_renderer = SDL_CreateRenderer(window._window, nullptr);
	if (!_renderer)
	{
		SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
	}

	ImGui_ImplSDL3_InitForSDLRenderer(window._window, _renderer);
	ImGui_ImplSDLRenderer3_Init(_renderer);
}

void SDLRenderer::RegisterGlobalsTo(std::string_view name, ScriptEngine &scriptEngine) noexcept
{
	auto &&render = scriptEngine.CreateTable();

	render["draw"] = [&](ResourceID texID, float x, float y, float w, float h, float tx, float ty, float tw, float th, float ang, float cx, float cy, uint32_t flip)
	{
		try
		{
			Draw(texID, x, y, w, h, tx, ty, tw, th, ang, cx, cy, flip);
		}
		catch (std::exception &e)
		{
			scriptEngine.ThrowError(std::format("C++ exception: {}", e.what()));
		}
	};

	scriptEngine.SetReadonlyGlobal(name, render);
}

void SDLRenderer::Draw(ResourceID texID, float x, float y, float w, float h, float tx, float ty, float tw, float th, float ang, float cx, float cy, uint32_t flip)
{
	auto &&texRes = window.engine.textureManager.GetResource(texID);
	if (!texRes)
	{
		auto &&msg = std::format("Texture not found: <{}>", texID);
		throw std::exception(msg.c_str());
	}

	auto &&tex = std::dynamic_pointer_cast<SDLTexture>(texRes);
	if (!tex)
	{
		auto &&msg = std::format("Invalid texture type");
		throw std::exception(msg.c_str());
	}

	_commandQueue.push_back(Command{
	    .tex = tex->GetRaw(),
	    .dst = SDL_FRect{(float)x, (float)y, (float)w, (float)h},
	    .src = SDL_FRect{(float)x, (float)y, (float)w, (float)h},
	    .ang = ang,
	    .ctr = SDL_FPoint{cx, cy},
	    .flip = (SDL_FlipMode)flip,
	});
}

void SDLRenderer::Begin()
{
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);
}

void SDLRenderer::End()
{
	auto &&io = ImGui::GetIO();

	SDL_SetRenderScale(_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColorFloat(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	std::sort(_commandQueue.begin(), _commandQueue.end(), [](const auto &lhs, const auto &rhs)
	{
		return lhs.tex < rhs.tex;
	});

	SDL_Texture *currentTexture = nullptr;

	for (const auto &cmd : _commandQueue)
	{
		SDL_RenderTextureRotated(_renderer, cmd.tex, &cmd.src, &cmd.dst, cmd.ang, &cmd.ctr, cmd.flip);
	}

	_commandQueue.clear();

	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), _renderer);
	SDL_RenderPresent(_renderer);
}
