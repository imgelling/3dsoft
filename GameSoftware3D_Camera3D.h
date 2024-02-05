#if !defined(GAMESOFTWARE3D_CAMERA3D_H)
#define GAMESOFTWARE3D_CAMERA3D_H

#include "GameMath.h"

namespace game
{
	class Camera3D
	{
	public:
		Vector3f position;
		Vector3f rotation;
		const Vector3f defaultForward = { 0.0f, 0.0f, 1.0f };
		const Vector3f defaultUp = { 0.0f,1.0f,0.0f };
		const Vector3f defaultRight = { 1.0f,0.0f,0.0f };
		Vector3f forward;
		Vector3f right;
		Vector3f up;
		Matrix4x4f view;
		Camera3D();
		Camera3D(const Vector3f& inPosition, const Vector3f& inRotation);
		Matrix4x4f CreateViewMatrix() noexcept;
		//float yaw, pitch, roll;
		Matrix4x4f rotateM(const float ang, const Vector3f& axis) noexcept;
		void SetRotation(const float x, const float y, const float z) noexcept;

	private:
	};

	inline Matrix4x4f Camera3D::rotateM(const float ang, const Vector3f& axis) noexcept
	{
		float c = cos(ang);
		float s = sin(ang);
		float x = axis.x;
		float y = axis.y;
		float z = axis.z;
		Vector3f temp(axis);
		temp.x = temp.x * (1.0f - c);
		temp.y = temp.y * (1.0f - c);
		temp.z = temp.z * (1.0f - c);


		Matrix4x4f ret;
		ret.m[0] = temp.x * x + c;
		ret.m[1] = temp.x * y + z * s;
		ret.m[2] = temp.x * z - y * s;

		ret.m[4] = temp.y * x - z * s;
		ret.m[5] = c + temp.y * y;
		ret.m[6] = temp.y * z + x * s;

		ret.m[8] = temp.z * x + y * s;
		ret.m[9] = temp.z * y - x * s;
		ret.m[10] = c + temp.z * z;

		return ret;
	}

	inline void Camera3D::SetRotation(const float x, const float y, const float z) noexcept
	{
		// Setup rotation matrices

		if (x)
		{
			rotation.x += x;
			//pitch += x;
			// limits over rotaion
			rotation.x = min(rotation.x, 3.1415f / 2.0f);
			rotation.x = max(rotation.x, -3.1415f / 2.0f);
			//pitch = min(pitch, 3.1415f / 2.0f);
			//pitch = max(pitch, -3.1415f / 2.0f);
		}
		else if (y)
		{
			rotation.y += y;
			//yaw += y;
		}
		else if (z)
		{
			// nothing yet
			rotation.z += z;
			//roll += z;
		}

		forward.x = -cos(rotation.y) * -cos(rotation.x);
		forward.y = -sin(rotation.x);
		forward.z = sin(rotation.y) * -cos(rotation.x);
		forward.Normalize();

		//Vector3f newUp(0.0f, 1.0f, 0.0f);
		right = defaultUp.Cross(forward);
		right.Normalize();

		up = (forward.Cross(right));
		up.Normalize();
	}

	inline Matrix4x4f Camera3D::CreateViewMatrix() noexcept
	{
		view.SetIdentity();
		view.m[0] = right.x;
		view.m[4] = right.y;
		view.m[8] = right.z;

		view.m[1] = up.x;
		view.m[5] = up.y;
		view.m[9] = up.z;

		view.m[2] = forward.x;
		view.m[6] = forward.y;
		view.m[10] = forward.z;

		//Matrix4x4f r;// = rotateM(roll, forward);

		//view = view * r;

		Matrix4x4f ct;
		ct.SetTranslation(-position.x, -position.y, -position.z);
		view = view * ct;

		return view;
	}

	inline Camera3D::Camera3D()
	{
		rotation.y = -3.14159f / 2.0f; // y is 90 degrees off
		position = { 0.0f,0.0f,0.0f };
		forward = defaultForward;
		up = defaultUp;
		right = defaultRight;
	}

	inline Camera3D::Camera3D(const Vector3f& inPosition, const Vector3f& inRotation)
	{
		position = inPosition;
		rotation = inRotation;
		forward = defaultForward;
		up = defaultUp;
		right = defaultRight;
	}
}

#endif
