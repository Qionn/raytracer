//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"


namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
		, m_pBuffer(SDL_GetWindowSurface(pWindow))
	{
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
		m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	}

	void Renderer::Render(Scene* pScene) const
	{
		int pixelCount = m_Width * m_Height;

		for (int i = 0; i < pixelCount; ++i)
		{
			RenderPixel(pScene, i);
		}

		SDL_UpdateWindowSurface(m_pWindow);
	}

	bool Renderer::SaveBufferToImage() const
	{
		return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
	}

	void Renderer::ToggleShadows()
	{
		m_ShadowsEnabled = !m_ShadowsEnabled;
	}

	void Renderer::CycleLightingMode()
	{
		const int modeCount = static_cast<int>(LightingMode::Count);
		m_LightingMode = static_cast<LightingMode>((static_cast<int>(m_LightingMode) + 1) % modeCount);
	}

	void Renderer::RenderPixel(Scene* pScene, int pixelIndex) const
	{
		Camera& camera = pScene->GetCamera();
		auto& materials = pScene->GetMaterials();
		auto& lights = pScene->GetLights();

		int px = FastMod(pixelIndex, m_Width);
		int py = pixelIndex / m_Width;

		float ndcX = (2.0f * (px + 0.5f) / m_Width - 1.0f);
		float ndcY = 1.0f - 2.0f * (py + 0.5f) / m_Height;

		Vector3 rayDirection = {
			ndcX * camera.fov * m_AspectRatio,
			ndcY * camera.fov,
			1.0f
		};

		rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
		rayDirection.Normalize();

		Ray ray{ camera.origin, rayDirection };

		HitRecord closestHit{};
		pScene->GetClosestHit(ray, closestHit);

		rayDirection = -rayDirection;

		Material* pMaterial = materials[closestHit.materialIndex];
		ColorRGB finalColor = colors::Black;

		if (closestHit.didHit)
		{
			for (auto& light : lights)
			{
				Vector3 lightRayDirection = LightUtils::GetDirectionToLight(light, closestHit.origin);

				ray.max = lightRayDirection.Normalize();
				ray.origin = closestHit.origin + closestHit.normal * 0.0001f;
				ray.direction = lightRayDirection;

				if (m_ShadowsEnabled && pScene->DoesHit(ray))
				{
					continue;
				}

				float observedArea = Vector3::Dot(closestHit.normal, lightRayDirection);

				switch (m_LightingMode)
				{
					case LightingMode::ObservedArea:
						if (observedArea > 0.0f)
						{
							finalColor += ColorRGB{ observedArea };
						}
						break;

					case LightingMode::Radiance:
						finalColor += LightUtils::GetRadiance(light, closestHit.origin);
						break;

					case LightingMode::BRDF:
						finalColor += pMaterial->Shade(closestHit, lightRayDirection, rayDirection);
						break;

					case LightingMode::Combined:
						if (observedArea > 0.0f)
						{
							finalColor += (observedArea * LightUtils::GetRadiance(light, closestHit.origin) *
										   pMaterial->Shade(closestHit, lightRayDirection, rayDirection));
						}
						break;
				}
			}
		}

		//Update Color in Buffer
		finalColor.MaxToOne();

		m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
														  static_cast<uint8_t>(finalColor.r * 255),
														  static_cast<uint8_t>(finalColor.g * 255),
														  static_cast<uint8_t>(finalColor.b * 255));
	}
}
