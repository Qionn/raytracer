#pragma once

#include <cstdint>

#include "ColorRGB.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	enum class LightingMode
	{
		ObservedArea,
		Radiance,
		BRDF,
		Combined,

		Count
	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void ToggleShadows();
		void CycleLightingMode();

	private:
		void RenderPixel(Scene* pScene, int pixelIndex) const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};

		bool m_ShadowsEnabled = true;
		LightingMode m_LightingMode = LightingMode::Combined;
	};
}
