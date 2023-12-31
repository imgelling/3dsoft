#pragma once

//// Returns the signed area of the triangle formed by
//// points a,b,c with clockwise winding
//inline float_t edgeFunctionCW(const game::Vector2f& a, const game::Vector2f& b, const game::Vector2f& c)
//{
//	return -((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
//}
//
//// Returns the signed area of the triangle formed by
//// points a,b,c with counter (anti) clockwise winding
//inline float_t edgeFunctionCCW(const game::Vector2f& a, const game::Vector2f& b, const game::Vector2f& c)
//{
//	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
//}

//void DrawColored(const Triangle& tri)
//{
//	game::Vector2f v0(tri.vertices[0].x, tri.vertices[0].y);
//	game::Vector2f v1(tri.vertices[1].x, tri.vertices[1].y);
//	game::Vector2f v2(tri.vertices[2].x, tri.vertices[2].y);

//	game::Rectf boundingBox = TriangleBoundingBox(tri);

//	float_t area = edgeFunctionCW(v0, v1, v2);
//	float_t oneOverArea = 1.0f / area;
//	float_t w0(0.0f);
//	float_t w1 = 0.0f;
//	float_t w2 = 0.0f;
//	float_t r = 0.0f;
//	float_t g = 0.0f;
//	float_t b = 0.0f;
//	game::Vector2f p;

//	// test optimization
//	bool foundTriangle = false;


//	for (int32_t j = (int32_t)boundingBox.y; j < (int32_t)boundingBox.bottom; j++) 
//	{
//		foundTriangle = false;
//		for (int32_t i = (int32_t)boundingBox.x; i < (int32_t)boundingBox.right; ++i) 
//		{
//			p = { i + 0.5f , j + 0.5f  };

//			w0 = edgeFunctionCW(v1, v2, p);
//			if (w0 < 0.0f)  // >= for clockwise triangles  <= counter
//			{					
//				if (foundTriangle)
//				{
//					break;
//				}
//				else
//				{
//					pixelMode.Pixel(i, j, game::Colors::Pink);
//					continue;
//				}
//			}
//			w1 = edgeFunctionCW(v2, v0, p);
//			if (w1 < 0.0f)
//			{
//				if (foundTriangle)
//				{
//					break;
//				}
//				else
//				{
//					pixelMode.Pixel(i, j, game::Colors::Pink);
//					continue;
//				}
//			}
//			w2 = edgeFunctionCW(v0, v1, p);
//			if (w2 < 0.0f)
//			{
//				if (foundTriangle)
//				{
//					break;
//				}
//				else
//				{
//					pixelMode.Pixel(i, j, game::Colors::Pink);
//					continue;
//				}
//			}


//			foundTriangle = true;

//			//// wireframe test
//			//int w = 200;
//			//if (((int)w0 <= w) || ((int)w1 <= w) || ((int)w2 <= w))
//			//{
//			//	pixelMode.Pixel(i, j, game::Colors::White);
//			//	continue;
//			//}


//			w0 *= oneOverArea;
//			w1 *= oneOverArea;
//			w2 *= oneOverArea;

//			// Calculates the color
//			r = w0 * tri.vertices[0].r + w1 * tri.vertices[1].r + w2 * tri.vertices[2].r;
//			g = w0 * tri.vertices[0].g + w1 * tri.vertices[1].g + w2 * tri.vertices[2].g;
//			b = w0 * tri.vertices[0].b + w1 * tri.vertices[1].b + w2 * tri.vertices[2].b;
//			pixelMode.Pixel(i, j, game::Color(r, g, b, 1.0f));

//		}
//	}
//	fence++;
//	//std::cout << boundingBox.right - boundingBox.left << "," << boundingBox.bottom - boundingBox.top << "\n";
//}

/*
* // Does it pass the top-left rule?
Vec2f v0 = { ... };
Vec2f v1 = { ... };
Vec2f v2 = { ... };

float w0 = edgeFunction(v1, v2, p);
float w1 = edgeFunction(v2, v0, p);
float w2 = edgeFunction(v0, v1, p);

Vec2f edge0 = v2 - v1;
Vec2f edge1 = v0 - v2;
Vec2f edge2 = v1 - v0;

bool overlaps = true;

// If the point is on the edge, test if it is a top or left edge,
// otherwise test if  the edge function is positive
overlaps &= (w0 == 0 ? ((edge0.y == 0 && edge0.x > 0) ||  edge0.y > 0) : (w0 > 0));
overlaps &= (w1 == 0 ? ((edge1.y == 0 && edge1.x > 0) ||  edge1.y > 0) : (w1 > 0));
overlaps &= (w1 == 0 ? ((edge2.y == 0 && edge2.x > 0) ||  edge2.y > 0) : (w2 > 0));

if (overlaps) {
	// pixel overlap the triangle
	...draw it
}
	*/