
inline void BlitFX(Lib3D::CLib3D& lib3d, unsigned short *in_ScreenPtr, TTexture* text, int y, int x )
{
	// TRANSPARENCY FOR CAR LIGHTS
	unsigned long	*dest=(unsigned long *)(in_ScreenPtr+ ((kInterfaceHeigth*lib3d.Width())>>1));
	unsigned long	*src=(unsigned long*)lib3d.GetImageBuffer();
	int				i = ((lib3d.Width()*lib3d.Height()));

	int				l1 = 92 + y;
	int				l2 = lib3d.Height() - l1;
	int				dec = l1*lib3d.Width() / 2;

	// top 2D
	memcpy( in_ScreenPtr, lib3d.GetImageBufferFull(), lib3d.m_dispX*(lib3d.m_dispY - lib3d.Height()) );

	// top 3D
	memcpy( dest, src, dec << 2);

	unsigned long	*ptr = (unsigned long*)text->Data();
	
	dest += dec;
	src += dec;

	// 32 bits
	while (l2--)
	{
		int s = lib3d.Width()/2;
		//unsigned long*	ptr2 = ptr + (x + ((256-176)/2)) / 2;
		unsigned long*	ptr2 = ptr + (x + ((256-240)/2)) / 2;
		while (s--)
		{
#ifdef __BREW__
			*dest++ = CLib2D::FastColorAdd( (unsigned short)(*ptr2), (unsigned short)(*src) ) | (CLib2D::FastColorAdd( (unsigned short)((*ptr2) >> 16), (unsigned short)((*src) >> 16 )) << 16);
			ptr2++;
			src++;
#else
			*dest++ = CLib2D::FastColorAdd2Pixel( *ptr2++, *src++ );
#endif
		}
		ptr += 256/2;
	}

	// bottom 2D
	int	offset = (lib3d.m_dispX*lib3d.m_dispY) - lib3d.m_dispX*(lib3d.m_dispY - lib3d.Height())/2;
	memcpy( in_ScreenPtr + offset, 
			lib3d.GetImageBufferFull() + offset, 
			lib3d.m_dispX*(lib3d.m_dispY - lib3d.Height()) );
}


inline void Blit(Lib3D::CLib3D& lib3d, unsigned short *in_ScreenPtr)
{
	memcpy( in_ScreenPtr, lib3d.GetImageBufferFull(), (lib3d.m_dispX*lib3d.m_dispY*2) );
}
