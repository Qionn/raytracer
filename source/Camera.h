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

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin}
		{
			ChangeFOV(_fovAngle);
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float tanHalfFov{ 0.0f };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		void ChangeFOV(float fov)
		{
			fovAngle = TO_RADIANS * fov;
			tanHalfFov = std::tan(fovAngle * 0.5f);
		}

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

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			constexpr float walkSpeed = 10.0f;
			constexpr float runSpeed = 35.0f;
			constexpr float rotationSpeed = 1.0f;

			int8_t xDir = pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A];
			int8_t yDir = pKeyboardState[SDL_SCANCODE_E] - pKeyboardState[SDL_SCANCODE_Q];
			int8_t zDir = pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S];

			bool isMoving = xDir != 0 || yDir != 0 || zDir != 0;
			bool isLeftMouseDown = mouseState & SDL_BUTTON(1);
			bool isRotating = isLeftMouseDown && (mouseX != 0.0f || mouseY != 0.0f);
			bool isRunning = pKeyboardState[SDL_SCANCODE_LSHIFT];

			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(isLeftMouseDown));

			if (isMoving)
			{
				Vector3 localForward = cameraToWorld.GetAxisZ();
				Vector3 localRight = cameraToWorld.GetAxisX();

				float speed = isRunning ? runSpeed : walkSpeed;

				origin += xDir * speed * localRight * deltaTime;
				origin += yDir * speed * up * deltaTime;
				origin += zDir * speed * localForward * deltaTime;
			}

			if (isRotating)
			{
				totalPitch += mouseY * rotationSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;

				constexpr float pi = static_cast<float>(M_PI / 2);
				totalPitch = std::clamp(totalPitch, -pi, pi);
			}

			CalculateCameraToWorld();
		}
	};
}
