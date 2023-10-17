#pragma once
#include <algorithm>
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const Vector3 sphereToRay = ray.origin - sphere.origin;

			float a = Vector3::Dot(ray.direction, ray.direction);
			float b = 2.0f * Vector3::Dot(ray.direction, sphereToRay);
			float c = Vector3::Dot(sphereToRay, sphereToRay) - (sphere.radius * sphere.radius);

			float discriminant = (b * b) - 4.0f * a * c;

			if (discriminant <= 0.0f)
			{
				return false;
			}

			float sqrtDiscriminant = std::sqrtf(discriminant);
			float inv2a = 1.0f / (2.0f * a);

			float t = (-b - sqrtDiscriminant) * inv2a;
			if (t < ray.min || t > ray.max)
			{
				t = (-b + sqrtDiscriminant) * inv2a;
				if (t < ray.min || t > ray.max)
				{
					return false;
				}
			}
			
			if (!ignoreHitRecord && t < hitRecord.t)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
				hitRecord.t = t;
			}

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			static HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			if (!ignoreHitRecord && t < hitRecord.t)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = plane.normal;
				hitRecord.t = t;
			}

			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			static HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float nvDot = Vector3::Dot(ray.direction, triangle.normal);
			if (AreEqual(nvDot, 0.0f))
			{
				return false;
			}

			TriangleCullMode cullMode = triangle.cullMode;

			if (ignoreHitRecord)
			{
				switch (cullMode)
				{
					case TriangleCullMode::FrontFaceCulling:
						cullMode = TriangleCullMode::BackFaceCulling;
						break;

					case TriangleCullMode::BackFaceCulling:
						cullMode = TriangleCullMode::FrontFaceCulling;
						break;
				}
			}

			switch (cullMode)
			{
				case TriangleCullMode::FrontFaceCulling:
					if (nvDot < 0.0f)
					{
						return false;
					}
					break;

				case TriangleCullMode::BackFaceCulling:
					if (nvDot > 0.0f)
					{
						return false;
					}
					break;
			}

			float t = Vector3::Dot(triangle.v0 - ray.origin, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}
			
			Vector3 intersect = ray.origin + ray.direction * t;

			Vector3 e1 = triangle.v0 - triangle.v1;
			Vector3 e2 = triangle.v1 - triangle.v2;
			Vector3 e3 = triangle.v2 - triangle.v0;

			Vector3 p1 = intersect - triangle.v1;
			Vector3 p2 = intersect - triangle.v2;
			Vector3 p3 = intersect - triangle.v0;

			float t1 = Vector3::Dot(Vector3::Cross(e1, p1), triangle.normal);
			float t2 = Vector3::Dot(Vector3::Cross(e2, p2), triangle.normal);
			float t3 = Vector3::Dot(Vector3::Cross(e3, p3), triangle.normal);

			if (t1 > 0.0f || t2 > 0.0f || t3 > 0.0f)
			{
				return false;
			}

			if (!ignoreHitRecord && t < hitRecord.t)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = intersect;
				hitRecord.normal = triangle.normal;
				hitRecord.t = t;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			static HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion

#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			static HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			switch (light.type)
			{
				case LightType::Point:
					return (light.origin - origin);

				case LightType::Directional:
					return -light.direction;
			}

			return Vector3::Zero;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			switch (light.type)
			{
				case LightType::Point:
					return light.color * (light.intensity / (light.origin - target).SqrMagnitude());

				case LightType::Directional:
					return light.color * light.intensity;
			}

			return colors::Black;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}