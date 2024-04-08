#if !defined(GAMESOFTWARE3D_PARTICLE_H)
#define GAMESOFTWARE3D_PARTICLE_H

#include "GameMath.h"
#include "GameSoftware3D_Data.h"
#include "GameSoftware3D_PointSprite.h"

namespace game
{
	class ParticleBase : public PointSprite
	{
	public:
		Vector3f velocity;
		float_t timeToLive;
		bool alive;

		ParticleBase()
		{
			alive = false;
			timeToLive = 0.0f;
		}
	private:
	};

	class EmitterBase
	{
	public :
		Matrix4x4f pointSprite;
		uint32_t numberOfParticles;
		std::vector<ParticleBase> particles;
		Vector3f Position;
		Mesh mesh;
		uint64_t particlesAlive;
		game::Random* random;
		bool Enabled;

		EmitterBase()
		{
			numberOfParticles = 0;
			particlesAlive = 0;
			Enabled = true;
			random = nullptr;
		}
		~EmitterBase()
		{
			if (random)
			{
				delete random;
			}
		}
		void Initialize(const uint32_t numParticles, const Vector3f& position) noexcept
		{
			numberOfParticles = numParticles;
			mesh.centerPoint = position; // Probably not needed
			Position = position;
			
			random = new Random;

			Triangle t1, t2;
			ParticleBase particle;
			for (uint32_t part = 0; part < numberOfParticles; ++part)
			{
				particle.GenerateQuad(t1, t2);
				mesh.tris.emplace_back(t1);
				mesh.tris.emplace_back(t2);
				particles.emplace_back(particle);
			}
		}

		void GenerateQuads()
		{
			uint32_t count = 0;

			for (uint32_t part = 0; part < numberOfParticles; ++part)
			{
				if (particles[part].alive)
				{

					pointSprite.m[12] = particles[part].position.x;
					pointSprite.m[13] = particles[part].position.y;
					pointSprite.m[14] = particles[part].position.z;

					particles[part].GenerateQuad(mesh.tris[count], mesh.tris[(uint64_t)1 + count], pointSprite);
					count += 2;
				}
			}
		}

		void GeneratePointSpriteMatrix(Camera3D& camera)
		{
			if (particlesAlive)
			{
				particles[0].GeneratePointSpriteMatrix(camera);
				pointSprite = particles[0].pointSprite;
			}
		}


	private:
	};



}


//class Fire : public game::EmitterBase
//{
//public:
//	void InitializeParticles() noexcept
//	{
//		random->NewSeed();
//		for (game::ParticleBase& part : particles)
//		{
//			part.alive = true;
//			part.position.x = Position.x + (random->RndRange(0, 100) / 650.0f) - 0.07f;
//			part.position.y = Position.y;
//			part.position.z = Position.z + (random->RndRange(0, 100) / 650.0f) - 0.07f;
//
//			part.velocity.y = (random->RndRange(0, 200) / 400.0f);
//			part.velocity.y = part.velocity.y < 0.005f ? 0.005f : part.velocity.y;
//			part.velocity.x = (random->RndRange(0, 200) / 400.0f);
//			part.velocity.x = part.velocity.x > 0.15f ? 0.15f : part.velocity.x;
//
//
//			part.timeToLive = 0.85f + random->RndRange(0, 25) / 100.0f;
//			part.size.x = part.timeToLive * 0.025f;
//			part.size.y = part.timeToLive * 0.025f;
//			part.color = game::Colors::White;
//
//			part.rotation = (float_t)(random->RndRange(0, 359) * 3.14159f / 180.0f);
//		}
//	}
//
//	void Update(const float_t msElapsed)
//	{
//
//		float_t time = msElapsed / 1000.0f;
//		rotation = (2 * 3.14f / 10.0f) * (time);
//
//		uint32_t count = 0;
//		particlesAlive = 0;
//		uint64_t sizeOfParticles = particles.size();
//
//		if (sizeOfParticles)
//		{
//			for (uint32_t part = 0; part < sizeOfParticles; ++part)
//			{
//				particles[part].timeToLive -= time;
//				if (particles[part].timeToLive > 0.0f)
//				{
//					particles[part].position.y -= particles[part].velocity.y * (time);
//					particles[part].position.x -= particles[part].velocity.x * (time);
//
//					particles[part].rotation += rotation;
//
//					particles[part].size.x = min(particles[part].timeToLive, 1.0f) * 0.025f;
//					particles[part].size.y = min(particles[part].timeToLive, 1.0f) * 0.025f;
//
//					if (particles[part].timeToLive < 0.35)
//					{
//						particles[part].color.Set(1.0f, 0.25f, 0, 0.25f);
//					}
//					else if (particles[part].timeToLive < 0.5)
//					{
//						particles[part].color.Set(1.0f, 0.25f, 0, 0.75f);
//					}
//					else if (particles[part].timeToLive < 0.9)
//					{
//						particles[part].color = game::Colors::DarkOrange;
//					}
//					else if (particles[part].timeToLive < 1.0)
//					{
//						particles[part].color = game::Colors::Yellow;
//					}
//					else if (particles[part].timeToLive < 1.1)
//					{
//						particles[part].color = game::Colors::White;
//					}
//					particlesAlive++;
//				}
//				else
//				{
//					particles[part].position.x = Position.x + (random->RndRange(0, 100) / 650.0f) - 0.07f;
//					particles[part].position.y = Position.y;
//					particles[part].position.z = Position.z + (random->RndRange(0, 100) / 650.0f) - 0.07f;
//
//					particles[part].velocity.y = (random->RndRange(0, 200) / 400.0f);
//					particles[part].velocity.y = particles[part].velocity.y < 0.005f ? 0.005f : particles[part].velocity.y;
//					particles[part].velocity.x = (random->RndRange(0, 200) / 400.0f);
//					particles[part].velocity.x = particles[part].velocity.x > 0.15f ? 0.15f : particles[part].velocity.x;
//
//					particles[part].timeToLive = 0.85f + random->RndRange(0, 25) / 100.0f;
//					particles[part].size.x = particles[part].timeToLive * 0.025f;
//					particles[part].size.y = particles[part].timeToLive * 0.025f;
//					particles[part].color = game::Colors::White;
//
//					particles[part].rotation = (float_t)(random->RndRange(0, 359) * 3.14159f / 180.0f);
//
//					// kills particle, wont be rendered
//					//particles[part].alive = false;
//				}
//			}
//		}
//	}
//private:
//	float_t rotation = 0.0f;
//};

//class StarField : public game::EmitterBase
//{
//public:
//	void InitializeParticles(game::Camera3D camera) noexcept
//	{
//		random->NewSeed();
//		for (game::ParticleBase& part : particles)
//		{
//			ResetParticle(part, camera);
//		}
//	}
//
//	void ResetParticle(game::ParticleBase& part, const game::Camera3D& camera)
//	{
//
//		part.position.x = (float_t)((random->Randf() * 2.0) - 1.0f) * 10.0f;
//		part.position.y = (float_t)((random->Randf() * 2.0) - 1.0f) * 10.0f;
//		if (part.alive) // not first interation
//		{
//			part.position.z = (10.0f);// +camera.position.z;
//		}
//		else
//		{
//			part.position.z = ((random->Randf() * 2.0f) - 1.0f) * 25.0f;
//			part.size.x = 0.025f;// part.timeToLive * 0.025f;
//			part.size.y = 0.025f;// part.timeToLive * 0.025f;
//		}
//
//		float_t randColor = random->Randf();
//		part.color = game::Colors::White;
//		if (randColor < 0.1f)
//			part.color = game::Colors::Yellow;
//		if (randColor < 0.05)
//			part.color = game::Colors::Red;
//		if (randColor < 0.005)
//			part.color = game::Colors::DarkOrange;
//		part.velocity.z = -random->Randf();
//		if (part.velocity.z > -0.2) part.velocity.z = -0.2;
//
//		part.rotation = (float_t)(random->RndRange(0, 359) * 3.14159f / 180.0f);
//
//		part.alive = true;
//	}
//
//	void Update(const float_t msElapsed, const game::Camera3D& camera)
//	{
//
//		float_t time = msElapsed / 1000.0f;
//		rotation = (2 * 3.14f / 20.0f) * (time);
//
//		uint32_t count = 0;
//		particlesAlive = 0;
//		uint64_t sizeOfParticles = particles.size();
//
//		for (uint32_t part = 0; part < sizeOfParticles; ++part)
//		{
//			//particles[part].alive = false;
//			// Is particle behind camera, skip it
//			//if ((particles[part].position - camera.position).z < 0) continue;
//			particles[part].rotation += rotation;
//			//particles[part].alive = true;
//			particlesAlive++;
//		}
//	}
//private:
//	float_t rotation = 0.0f;
//};

#endif 
