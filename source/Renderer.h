#pragma once

#include <cstdint>

#include "ColorRGB.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	class Material;
	struct Vector3;
	struct HitRecord;
	struct Light;

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
		ColorRGB LightingObservedArea(const HitRecord& hitRecord, const Vector3& l) const;
		ColorRGB LightingRadiance(const HitRecord& hitRecord, const Light& light) const;
		ColorRGB LightingBRDF(Material* pMaterial, const HitRecord& hitRecord, const Vector3& l, const Vector3& v) const;
		ColorRGB LightingCombined(Material* pMaterial, const HitRecord& hitRecord, const Light& light, const Vector3& l, const Vector3& v) const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		LightingMode m_LightingMode = LightingMode::Combined;

		bool m_ShadowsEnabled = false;
	};
}
