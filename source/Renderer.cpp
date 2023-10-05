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
				
					if (pScene->DoesHit(ray))
					{
						continue;
					}

					// Lamber cosine law
					auto lcl = Vector3::Dot(closestHit.normal, lightRayDirection);

					if (lcl >= 0)
					{
						finalColor += LightUtils::GetRadiance(light, closestHit.origin)
							* materials[closestHit.materialIndex]->Shade(closestHit, lightRayDirection, rayDirection)
							* lcl;
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
