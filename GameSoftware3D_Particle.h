#if !defined(GAMESOFTWARE3D_PARTICLE_H)
#define GAMESOFTWARE3D_PARTICLE_H

#include "GameMath.h"
#include "GameSoftware3D_Data.h"
#include "GameSoftware3D_PointSprite.h"

namespace game
{
	class Particle
	{
	public:
		Particle()
		{
			alive = false;
			timeToLive = 0.0f;
		}
		Vector3f velocity;
		bool alive;
		float_t timeToLive;
		void Update(const float_t msElapsed)
		{

		}
		void Initialize(const Pointf& __restrict size, const Vector3f& __restrict position, const float_t rotation, const Color& color)
		{
			sprite.position = position;
			sprite.rotation = rotation;
			sprite.color = color;
			sprite.size = size;
		}

		PointSprite sprite;
	private:
	};

	class Emitter
	{
	public :
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
				particle.sprite.GenerateQuad(t1, t2);
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
				part.sprite.position.x = Position.x + (random.RndRange(0, 100) / 650.0f) - 0.07f;
				part.sprite.position.y = Position.y;
				part.sprite.position.z = Position.z + (random.RndRange(0, 100) / 650.0f) - 0.07f;

				part.velocity.y = (random.RndRange(0, 200) / 400.0f);
				part.velocity.y = part.velocity.y < 0.005f ? 0.005f : part.velocity.y;
				part.velocity.x = (random.RndRange(0, 200) / 400.0f);
				part.velocity.x = part.velocity.x > 0.15f ? 0.15f : part.velocity.x;


				part.timeToLive = 0.85f + random.RndRange(0, 25) / 100.0f;
				part.sprite.size.x = part.timeToLive * 0.025f;
				part.sprite.size.y = part.timeToLive * 0.025f;
				part.sprite.color = Colors::White;
			}
		}

		void UpdateBillboard(const Camera3D& camera)
		{
			particles[0].sprite.GenerateBillboardMatrix(camera);
			billboard = particles[0].sprite.billboard;
		}

		// go in gamemath.h
		Pointf lerp2D(const Pointf& __restrict b, const Pointf& __restrict a, float_t t) {
			Pointf result;
			result.x = a.x + t * (b.x - a.x);
			result.y = a.y + t * (b.y - a.y);
			return result;
		}

		void Update(const float_t msElapsed)
		{		
			static float_t rotation = 0.0f;

			float_t time = msElapsed / 1000.0f;
			rotation += (2 * 3.14f / 10.0f) * (time);
			
			uint32_t count = 0;
			partsAlive = 0;
			for (Particle &part:particles)
			{
				part.timeToLive -= time;
				if (part.timeToLive <= 0.0f)
				{
					part.sprite.position.x = Position.x + (random.RndRange(0, 100) / 650.0f) - 0.07f;
					part.sprite.position.y = Position.y;
					part.sprite.position.z = Position.z + (random.RndRange(0, 100) / 650.0f) - 0.07f;

					part.velocity.y = (random.RndRange(0, 200) / 400.0f);
					part.velocity.y = part.velocity.y < 0.005f ? 0.005f : part.velocity.y;
					part.velocity.x = (random.RndRange(0, 200) / 400.0f);
					part.velocity.x = part.velocity.x > 0.15f ? 0.15f : part.velocity.x;


					part.timeToLive = 0.85f + random.RndRange(0, 25) / 100.0f;
					part.sprite.size.x = part.timeToLive * 0.025f;
					part.sprite.size.y = part.timeToLive * 0.025f;
					part.sprite.color = Colors::White;

					// kills particle, wont be rendered
					//part.alive = false;
				}
				else
				{
					part.sprite.position.y -= part.velocity.y * (time);
					part.sprite.position.x -= part.velocity.x * (time);

					//part.sprite.size = lerp2D({ 0.025f,0.025f }, { 0,0 }, min(part.timeToLive,1.0f));
					part.sprite.size.x = part.timeToLive * 0.025f;
					part.sprite.size.y = part.timeToLive * 0.025f;

					if (part.timeToLive < 0.35)
					{
						part.sprite.color.Set(1.0f, 0.25f, 0, 0.25f);
					}
					else if (part.timeToLive < 0.5)
					{
						part.sprite.color.Set(1.0f, 0.25f, 0, 0.75f);
					}
					else if (part.timeToLive < 0.9)
					{
						part.sprite.color = Colors::DarkOrange;
					}
					else if (part.timeToLive < 1.0)
					{
						part.sprite.color = Colors::Yellow;
					}
					else if (part.timeToLive < 1.1)
					{
						part.sprite.color = Colors::White;
					}

					//if (part.alive)
					{
						part.sprite.rotation = rotation;
						billboard.m[12] = part.sprite.position.x;
						billboard.m[13] = part.sprite.position.y;
						billboard.m[14] = part.sprite.position.z;

						part.sprite.UpdateQuad(mesh.tris[count], mesh.tris[(uint64_t)1 + count], billboard);
						count += 2;
						partsAlive++;
					}
				}
			}
		}

		Matrix4x4f billboard;
		uint32_t numberOfParticles;
		std::vector<Particle> particles;
		Vector3f Position;
		Mesh mesh;
		uint64_t partsAlive;
		game::Random random;

	private:
	};



}


#endif 
