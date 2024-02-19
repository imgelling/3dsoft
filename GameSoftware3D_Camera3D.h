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
		Matrix4x4f lookAt;
		Camera3D();
		Camera3D(const Vector3f& inPosition, const Vector3f& inRotation);
		void GenerateLookAtMatrix(Vector3f& point) noexcept;
		void GenerateViewMatrix() noexcept;
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
			// limits over rotaion
			rotation.x = min(rotation.x, 3.1415f / 2.0f);
			rotation.x = max(rotation.x, -3.1415f / 2.0f);
		}
		else if (y)
		{
			rotation.y += y;
		}
		else if (z)
		{
			rotation.z += z;
		}

		//Matrix4x4f rotx;
		//Matrix4x4f roty;
		//rotx.SetRotationX(x);
		//roty.SetRotationY(y);
		//forward = forward * roty * rotx;
		forward.x = cos(-rotation.y) * cos(-rotation.x);
		forward.y = sin(-rotation.x);
		forward.z = sin(-rotation.y) * cos(-rotation.x);


		forward.Normalize();

		right = defaultUp.Cross(forward);
		right.Normalize();

		up = (forward.Cross(right));
		up.Normalize();
	}

	inline void Camera3D::GenerateViewMatrix() noexcept
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

		Matrix4x4f ct;
		ct.SetTranslation(-position.x, -position.y, -position.z);
		view = view * ct;
	}

	inline void Camera3D::GenerateLookAtMatrix(Vector3f& point) noexcept
	{
		lookAt.SetIdentity();
		Vector3f f = point - position;
		//forward = point - position;
		f.Normalize();
		//forward.Normalize();
		Vector3f r = defaultUp.Cross(f);
		//right = defaultUp.Cross(forward);
		r.Normalize();
		//right.Normalize();
		Vector3f u = f.Cross(r);
		//up = (forward.Cross(right));
		//up.Normalize();
		u.Normalize();

		lookAt.m[0] = r.x;
		lookAt.m[4] = r.y;
		lookAt.m[8] = r.z;

		lookAt.m[1] = u.x;
		lookAt.m[5] = u.y;
		lookAt.m[9] = u.z;

		lookAt.m[2] = f.x;
		lookAt.m[6] = f.y;
		lookAt.m[10] = f.z;

		lookAt.m[12] = r.Dot(position * -1.0f);
		lookAt.m[13] = u.Dot(position * -1.0f);
		lookAt.m[14] = f.Dot(position * -1.0f);

		//Matrix4x4f ct;
		//ct.SetTranslation(-position.x, -position.y, -position.z);
		//lookAt = lookAt * ct;
	}



	inline Camera3D::Camera3D()
	{
		position = { 0.0f,0.0f,0.0f };
		rotation = { 0.0f,-3.14159f / 2.0f,0.0f };
		forward = defaultForward;
		up = defaultUp;
		right = defaultRight;
		SetRotation(0, 0, 0);
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
