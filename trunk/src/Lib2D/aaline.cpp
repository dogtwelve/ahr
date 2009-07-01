#include "aaline.h"
#include "vector.h"
#include "lib2d/Lib2d.h"



#define OUTCODE_TOP		0x1
#define OUTCODE_BOTTOM	0x2
#define OUTCODE_RIGHT	0x4
#define OUTCODE_LEFT	0x8


int ComputeOutCode(const Vector2s& position,
				   const Vector2s& pos_min,
				   const Vector2s& pos_max)
{
	int outcode = 0;
	if(position.y > pos_max.y)
		outcode |= OUTCODE_TOP;
	else if(position.y < pos_min.y)
		outcode |= OUTCODE_BOTTOM;
	if(position.x > pos_max.x)
		outcode |= OUTCODE_RIGHT;
	else if(position.x < pos_min.x)
		outcode |= OUTCODE_LEFT;
	return outcode;
}

bool ClipLine(Vector2s& start,
			  Vector2s& end,
			  const Vector2s& pos_min,
			  const Vector2s& pos_max)
{
	int outcode_start =  ComputeOutCode(start,pos_min,pos_max);
	int outcode_end =  ComputeOutCode(end,pos_min,pos_max);
	int outcode_out;
	bool done = false;
	bool accept = false;

	do 
	{
		if(!(outcode_start|outcode_end))
		{
			done = true;
			accept = true;
		}
		else if(outcode_start&outcode_end)
		{
			done = true;
		}
		else
		{
			int x;
			int y;

			outcode_out = outcode_start ? outcode_start : outcode_end;
			if(outcode_out & OUTCODE_TOP)
			{
				x = (end.x - start.x)*(pos_max.y - start.y);
				x /= end.y - start.y;
				x += start.x;
				y = pos_max.y;
			}
			else if(outcode_out & OUTCODE_BOTTOM)
			{
				x = (end.x - start.x)*(pos_min.y - start.y);
				x /= end.y - start.y;
				x += start.x;
				y = pos_min.y;
			}
			else if(outcode_out & OUTCODE_RIGHT)
			{
				y = (end.y - start.y)*(pos_max.x - start.x);
				y /= end.x - start.x;
				y += start.y;
				x = pos_max.x;
			}
			else 
			{
				y = (end.y - start.y)*(pos_min.x - start.x);
				y /= end.x - start.x;
				y += start.y;
				x = pos_min.x;
			}
			if(outcode_out == outcode_start)
			{
				start.x = x;
				start.y = y;
				outcode_start = ComputeOutCode(start,pos_min,pos_max);
			}
			else
			{
				end.x = x;
				end.y = y;
				outcode_end = ComputeOutCode(end,pos_min,pos_max);

			}
		}
	} while(!done);
	return accept;
}
/*
int ComputeOutCode(float x,float y,
				   float xmin,float ymin,
				   float xmax,float ymax)
{
	int outcode = 0;
	if(y > ymax)
		outcode |= OUTCODE_TOP;
	else if(y < ymin)
		outcode |= OUTCODE_BOTTOM;
	if(x > xmax)
		outcode |= OUTCODE_RIGHT;
	else if(x < xmin)
		outcode |= OUTCODE_LEFT;
	return outcode;
}



bool ClipLine(Vector2s& start,
			  Vector2s& end,
			  const Vector2s& pos_min,
			  const Vector2s& pos_max)
{
	float x0 = start.x;
	float y0 = start.y;
	float x1 = end.x;
	float y1 = end.y;
	float xmin = pos_min.x;
	float ymin = pos_min.y;
	float xmax = pos_max.x;
	float ymax = pos_max.y;

	int outcode_start =  ComputeOutCode(x0,y0,xmin,ymin,xmax,ymax);
	int outcode_end =  ComputeOutCode(x1,y1,xmin,ymin,xmax,ymax);
	int outcode_out;
	bool done = false;
	bool accept = false;

	do 
	{
		if(!(outcode_start|outcode_end))
		{
			done = true;
			accept = true;
		}
		else if(outcode_start&outcode_end)
		{
			done = true;
		}
		else
		{
			float x;
			float y;

			outcode_out = outcode_start ? outcode_start : outcode_end;
			if(outcode_out & OUTCODE_TOP)
			{
				x = (x1 - x0)*(ymax - y0);
				x /= y1 - y0;
				x += x0;
				y = ymax ;
			}
			else if(outcode_out & OUTCODE_BOTTOM)
			{
				x = (x1 - x0)*(ymin - y0);
				x /= y1 - y0;
				x += x0;
				y = ymin;
			}
			else if(outcode_out & OUTCODE_RIGHT)
			{
				y = (y1 - y0)*(xmax - x0);
				y /= x1 - x0;
				y += y0;
				x = xmax;
			}
			else 
			{
				y = (y1 - y0)*(xmin - x0);
				y /= x1 - x0;
				y += y0;
				x = xmin;
			}
			if(outcode_out == outcode_start)
			{
				x0 = x;
				y0 = y;
				outcode_start =  ComputeOutCode(x0,y0,xmin,ymin,xmax,ymax);
			}
			else
			{
				x1 = x;
				y1 = y;
				outcode_end =  ComputeOutCode(x1,y1,xmin,ymin,xmax,ymax);
			}
		}
	} while(!done);

	if(accept)
	{
		start.x = x0;
		start.y = y0;

		end.x = x1;
		end.y = y1;
	}
	return accept;
}
*/

void ComposeColor(pixel_type* dest,pixel_type color,int mixfactor)
{
	switch(mixfactor)
	{
		case 1:
				*dest = CLib2D::FastColorMix75_25(*dest,color);
				break;
		case 2:
				*dest = CLib2D::FastColorMix63_37(*dest,color);
				break;
		case 3:
				*dest = CLib2D::FastColorMix50_50(*dest,color);
				break;
		case 4:
				*dest = CLib2D::FastColorMix63_37(color,*dest);
				break;
		case 5:
				*dest = CLib2D::FastColorMix75_25(color,*dest);
				break;
		case 6:
				*dest = CLib2D::FastColorMix88_12(color,*dest);
				break;
		case 7:
				*dest = color;
				break;
	}
}



bool DrawAALine(Image& buffer, 
				const Vector2s& start_at_in,
				const Vector2s& end_at_in,
				const Vector2s& clipping_min,
				const Vector2s& clipping_max,
				unsigned short LineColor,
				AALINE_MODE mode)
{
	int x, y, inc; // these must be >=32 bits

	Vector2s start_at(start_at_in);
	Vector2s end_at(end_at_in);


	if(!ClipLine(start_at,end_at,clipping_min,clipping_max))
		return false;


	Vector2s delta(end_at-start_at);

	pixel_type* scan;

	if(!delta.x)
	{
		int counter;
		if(delta.y < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;

		if(mode == MIX)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = LineColor;
				scan += buffer.width();
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = CLib2D::FastColorAdd(*scan,LineColor);
				scan += buffer.width();
			}
	}
	else if(!delta.y)
	{
		int counter;
		if(delta.x < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;
		if(mode == MIX)
			for(counter = 0; counter < delta.x; counter++)
			{
				*scan++ = LineColor;
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.x; counter++)
			{
				*scan++ = CLib2D::FastColorAdd(*scan,LineColor);
			}
	}
	else if (Lib3D::Abs(delta.x) == Lib3D::Abs(delta.y)) 
	{
		int counter;
		int offset;
		if(delta.y < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;
		if(delta.x>0)
			offset = buffer.width() + 1;
		else
			offset = buffer.width() - 1;

		if(mode == MIX)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = LineColor;
				scan += offset;
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = CLib2D::FastColorAdd(*scan,LineColor);
				scan += offset;
			}
	}
	else if (Abs(delta.x) > Abs(delta.y)) 
	{
		if (delta.x < 0) 
		{
			delta = Vector2s0 - delta;
			my_swap(start_at, end_at);
		}
		x = start_at.x << 16;
		y = start_at.y << 16;
		inc = (delta.y * 65536) / delta.x;

		if(mode == MIX)
			while ((x >> 16) < end_at.x) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::MixColor16(*scan, LineColor,(y >> 12) & 0xF);
				scan += buffer.width();
				*scan = CLib2D::MixColor16(*scan, LineColor,(~y >> 12) & 0xF);

				x += (1 << 16);
				y += inc;
			}
		else if(mode == ADDITIVE)
			while ((x >> 16) < end_at.x) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::FastColorAddAlphaHD(LineColor,*scan,(~y >> 12) & 0xF);
				scan += buffer.width();
				*scan = CLib2D::FastColorAddAlphaHD(LineColor,*scan,(y >> 12) & 0xF);

				x += (1 << 16);
				y += inc;
			}
	} 
	else 
	{
		if (delta.y < 0) 
		{
			delta = Vector2s0 - delta;
			my_swap(start_at, end_at);
		}
		x = start_at.x << 16;
		y = start_at.y << 16;
		inc = (delta.x * 65536) / delta.y;

		if(mode == MIX)
			while ((y >> 16) < end_at.y) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::MixColor16(*scan, LineColor,(x >> 12) & 0xF);
				scan ++;
				*scan = CLib2D::MixColor16(*scan, LineColor,(~x >> 12) & 0xF);

				x += inc;
				y += (1 << 16);
			}
		else if(mode == ADDITIVE)
			while ((y >> 16) < end_at.y) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::FastColorAddAlphaHD(LineColor,*scan,(~x >> 12) & 0xF);
				scan ++;
				*scan = CLib2D::FastColorAddAlphaHD(LineColor,*scan,(x >> 12) & 0xF);

				x += inc;
				y += (1 << 16);
			}
	}
	return true;
}



bool DrawAALineGradient(Image& buffer, 
						const Vector2s& start_at_in,
						const Vector2s& end_at_in,
						const Vector2s& clipping_min,
						const Vector2s& clipping_max,
						unsigned short* GradientStart,
						int GradientNum,
						AALINE_MODE mode)
{
	int x, y, inc; // these must be >=32 bits

	Vector2s start_at(start_at_in);
	Vector2s end_at(end_at_in);


	if(!ClipLine(start_at,end_at,clipping_min,clipping_max))
		return false;


	Vector2s delta(end_at-start_at);

	pixel_type* scan;

	 int color_offset;
	 int color_delta;


	if(!delta.x)
	{
		int counter;
		if(!delta.y)
			return true;
		if(delta.y < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;

		color_offset = 0;
		color_delta = (GradientNum<<16)/delta.y;

		//*scan = MixColor16(*scan, LineColor,(~x >> 12) & 0x7);

		if(mode == MIX)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = GradientStart[color_offset>>16];
				scan += buffer.width();
				color_offset += color_delta;
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = CLib2D::FastColorAdd(*scan,GradientStart[color_offset>>16]);
				scan += buffer.width();
				color_offset += color_delta;
			}
	}
	else if(!delta.y)
	{
		int counter;
		if(delta.x < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;

		color_offset = 0;
		color_delta = (GradientNum<<16)/delta.x;

		if(mode == MIX)
			for(counter = 0; counter < delta.x; counter++)
			{
				*scan++ = GradientStart[color_offset>>16];
				color_offset += color_delta;
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.x; counter++)
			{
				*scan++ = CLib2D::FastColorAdd(*scan,GradientStart[color_offset>>16]);
				color_offset += color_delta;
			}
	}
	else if (Lib3D::Abs(delta.x) == Lib3D::Abs(delta.y)) 
	{
		int counter;
		int offset;

		if(delta.y < 0)
		{
			my_swap(start_at, end_at);
			delta = Vector2s0 - delta;
		}
		scan = buffer.data() + (start_at.y * buffer.width()) + start_at.x;

		color_offset = 0;
		color_delta = (GradientNum<<16)/delta.y;


		if(delta.x>0)
			offset = buffer.width() + 1;
		else
			offset = buffer.width() - 1;

		if(mode == MIX)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = GradientStart[color_offset>>16];
				scan += offset;
				color_offset += color_delta;
			}
		else if(mode == ADDITIVE)
			for(counter = 0; counter < delta.y; counter++)
			{
				*scan = CLib2D::FastColorAdd(*scan,GradientStart[color_offset>>16]);
				scan += offset;
				color_offset += color_delta;
			}
	}
	else if (Abs(delta.x) > Abs(delta.y)) 
	{
		if (delta.x < 0) 
		{
			delta = Vector2s0 - delta;
			my_swap(start_at, end_at);
		}
		x = start_at.x << 16;
		y = start_at.y << 16;
		inc = (delta.y * 65536) / delta.x;

		color_offset = 0;
		color_delta = (GradientNum<<16)/delta.x;

		if(mode == MIX)
			while ((x >> 16) < end_at.x) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::MixColor16(*scan,GradientStart[color_offset>>16],(y >> 12) & 0xF);
				scan += buffer.width();
				*scan = CLib2D::MixColor16(*scan,GradientStart[color_offset>>16],(~y >> 12) & 0xF);

				color_offset += color_delta;
				x += (1 << 16);
				y += inc;
			}
		else if(mode == ADDITIVE)
			while ((x >> 16) < end_at.x) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::FastColorAddAlphaHD(GradientStart[color_offset>>16],*scan,(~y >> 12) & 0xF);

				scan += buffer.width();
				*scan = CLib2D::FastColorAddAlphaHD(GradientStart[color_offset>>16],*scan,(y >> 12) & 0xF);

				color_offset += color_delta;
				x += (1 << 16);
				y += inc;
			}
	} 
	else 
	{
		if (delta.y < 0) 
		{
			delta = Vector2s0 - delta;	
			my_swap(start_at, end_at);
		}
		x = start_at.x << 16;
		y = start_at.y << 16;
		inc = (delta.x * 65536) / delta.y;


		color_offset = 0;
		color_delta = (GradientNum<<16)/delta.y;

		if(mode == MIX)
			while ((y >> 16) < end_at.y) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::MixColor16(*scan,GradientStart[color_offset>>16],(x >> 12) & 0xF);
				scan ++;
				*scan = CLib2D::MixColor16(*scan,GradientStart[color_offset>>16],(~x >> 12) & 0xF);

				color_offset += color_delta;
				x += inc;
				y += (1 << 16);
			}
		else if(mode == ADDITIVE)
			while ((y >> 16) < end_at.y) 
			{
				scan = buffer.data() + ((y >> 16) * buffer.width()) + (x >> 16);
				*scan = CLib2D::FastColorAddAlphaHD(GradientStart[color_offset>>16],*scan,(~x >> 12) & 0xF);
				scan ++;
				*scan = CLib2D::FastColorAddAlphaHD(GradientStart[color_offset>>16],*scan,(x >> 12) & 0xF);

				color_offset += color_delta;
				x += inc;
				y += (1 << 16);
			}
	}
	return true;
}


