#pragma once
#include <algorithm>
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle)
			: origin{ _origin }
			, fovAngle{ _fovAngle }
		{

		}

		Vector3 origin;

		float totalPitch	= 0.0f;
		float totalYaw		= 0.0f;

		float fovAngle;
		float prevFovAngle	= 0.0f;
		float fov			= 0.0f;

		float rotationSpeed	= 1.0f;
		float walkSpeed		= 10.0f;
		float dragSpeed		= 5.0f;

		Matrix cameraToWorld;

		Matrix CalculateCameraToWorld()
		{
			auto translation = Matrix::CreateTranslation(origin);
			auto rotation = Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);

			cameraToWorld = rotation * translation;
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			if (fovAngle != prevFovAngle)
			{
				prevFovAngle = fovAngle;
				fov = std::tan(TO_RADIANS * fovAngle * 0.5f);
			}

			HandleKeyboardInput(deltaTime);
			HandleMouseInput(deltaTime);

			CalculateCameraToWorld();
		}

		void HandleKeyboardInput(float dt)
		{
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const int8_t xDir = (pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A] +
								 pKeyboardState[SDL_SCANCODE_RIGHT] - pKeyboardState[SDL_SCANCODE_LEFT]);

			const int8_t zDir = (pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S] +
								 pKeyboardState[SDL_SCANCODE_UP] - pKeyboardState[SDL_SCANCODE_DOWN]);

			origin += zDir * walkSpeed * cameraToWorld.GetAxisZ() * dt;
			origin += xDir * walkSpeed * cameraToWorld.GetAxisX() * dt;
		}

		void HandleMouseInput(float dt)
		{
			int mouseX, mouseY;
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const bool isLeftMouseDown	= static_cast<bool>(mouseState & SDL_BUTTON(1));
			const bool isRightMouseDown	= static_cast<bool>(mouseState & SDL_BUTTON(3));
			const bool areBothMouseDown	= isLeftMouseDown && isRightMouseDown;

			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(isLeftMouseDown || isRightMouseDown));

			if (areBothMouseDown)
			{
				origin -= mouseY * dragSpeed * Vector3::UnitY * dt;
			}
			else if (isRightMouseDown)
			{
				totalYaw += mouseX * rotationSpeed * dt;
				totalPitch += mouseY * rotationSpeed * dt;
			}
			else if (isLeftMouseDown)
			{
				origin -= mouseY * dragSpeed * cameraToWorld.GetAxisZ() * dt;
				totalYaw += mouseX * rotationSpeed * dt;
			}
		}
	};
}
