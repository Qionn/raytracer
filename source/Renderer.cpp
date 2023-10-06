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

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const auto aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float ndcX = (2.0f * (px + 0.5f) / m_Width - 1.0f);
			float ndcY = 1.0f - 2.0f * (py + 0.5f) / m_Height;

			Vector3 rayDirection = {
				ndcX * camera.tanHalfFov * aspectRatio,
				ndcY * camera.tanHalfFov,
				1.0f
			};

			rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			Ray ray{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(ray, closestHit);

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

					switch (m_LightingMode)
					{
						case LightingMode::ObservedArea:
							finalColor += LightingObservedArea(closestHit, lightRayDirection);
							break;

						case LightingMode::Radiance:
							finalColor += LightingRadiance(closestHit, light);
							break;

						case LightingMode::BRDF:
							finalColor += LightingBRDF(materials[closestHit.materialIndex], closestHit, lightRayDirection, rayDirection);
							break;

						case LightingMode::Combined:
							finalColor += LightingCombined(materials[closestHit.materialIndex], closestHit, light, lightRayDirection, rayDirection);
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

	//@END
	//Update SDL Surface
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
	
	int value = static_cast<int>(m_LightingMode);
	value = (value + 1) % modeCount;

	m_LightingMode = static_cast<LightingMode>(value);
}

ColorRGB Renderer::LightingObservedArea(const HitRecord& closestHit, const Vector3& l) const
{
	auto lcl = Vector3::Dot(closestHit.normal, l);
	return (lcl >= 0.0f) ? ColorRGB{ lcl, lcl, lcl } : colors::Black;
}

ColorRGB Renderer::LightingRadiance(const HitRecord& closestHit, const Light& light) const
{
	return LightUtils::GetRadiance(light, closestHit.origin);
}

ColorRGB Renderer::LightingBRDF(Material* pMaterial, const HitRecord& closestHit, const Vector3& l, const Vector3& v) const
{
	return pMaterial->Shade(closestHit, l, v);
}

ColorRGB Renderer::LightingCombined(Material* pMaterial, const HitRecord& closestHit, const Light& light, const Vector3& l, const Vector3& v) const
{
	auto lcl = Vector3::Dot(closestHit.normal, l);
	if (lcl < 0.0f)
	{
		return colors::Black;
	}

	return LightUtils::GetRadiance(light, closestHit.origin) * pMaterial->Shade(closestHit, l, v) * lcl;
}
