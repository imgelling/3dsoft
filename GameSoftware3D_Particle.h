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
			for (uint32_t part = 0; part < particlesAlive; ++part)
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

		void GeneratePointSprite(Camera3D& camera)
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


#endif 
