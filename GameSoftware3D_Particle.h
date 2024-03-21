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

		void InitializeParticles(const Pointf& __restrict size, const Vector3f& __restrict inposition, const float_t rotation, const Color& color)
		{
			// original
			//for (Particle &parts:particles)
			//{
			//	parts.Initialize(size, position, rotation, color);
			//}
			Random random;
			Vector3f pos;// = inposition;
			float_t rpos = 0;
			uint32_t c = 0;
			Color col;
			random.NewSeed();
			for (Particle& parts : particles)
			{
				rpos = (random.RndRange(0, 100) / 400.0f) - 0.13f;
				pos = Position;
				pos.x += rpos;
				rpos = (random.RndRange(0, 100) / 400.0f) - 0.13f;
				pos.z += rpos;
				parts.velocity.y = (random.RndRange(0, 200) / 400.0f);
				parts.velocity.y = max(parts.velocity.y, 0.005f);
				//c = random.RndRange(0, 2);
				//if (c == 0)
					col = Colors::Yellow;
				//else if (c == 1)
					//col = Colors::Red;
				//else if (c == 2)
					//col = Colors::DarkGray;
				//parts.sprite.color.Set(random.RndRange(0, 255), random.RndRange(0, 255), random.RndRange(0, 255), 255);
				parts.alive = true;
				parts.timeToLive = 0.5f + random.RndRange(0, 100) / 100.0f;
				parts.Initialize(size, pos, rotation, col);
			}
		}

		void UpdateBillboard(const Camera3D& camera)
		{
			particles[0].sprite.GenerateBillboardMatrix(camera);
			billboard = particles[0].sprite.billboard;
		}

		Pointf lerp2D(const Pointf& __restrict b, const Pointf& __restrict a, float_t t) {
			Pointf result;
			result.x = a.x + t * (b.x - a.x);
			result.y = a.y + t * (b.y - a.y);
			return result;
		}

		void Update(const float_t msElapsed)
		{		
			//static float_t pos = 0.0f;
			static float_t rotation = 0.0f;
			game::Random random;
			float_t time = msElapsed / 1000.0f;
			rotation += (2 * 3.14f / 10.0f) * (time);
			//pos = 0.5f * (msElapsed / 1000.0f);
			
			uint32_t count = 0;
			partsAlive = 0;
			for (Particle &part:particles)
			{
				//sprite.position = { 0,-1,0};
				part.sprite.position.y -= part.velocity.y * (time);
				part.timeToLive -= time;
				part.sprite.size = lerp2D({ 0.025f,0.025f }, { 0,0 }, part.timeToLive);
				if (part.timeToLive < 0.95)
				{
					part.sprite.color = Colors::Red;
				}
				if (part.timeToLive < 0.55)
				{
					part.sprite.color = Colors::DarkGray;
				}
				if (/*part.sprite.position.y < -0.5f ||*/ part.timeToLive < 0.0f)
				{
					part.sprite.position.y = Position.y;// model.centerPoint.y;
					part.sprite.size.x = 0.025f;
					part.sprite.size.y = 0.025f;
					part.timeToLive = 0.85f + random.RndRange(0,100) / 100.0f;
					part.sprite.color = Colors::Yellow;
					//pos = 0.0f;

					// kills particle, wont be rendered
					//part.alive = false;
				}
				if (part.alive)
				{
					part.sprite.billboard = billboard;
					part.sprite.rotation = rotation;
					part.sprite.billboard.m[12] = part.sprite.position.x;
					part.sprite.billboard.m[13] = part.sprite.position.y;
					part.sprite.billboard.m[14] = part.sprite.position.z;

					part.sprite.UpdateQuad(mesh.tris[count], mesh.tris[(uint64_t)1 + count]);
					count += 2;
					partsAlive++;
				}
			}
		}

		Matrix4x4f billboard;
		uint32_t numberOfParticles;
		std::vector<Particle> particles;
		Vector3f Position;
		Mesh mesh;
		uint32_t partsAlive;

	private:
	};



}


#endif 
