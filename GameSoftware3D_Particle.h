#if !defined(GAMESOFTWARE3D_PARTICLE_H)
#define GAMESOFTWARE3D_PARTICLE_H

#include "GameMath.h"
#include "GameSoftware3D_Data.h"
#include "GameSoftware3D_PointSprite.h"

namespace game
{
	class Particle : public PointSprite
	{
	public:
		//PointSprite sprite;
		Vector3f velocity;
		bool alive;
		float_t timeToLive;

		Particle()
		{
			alive = false;
			timeToLive = 0.0f;
		}
		void Update(const float_t msElapsed)
		{

		}
		void Initialize(const Pointf& __restrict size, const Vector3f& __restrict position, const float_t rotation, const Color& color)
		{
			this->position = position;
			this->rotation = rotation;
			this->color = color;
			this->size = size;
		}


	private:
	};

	class Emitter
	{
	public :
		Matrix4x4f pointSprite;
		uint32_t numberOfParticles;
		std::vector<Particle> particles;
		Vector3f Position;
		Mesh mesh;
		uint64_t partsAlive;
		game::Random random;

		Emitter()
		{
			numberOfParticles = 0;
			partsAlive = 0;
		}
		void Initialize(const uint32_t numParticles, const Vector3f& position)
		{
			numberOfParticles = numParticles;
			mesh.centerPoint = position;
			Position = position;
			
			Triangle t1, t2;
			Particle particle;
			for (uint32_t points = 0; points < numberOfParticles; ++points)
			{
				particle.GenerateQuad(t1, t2);
				mesh.tris.emplace_back(t1);
				mesh.tris.emplace_back(t2);
				particles.emplace_back(particle);
			}			
		}

		// needs to be custom, maybe just add a particle
		void InitializeParticles(const Pointf& __restrict size, const Vector3f& __restrict inposition, const float_t rotation, const Color& color)
		{
			random.NewSeed();
			for (Particle& part : particles)
			{
				part.position.x = Position.x + (random.RndRange(0, 100) / 650.0f) - 0.07f;
				part.position.y = Position.y;
				part.position.z = Position.z + (random.RndRange(0, 100) / 650.0f) - 0.07f;

				part.velocity.y = (random.RndRange(0, 200) / 400.0f);
				part.velocity.y = part.velocity.y < 0.005f ? 0.005f : part.velocity.y;
				part.velocity.x = (random.RndRange(0, 200) / 400.0f);
				part.velocity.x = part.velocity.x > 0.15f ? 0.15f : part.velocity.x;


				part.timeToLive = 0.85f + random.RndRange(0, 25) / 100.0f;
				part.size.x = part.timeToLive * 0.025f;
				part.size.y = part.timeToLive * 0.025f;
				part.color = Colors::White;

				part.rotation = (float_t)(random.RndRange(0, 359) * 3.14159f / 180.0f);
			}
		}

		// go in gamemath.h
		Pointf lerp2D(const Pointf& __restrict b, const Pointf& __restrict a, float_t t) {
			Pointf result;
			result.x = a.x + t * (b.x - a.x);
			result.y = a.y + t * (b.y - a.y);
			return result;
		}

		void Update(const float_t msElapsed, const Camera3D& camera)
		{		
			static float_t rotation = 0.0f;

			float_t time = msElapsed / 1000.0f;
			rotation = (2 * 3.14f / 10.0f) * (time);
			
			uint32_t count = 0;
			partsAlive = 0;
			uint64_t sizeOfParticles = particles.size();
			if (sizeOfParticles)
			{
				particles[0].GeneratePointSpriteMatrix(camera);
				pointSprite = particles[0].pointSprite;
				for (uint32_t part = 0; part < sizeOfParticles; ++part)
				{
					particles[part].timeToLive -= time;
					if (particles[part].timeToLive <= 0.0f)
					{
						particles[part].position.x = Position.x + (random.RndRange(0, 100) / 650.0f) - 0.07f;
						particles[part].position.y = Position.y;
						particles[part].position.z = Position.z + (random.RndRange(0, 100) / 650.0f) - 0.07f;

						particles[part].velocity.y = (random.RndRange(0, 200) / 400.0f);
						particles[part].velocity.y = particles[part].velocity.y < 0.005f ? 0.005f : particles[part].velocity.y;
						particles[part].velocity.x = (random.RndRange(0, 200) / 400.0f);
						particles[part].velocity.x = particles[part].velocity.x > 0.15f ? 0.15f : particles[part].velocity.x;


						particles[part].timeToLive = 0.85f + random.RndRange(0, 25) / 100.0f;
						particles[part].size.x = particles[part].timeToLive * 0.025f;
						particles[part].size.y = particles[part].timeToLive * 0.025f;
						particles[part].color = Colors::White;

						particles[part].rotation = (float_t)(random.RndRange(0, 359) * 3.14159f / 180.0f);

						// kills particle, wont be rendered
						//part.alive = false;
					}
					else
					{
						particles[part].position.y -= particles[part].velocity.y * (time);
						particles[part].position.x -= particles[part].velocity.x * (time);

						//part.sprite.size = lerp2D({ 0.025f,0.025f }, { 0,0 }, min(part.timeToLive,1.0f));
						particles[part].size.x = min(particles[part].timeToLive, 1.0f) * 0.025f;
						particles[part].size.y = min(particles[part].timeToLive, 1.0f) * 0.025f;

						if (particles[part].timeToLive < 0.35)
						{
							particles[part].color.Set(1.0f, 0.25f, 0, 0.25f);
						}
						else if (particles[part].timeToLive < 0.5)
						{
							particles[part].color.Set(1.0f, 0.25f, 0, 0.75f);
						}
						else if (particles[part].timeToLive < 0.9)
						{
							particles[part].color = Colors::DarkOrange;
						}
						else if (particles[part].timeToLive < 1.0)
						{
							particles[part].color = Colors::Yellow;
						}
						else if (particles[part].timeToLive < 1.1)
						{
							particles[part].color = Colors::White;
						}

						//if (part.alive)
						{
							particles[part].rotation += rotation;
							pointSprite.m[12] = particles[part].position.x;
							pointSprite.m[13] = particles[part].position.y;
							pointSprite.m[14] = particles[part].position.z;

							particles[part].UpdateQuad(mesh.tris[count], mesh.tris[(uint64_t)1 + count], pointSprite);
							count += 2;
							partsAlive++;
						}
					}
				}
			}
		}
	private:
	};



}


#endif 
