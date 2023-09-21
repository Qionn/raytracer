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

	Vector3 rayDirection{ 0.0f, 0.0f, 1.0f };
	const auto aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	Sphere testSphere{ {0.0f, 0.0f, 100.0f}, 50.0f, 0 };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 rayDirection = {
				(2.0f * (px + 0.5f) / m_Width - 1.0f) * aspectRatio,
				1.0f - 2.0f * (py + 0.5f) / m_Height,
				1.0f
			};
			rayDirection.Normalize();

			Ray viewRay{ Vector3::Zero, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				//finalColor = materials[closestHit.materialIndex]->Shade();
				float scalted_t = (closestHit.t - 50.0f) / 40.0f;
				finalColor = { scalted_t, scalted_t, scalted_t };
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
