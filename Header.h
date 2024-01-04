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

//
//void DrawWireFrame(const Triangle& tri, const game::Color& color)
//{
//	fence++;
//	pixelMode.LineClip(
//		(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
//		(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
//		color);
//	pixelMode.LineClip(
//		(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
//		(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
//		color);
//	pixelMode.LineClip(
//		(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
//		(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
//		color);
//}


//void DrawColoredBlock(const Triangle& tri)
//{
//	fence++;
//	game::Vector2f v0(tri.vertices[0].x, tri.vertices[0].y);
//	game::Vector2f v1(tri.vertices[1].x, tri.vertices[1].y);
//	game::Vector2f v2(tri.vertices[2].x, tri.vertices[2].y);
//
//	EdgeEquation e0(v1, v2);
//	EdgeEquation e1(v2, v0);
//	EdgeEquation e2(v0, v1);
//	EdgeEquation b0(v1, v2);
//	EdgeEquation b1(v2, v0);
//	EdgeEquation b2(v0, v1);
//
//
//	// back face cull
//	float area = (e0.c + e1.c + e2.c);
//	if (area < 0)
//	{
//		return;
//	}
//
//	game::Recti boundingBox = TriangleBoundingBox(tri);
//	game::Vector2f pixelOffset;
//
//	int blockSize = 4;
//	float s = (float)blockSize - 1;
//
//	// Round to block grid.
//
//	int minX = boundingBox.x - blockSize;// &~(blockSize - 1);
//	int maxX = boundingBox.right + blockSize;// &~(blockSize - 1);
//	int minY = boundingBox.y - blockSize;// &~(blockSize - 1);
//	int maxY = boundingBox.bottom + blockSize;// &~(blockSize - 1);
//
//	game::Vector2f samples[4];
//	bool eres[4][3] = {};
//	pixelMode.Rect({ minX,minY,maxX, maxY }, game::Colors::White);
//	bool lasthblock = false;
//	bool lastvblock = false;
//	for (int y = minY; y < maxY; y += blockSize)
//	{
//		lasthblock = false;
//		for (int x = minX; x < maxX; x += blockSize)
//		{
//			// tl
//			samples[0].x = x + 0.5f;
//			samples[0].y = y + 0.5f;
//
//			// tr
//			samples[1].x = x + blockSize + 0.5f;
//			samples[1].y = y + 0.5f;
//
//			// br
//			samples[2].x = x + blockSize + 0.5f;
//			samples[2].y = y + blockSize + 0.5f;
//
//			// bl
//			samples[3].x = x + 0.5f;
//			samples[3].y = y + blockSize + 0.5f;
//
//			bool found[4] = {};
//			//eres[4][3] = {};
//			for (int i = 0; i < 2; i++)
//			{
//				eres[i][0] = e0.test(samples[i].x, samples[i].y);
//				eres[i][1] = e1.test(samples[i].x, samples[i].y);
//				eres[i][2] = e2.test(samples[i].x, samples[i].y);
//				if (eres[i][0]) goto step;
//				if (eres[i][1]) goto step;
//				if (eres[i][2]) goto step;
//				found[i] = true;
//				pixelMode.Rect({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::White);
//			step:
//				e0.stepX((float_t)s);
//				e1.stepX((float_t)s);
//				e2.stepX((float_t)s);
//			}
//			e0 = b0;// .stepX(-(float_t)s * 2.0f);
//			e1 = b1;// .stepX(-(float_t)s * 2.0f);
//			e2 = b2;// .stepX(-(float_t)s * 2.0f);
//			e0.stepY((float_t)s);
//			e1.stepY((float_t)s);
//			e2.stepY((float_t)s);
//			//eres[4] = {};
//			for (int i = 2; i < 4; i++)
//			{
//				//if (e0.test(samples[i].x, samples[i].y)) goto step2;
//				//if (e1.test(samples[i].x, samples[i].y)) goto step2;
//				//if (e2.test(samples[i].x, samples[i].y)) goto step2;
//				eres[i][0] = e0.test(samples[i].x, samples[i].y);
//				eres[i][1] = e1.test(samples[i].x, samples[i].y);
//				eres[i][2] = e2.test(samples[i].x, samples[i].y);
//				if (eres[i][0]) goto step2;
//				if (eres[i][1]) goto step2;
//				if (eres[i][2]) goto step2;
//				found[i] = true;
//				pixelMode.Rect({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::White);
//			step2:
//				e0.stepX((float_t)s);
//				e1.stepX((float_t)s);
//				e2.stepX((float_t)s);
//			}
//
//			int result = found[0] + found[1] + found[2] + found[3];
//			if (result == 0)
//			{
//				bool e00Same = eres[0][0] == eres[0][1] == eres[0][2];
//				bool e01Same = eres[1][0] == eres[1][1] == eres[1][2];
//				bool e10Same = eres[2][0] == eres[2][1] == eres[2][2];
//				bool e11Same = eres[3][0] == eres[3][1] == eres[3][2];
//
//				//if ((boundingBox.right - x < blockSize) && (lasthblock))
//				if (!e00Same || !e01Same || !e10Same || !e11Same)
//				{
//					pixelMode.RectFilled({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::Blue);
//				}
//				else if ((boundingBox.left - y < blockSize) && (lasthblock))
//				{
//					pixelMode.RectFilled({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::Blue);
//				}
//				else
//				{
//					//pixelMode.RectFilled({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::Green);
//				}
//				lasthblock = false;
//
//
//				// special case?
//			}
//			// Full block
//			else if (result == 4)
//			{
//				lasthblock = true;
//				lastvblock = true;
//				pixelMode.RectFilled({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::Magenta);
//			}
//			// Partial block
//			else
//			{
//				lasthblock = true;
//				lasthblock = true;
//				pixelMode.RectFilled({ x,y,(x + blockSize),(y + blockSize) }, game::Colors::DarkGray);
//			}
//
//
//
//
//			//pixelMode.Pixel(x, y, game::Colors::White);
//		}
//	}
//
//}