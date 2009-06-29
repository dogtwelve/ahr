#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <math.h>
#include <assert.h>

#pragma warning(disable : 4996)


#define OUTPUT_LIGHTS_VERTICES
#define USE_OGL	// export object for OpenGL

#define MAX_VERTEX		4000
#define MATERIAL_MAX	100
#define SECTION_VERT	9

typedef struct	
{	
	unsigned short	nb_vert;
	unsigned short	nb_face;

} OBJ_HEADER;

typedef struct
{
#ifdef USE_OGL
	unsigned short	idx0, idx1, idx2;	// low part of idx
	unsigned short	idx;				// 2bits high part of idx + matidx (ii) |ii001122|
#else
	unsigned char	idx0, idx1, idx2;	// low part of idx
	unsigned char	idx;				// 2bits high part of idx + matidx (ii) |ii001122|
#endif // USE_OGL
	unsigned char	u0, v0, u1, v1, u2, v2;

} OBJ_FACE;

typedef struct
{
	unsigned char	u, v;

} OBJ_TV;

#define ARGUMENT_INDEX_ASE				1
#define ARGUMENT_INDEX_OBJ				2
#define ARGUMENT_INDEX_CONFIG			3
#define ARGUMENT_INDEX_CFG				4

#define OPTION_TRAFIC				(1 << 0)
#define OPTION_KEEP_VERTEX_ORDER	(1 << 1)
#define OPTION_NORMALS				(1 << 2)
#define OPTION_SIMPLE_MESH			(1 << 3)
#define OPTION_CAR					(1 << 4)

bool FileSkipToToken(FILE *fin, char *token, char *stopToken)
{
	bool bFound = false;

	char	txt[512];

	while (!feof(fin) && fscanf(fin, "%s", txt) > 0)
	{
		if (stopToken != NULL && strcmp(txt, stopToken) == 0)
			break;
		if (strcmp(txt, token) == 0)
		{
			bFound = true;
			break;
		}
	}

	return bFound;
}

void showUsage(int argc)
{
	printf("%d arguments\n", argc);
	printf("Usage : OBJEXP inputAS2filename outputOBJfilename [config.txt config.cfg] [-trafic -keepVertexOrder -normals] \n\n");
	exit(1);
}

// Car axis:
//	X - to right
//	Y - depth
//	Z - up
// Compensate flipping X and changing Y with Z


int	main( int argc, char** argv )
{
	char *fileAseName = NULL;
	char *fileObjName = NULL;
	char *fileConfigName = NULL;
	char *fileCfgName = NULL;

	if (argc <= 2)
	{
		showUsage(argc);		
	}

	fileAseName = argv[ARGUMENT_INDEX_ASE];
	fileObjName = argv[ARGUMENT_INDEX_OBJ];
	
	int options = 0;

	int k = 0;
	int c = ARGUMENT_INDEX_CONFIG;
	while (c < argc)
	{
		if (!strcmp(argv[c], "-trafic"))
			options |= OPTION_TRAFIC;
		else if (!strcmp(argv[c], "-keepVertexOrder"))
			options |= OPTION_KEEP_VERTEX_ORDER;
		else if (!strcmp(argv[c], "-normals"))
			options |= OPTION_NORMALS;
		else if (!strcmp(argv[c], "-simpleMesh"))
			options |= OPTION_SIMPLE_MESH;
		else if (!strcmp(argv[c], "-car"))
			options |= OPTION_CAR;
		else
		{
			if (k == 0)
				fileConfigName = argv[c];
			else if (k == 1)
				fileCfgName = argv[c];
			k++;
		}

		c++;
	}

	if (fileConfigName != NULL && fileCfgName == NULL)
	{
		showUsage(argc);
	}

	const int texturewidth = 256;
	//if (texturewidth > 256)
	//{
	//	printf("== FILE : %s == ", fileAseName );
	//	printf("Input texture size Error (must be <= 256)!\n\n");
	//	return -1;
	//}


	FILE*	fin = fopen( fileAseName, "rt" );

	if (fin == NULL)
	{
		printf("== FILE : %s == ", fileAseName );
		printf("Input File Error!\n");
		return -1;
	}

	// find mesh
	char	txt[512];
	int		nbv, nbf, tmp;
	txt[0] = 0;

	while (strcmp( txt, "*MESH" ) )
		fscanf( fin, "\t%s\t{\n", txt );

	fscanf( fin, "\t%s\t%d\n", txt, &tmp );
	fscanf( fin, "\t%s\t%d\n", txt, &nbv );
	fscanf( fin, "\t%s\t%d\n", txt, &nbf );
	fscanf( fin, "\t%s\t{\n", txt );


	//----------------------------------
	// VERTEX
	
	if (nbv >= MAX_VERTEX)
	{
		printf("== FILE : %s == ", fileAseName );
		printf("Input nb vertex Error (must be < %d)!\n\n", MAX_VERTEX);
		return -1;
	}

	short	*datav = (short*)malloc( 3 * 2 * nbv );
	short	*pv = datav;
	short	n = nbv;

#define BIG_INT			0x7FFFFFFF

	long minx = BIG_INT, maxx = -BIG_INT;
	long miny = BIG_INT, maxy = -BIG_INT;
	long minz = BIG_INT, maxz = -BIG_INT;

	while (n--)
	{
		float		x, y, z;
		int			idx;

		fscanf( fin, "\t*MESH_VERTEX\t%d\t%f\t%f\t%f\n", &idx, &x, &y, &z );

		long		xl, yl, zl;

		xl = (long)(x * 50 / 100);	// scale (1cm = 2cm in engine)
		yl = (long)(y * 50 / 100);
		zl = (long)(z * 50 / 100);

		if ((abs(xl)&0xFFFF0000) || (abs(yl)&0xFFFF0000) || (abs(zl)&0xFFFF0000))
		{
			printf("== FILE : %s == ", fileAseName );
			printf("Overflow -> to big !!!\n\n" );
			return -1;
		}

		if (maxx < xl)	maxx = xl;
		if (maxy < yl)	maxy = yl;
		if (maxz < zl)	maxz = zl;

		if (minx > yl)	minx = xl;
		if (miny > xl)	miny = yl;
		if (minz > zl)	minz = zl;

		if (options & OPTION_SIMPLE_MESH)
		{
			*pv++ = (short)xl;
			*pv++ = (short)zl;
			*pv++ = (short)(-yl);
		}
		else
		{
			*pv++ = (short)(-xl); // change X axis
			*pv++ = (short)zl;
			*pv++ = (short)yl;
		}
	}


	fscanf( fin, "\t}\n", txt );
	fscanf( fin, "\t%s\t{\n", txt );

	//printf("\tMIN: %d, %d, %d\n", minx, miny, minz);
	//printf("\tMAX: %d, %d, %d\n", maxx, maxy, maxz);

	//----------------------------------
	// FACES

	// to tag vert to find isolated vertices
	int		tagvert[MAX_VERTEX];
	memset( tagvert, 0, MAX_VERTEX * 4 );

	OBJ_FACE	*dataf = (OBJ_FACE*)malloc( sizeof(OBJ_FACE) * nbf );
	OBJ_FACE	*pf = dataf;
	n = nbf;

	while (n--)
	{
		char		str[256];
		int			a, b, c;
		int			idx;

		fscanf( fin, "\t*MESH_FACE\t%d:\tA:\t%d\tB:\t%d\tC:\t%d", &idx, &a, &b, &c );
		str[0] = 0;
		while (strcmp( str, "*MESH_MTLID" ))
			fscanf( fin, "%s", str );
		fscanf( fin, "\t%d", &idx );

		// save indexes (8 low bits in idx? and 2 higher bits in idx012) + material idx 
#ifdef USE_OGL
		pf->idx0 = a;
		pf->idx1 = b;
		pf->idx2 = c;
		pf->idx = idx;
#else
		pf->idx0 = a&0xFF;
		pf->idx1 = b&0xFF;
		pf->idx2 = c&0xFF;
		pf->idx012 = (((a&0xF00)>>8)<<4) | (((b&0xF00)>>8)<<2) | ((c&0xF00)>>8) | (idx<<6);
#endif // USE_OGL

		pf++;

		tagvert[ a ] = 1;
		tagvert[ b ] = 1;
		tagvert[ c ] = 1;
	}


	//----------------------------------
	// TVERT
	int		nbtv;

	fscanf( fin, "\t}\n"/*, txt*/ );
	fscanf( fin, "\t%s\t%d\n", txt, &nbtv );
	fscanf( fin, "\t%s\t{\n", txt );


	OBJ_TV	*datatv = (OBJ_TV*)malloc( sizeof(OBJ_TV) * nbtv );
	OBJ_TV	*ptv = datatv;
	n = nbtv;

	while (n--)
	{
		float		u, v, w;
		int			idx;

		fscanf( fin, "\t*MESH_TVERT\t%d\t%f\t%f\t%f\n", &idx, &u, &v, &w );

		int			ui, vi;

		ui = (int)((u * texturewidth) + 0.5f);
		if (ui < 0)
			ui = 0;
		if (ui > 255)
			ui = 255;

		vi = (int)((v * texturewidth) + 0.5f);
		if (vi > texturewidth + 0.5)
			vi -= texturewidth;

		if (vi < 0)
			vi = 0;
		if (vi > 255)
			vi = 255;

		ptv->u = (unsigned char)ui;
		ptv->v = (unsigned char)vi;

		ptv++;
	}
	
	//----------------------------------
	// TFACE
	int		nbtf;

	fscanf( fin, "\t}\n", txt );
	fscanf( fin, "\t%s\t%d\n", txt, &nbtf );
	fscanf( fin, "\t%s\t{\n", txt );

	if (nbtf != nbf)
	{
		printf("== FILE : %s == ", fileAseName );
		printf("Error : nb tface != nb face\n\n");
		return -1;
	}


	pf = dataf;
	n = nbf;

	while (n--)
	{
		int		a, b, c;
		int		idx;

		fscanf( fin, "\t*MESH_TFACE\t%d\t%d\t%d\t%d\n", &idx, &a, &b, &c );

		pf->u0 = datatv[a].u;
		pf->v0 = datatv[a].v;
		pf->u1 = datatv[b].u;
		pf->v1 = datatv[b].v;
		pf->u2 = datatv[c].u;
		pf->v2 = datatv[c].v;

		pf++;
	}

	//----------------------------------
	// NORMALS
	signed short *normals = NULL;
	int nNormals = 0;
	
	if ((options & OPTION_NORMALS) != 0)
	{
		bool bReadNormals = false;

		if (FileSkipToToken(fin, "*MESH_NORMALS", NULL))
		{
			bReadNormals = true;
		}

		if (bReadNormals)
		{
			int		k = 0;
			float	a, b, c;
			int		idx;

			nNormals = 9 * nbf;

			normals = new signed short[nNormals];
			
			while (FileSkipToToken(fin, "*MESH_VERTEXNORMAL", "}"))
			{
				int res = fscanf( fin, "%d %f %f %f\n", &idx, &a, &b, &c );

				float t = sqrt(a * a + b * b + c * c);
				if ( t != 0)
				{
					a /= t;
					b /= t;
					c /= t;
				}

				if (k < nNormals)
				{
					normals[k++] = (signed short)((a) * 4096);
					normals[k++] = (signed short)((-c) * 4096);	// Z in max
					normals[k++] = (signed short)((-b) * 4096);	// Y in max
				}
				else
					k += 3;
			}

			if (k > nNormals)
			{
				// Shouldn't happen :)
				printf("Warning: there are more than 9 * nbFaces normals (normals: %d vs 9 * faces: %d) !\n", k, nNormals);
			}
		}
	}

	fclose( fin );

	// -------------- FRONT/BACK/SIREN LIGHTS AND TIRES ISOLATED VERTICE -----------------------

	int		light_FL = 0; // front left light
	int		light_FR = 0; // front right light
	int		light_SL = 0; // siren left
	int		light_SR = 0; // siren right
	int		light_BL = 0; // back left light
	int		light_BR = 0; // back right light

	int		tire_FL = 0; // front left tire
	int		tire_FR = 0; // front right tire
	int		tire_BL = 0; // back left tire
	int		tire_BR = 0; // back right tire

	int		lights[6];

	int		lights_count = 0;
	int		tires_count = 0;

	{
		for (int i = 0; i < nbv; i++)
		{
			// if isolated vertice
			if (tagvert[i] == 0)
			{
				int y = datav[i*3+1];

				// tire if near ground
				if (y < 5)
				{
					// find which tire
					int	x = - datav[i*3]; // flip X again
					int	z = datav[i*3 + 2];

					if (x < 0)
					{
						if (z < 0)
							tire_FL = i;
						else
							tire_BL = i;
					}
					else
					{
						if (z < 0)
							tire_FR = i;
						else
							tire_BR = i;
					}

					// rax - tire above the ground
					datav[i*3 + 1] = 1;

					tires_count++;
				}
				else
				{
					// store lights vertecies here, decide what vertex what light is later
					if (lights_count >= 6)
					{
						printf ("Error: there are more than 6 lights verices!\n");
					}
					else
					{
						lights[lights_count++] = i;
					}
				}
			}
		}

		// sort by z so we can deside if this is front,siren or stop light
		for (int i = 0; i < lights_count - 1; i++)
		{
			for (int j = i+1; j < lights_count; j++)
			{
				if (datav[lights[i]*3+2] > datav[lights[j]*3+2])
				{
					int temp = lights[i];
					lights[i] = lights[j];
					lights[j] = temp;
				}
			}
		}

		if (lights_count == 2)
		{
			light_FL = lights[0];
			light_BL = lights[1];
		}
		else if (lights_count == 3)
		{
			light_FL = lights[0];
			light_SL = lights[1];
			light_BL = lights[2];
		}
		else if (lights_count == 4)
		{
			if (datav[lights[0]*3] < 0)
			{
				light_FL = lights[0];
				light_FR = lights[1];
			}
			else
			{
				light_FL = lights[1];
				light_FR = lights[0];
			}

			if (datav[lights[2]*3] < 0)
			{
				light_BL = lights[2];
				light_BR = lights[3];
			}
			else
			{
				light_BL = lights[3];
				light_BR = lights[2];
			}
		}
		// 2 headlights, 2 stops and 1 sireen
		else if (lights_count == 5)
		{
			if (datav[lights[0]*3] < 0)
			{
				light_FL = lights[0];
				light_FR = lights[1];
			}
			else
			{
				light_FL = lights[1];
				light_FR = lights[0];
			}

			light_SL = lights[2];
			light_SR = lights[2];

			if (datav[lights[3]*3] < 0)
			{
				light_BL = lights[3];
				light_BR = lights[4];
			}
			else
			{
				light_BL = lights[4];
				light_BR = lights[3];
			}
		}
		else if (lights_count == 6)
		{
			if (datav[lights[0]*3] < 0)
			{
				light_FL = lights[0];
				light_FR = lights[1];
			}
			else
			{
				light_FL = lights[1];
				light_FR = lights[0];
			}

			if (datav[lights[2]*3] < 0)
			{
				light_SL = lights[2];
				light_SR = lights[3];
			}
			else
			{
				light_SL = lights[3];
				light_SR = lights[2];
			}

			if (datav[lights[4]*3] < 0)
			{
				light_BL = lights[4];
				light_BR = lights[5];
			}
			else
			{
				light_BL = lights[5];
				light_BR = lights[4];
			}
		}
	}

#ifdef OUTPUT_LIGHTS_VERTICES
	if ((options & OPTION_SIMPLE_MESH) == 0)
	{
		printf("\tLights: %d\n", lights_count);
		printf("\t\tFront %d (%d, %d, %d), %d (%d, %d, %d)\n", light_FL, datav[light_FL*3+0], datav[light_FL*3+1], datav[light_FL*3+2],
			light_FR, datav[light_FR*3+0], datav[light_FR*3+1], datav[light_FR*3+2]);
		printf("\t\tBack  %d (%d, %d, %d), %d (%d, %d, %d)\n", light_BL, datav[light_BL*3+0], datav[light_BL*3+1], datav[light_BL*3+2],
			light_BR, datav[light_BR*3+0], datav[light_BR*3+1], datav[light_BR*3+2]);

		printf("\t\tSiren %d (%d, %d, %d), %d (%d, %d, %d)\n", light_SL, datav[light_SL*3+0], datav[light_SL*3+1], datav[light_SL*3+2],
			light_SR, datav[light_SR*3+0], datav[light_SR*3+1], datav[light_SR*3+2]);

		printf("\tTires:\n");
		printf("\t\tFront %d (%d, %d, %d), %d (%d, %d, %d)\n", tire_FL, datav[tire_FL*3+0], datav[tire_FL*3+1], datav[tire_FL*3+2],
			tire_FR, datav[tire_FR*3+0], datav[tire_FR*3+1], datav[tire_FR*3+2]);
		printf("\t\tBack  %d (%d, %d, %d), %d (%d, %d, %d)\n", tire_BL, datav[tire_BL*3+0], datav[tire_BL*3+1], datav[tire_BL*3+2],
			tire_BR, datav[tire_BR*3+0], datav[tire_BR*3+1], datav[tire_BR*3+2]);
	}
#endif // OUTPUT_LIGHTS_VERTICES

	//===================================


#ifdef USE_OGL

	// Create lights, tires vectors

	short vLights[6 * 3];
	memset(vLights, 0, sizeof(vLights));

	short *v = vLights;
	*v++ = datav[light_FL * 3 + 0];
	*v++ = datav[light_FL * 3 + 1];
	*v++ = datav[light_FL * 3 + 2];

	if (light_FR > 0)
	{
		*v++ = datav[light_FR * 3 + 0];
		*v++ = datav[light_FR * 3 + 1];
		*v++ = datav[light_FR * 3 + 2];
	}
	else v += 3;

	*v++ = datav[light_BL * 3 + 0];
	*v++ = datav[light_BL * 3 + 1];
	*v++ = datav[light_BL * 3 + 2];

	if (light_BR > 0)
	{
		*v++ = datav[light_BR * 3 + 0];
		*v++ = datav[light_BR * 3 + 1];
		*v++ = datav[light_BR * 3 + 2];
	}
	else v += 3;

	if (light_SL > 0)
	{
		*v++ = datav[light_SL * 3 + 0];
		*v++ = datav[light_SL * 3 + 1];
		*v++ = datav[light_SL * 3 + 2];
	}
	else v += 3;

	if (light_SR > 0)
	{
		*v++ = datav[light_SR * 3 + 0];
		*v++ = datav[light_SR * 3 + 1];
		*v++ = datav[light_SR * 3 + 2];
	}
	else v += 3;

	short vTires[4 * 3];
	v = vTires;

	*v++ = datav[tire_FL * 3 + 0];
	*v++ = datav[tire_FL * 3 + 1];
	*v++ = datav[tire_FL * 3 + 2];

	if (tire_FR > 0)
	{
		*v++ = datav[tire_FR * 3 + 0];
		*v++ = datav[tire_FR * 3 + 1];
		*v++ = datav[tire_FR * 3 + 2];
	}
	else v += 3;

	*v++ = datav[tire_BL * 3 + 0];
	*v++ = datav[tire_BL * 3 + 1];
	*v++ = datav[tire_BL * 3 + 2];

	if (tire_BR > 0)
	{
		*v++ = datav[tire_BR * 3 + 0];
		*v++ = datav[tire_BR * 3 + 1];
		*v++ = datav[tire_BR * 3 + 2];
	}
	else v += 3;

	OBJ_FACE* df = dataf;
	short* dv = datav;
	short* vA[3];
	short* nA[3];
	unsigned char* tA[3];	

	int nVertex = 0;
	int pIndicesNum[MATERIAL_MAX];

	memset(pIndicesNum, 0, sizeof(pIndicesNum));

	short *pVertex = new short[nbf * 3 * 3];	
	unsigned char *pTex = new unsigned char[nbf * 3 * 2];
	unsigned short *pIndices[MATERIAL_MAX];

	for (int i=0; i<MATERIAL_MAX; ++i)
		pIndices[i] = new unsigned short[nbf * 3];

	short *pNormals = NULL;
	if (normals > 0)
		pNormals = new short[nbf * 3 * 3];

	int nMaterials = 1;	

	for (int i=0; i<nbf; ++i)
	{
		const int a = df->idx0;// | (((df[3]>>4)&0x3)<<8);
		const int b = df->idx1;//| (((df[3]>>2)&0x3)<<8);
		const int c = df->idx2;// | (((df[3])&0x3)<<8);

		const int matId = df->idx;// >> 6;

		assert(matId < MATERIAL_MAX);
		if (matId + 1 > nMaterials)
			nMaterials = matId + 1;

		int &nIndices = pIndicesNum[matId];

		vA[0] = dv + 3 * a;
		vA[1] = dv + 3 * b;
		vA[2] = dv + 3 * c;

		tA[0] = &df->u0;
		tA[1] = &df->u1;
		tA[2] = &df->u2;

		if (normals)
		{
			nA[0] = normals + (i * 9);
			nA[1] = normals + (i * 9 + 3);
			nA[2] = normals + (i * 9 + 6);
		}

		bool bCheckNormals = normals &&
			// If car, do not check normals for anvelope (needed for wheel rotation)
			((options & OPTION_CAR) == 0 || matId != 3);

		for (int k=0; k<3; k++)
		{
			int index = -1;
			short *v = pVertex;
			short *norm = pNormals;
			unsigned char *t = pTex;

			if ((options & OPTION_KEEP_VERTEX_ORDER) == 0)
			{
				// check if pair (vA[k], tA[k]) is unique				
				for (int l=0; l<nVertex; ++l)
				{
					if (vA[k][0] == v[0] &&
						vA[k][1] == v[1] &&
						vA[k][2] == v[2] &&
						tA[k][0] == t[0] &&
						tA[k][1] == t[1])
					{
						if (bCheckNormals)
						{
							if (nA[k][0] == norm[0] &&
								nA[k][1] == norm[1] && 
								nA[k][2] == norm[2])
							{
								index = l;
								break;
							}
						}
						else
						{
							index = l;
							break;
						}
					}

					v += 3;
					t += 2;
					norm += 3;
				}
			}

			if (index >= 0)
			{
				pIndices[matId][nIndices++] = index;
			}
			else
			{
				// add vA, tA to vertex, tex
				index = nVertex;
				v = pVertex + index * 3;
				t = pTex + index * 2;
				norm = pNormals + index * 3;
				
				v[0] = vA[k][0];
				v[1] = vA[k][1];
				v[2] = vA[k][2];

				t[0] = tA[k][0];
				t[1] = tA[k][1];

				if (pNormals)
				{
					norm[0] = nA[k][0];
					norm[1] = nA[k][1];
					norm[2] = nA[k][2];
				}

				nVertex++;
				
				pIndices[matId][nIndices++] = index;
			}
		}

		df++;		
	}
	
	// Write file

	FILE*	fout = fopen( fileObjName, "wb" );
	if (fout == NULL)
	{
		printf("== FILE : %s == ", fileObjName );
		printf("Error opening output file\n\n");
		return -1;
	}

	unsigned char idx = nMaterials;
	fwrite( &idx, 1, 1, fout );
	fwrite( &nVertex, 4, 1, fout );	
	fwrite( pVertex, nVertex * 3 * 2, 1, fout );
	fwrite( pTex, nVertex * 2, 1, fout );
	
	if (pNormals)
	{
		idx = 1;
		fwrite( &idx, 1, 1, fout );
		fwrite( pNormals, nVertex * 3 * 2, 1, fout );
	}
	else
	{
		idx = 0;
		fwrite( &idx, 1, 1, fout );
	}

	int nIndices = 0;

	for (int i=0; i<nMaterials; ++i)
	{
		fwrite( &pIndicesNum[i], 4, 1, fout );

		if (pIndicesNum[i] > 0)
			fwrite( pIndices[i], pIndicesNum[i] * 2, 1, fout);		

		nIndices += pIndicesNum[i];
	}

	fwrite(vLights, 6 * 3 * 2, 1, fout);
	fwrite(vTires, 4 * 3 * 2, 1, fout);

	printf("Indices: %d, vertices: %d\n", nIndices, nVertex);

	delete[] pVertex;
	delete[] pTex;

	if (pNormals)
		delete[] pNormals;

	for (int i=0; i<MATERIAL_MAX; ++i)
		delete[] pIndices[i];


#else // USE_OGL

	//===================================

	OBJ_HEADER	header;

	header.nb_vert = nbv;
	header.nb_face = nbf;

	FILE*	fout = fopen( fileObjName, "wb" );
	if (fout == NULL)
	{
		printf("== FILE : %s == ", fileObjName );
		printf("Error opening output file\n\n");
		return -1;
	}

	fwrite( &header.nb_vert, 2, 1, fout );
	fwrite( &header.nb_face, 2, 1, fout );
	fwrite( datav, nbv*3*2, 1, fout );
	fwrite( dataf, nbf*sizeof(OBJ_FACE), 1, fout );

	// save lights data
	fwrite( &lights_count, 1, 1, fout );
	if (lights_count)
	{
		unsigned short tmp_arr[6];
		tmp_arr[0] = light_FL;
		tmp_arr[1] = light_FR;
		tmp_arr[2] = light_BL;
		tmp_arr[3] = light_BR;
		tmp_arr[4] = light_SL;
		tmp_arr[5] = light_SR;

		fwrite( tmp_arr, 6, 2, fout );
	}

	// save tires data
	fwrite( &tires_count, 1, 1, fout );
	if (tires_count)
	{
		unsigned short tmp_arr[4];
		tmp_arr[0] = tire_FL;
		tmp_arr[1] = tire_FR;
		tmp_arr[2] = tire_BL;
		tmp_arr[3] = tire_BR;

		fwrite( tmp_arr, 4, 2, fout );
	}

#endif // USE_OGL

	fclose(fout);

	free( datatv );
	free( datav );
	free( dataf );

	// ===============================================
	// CONFIG
	// ===============================================

	if (fileConfigName != NULL && (options & OPTION_TRAFIC) == 0)
	{
		// cars

		fin = fopen( fileConfigName, "rt" );

		if (fin == NULL)
		{
			printf("== FILE : %s == ", fileConfigName );
			printf("Input File Error!\n\n");
			return -1;
		}


		txt[0] = 0;

		// find config
		while (strcmp( txt, "[ENGINE]" ) )
			fscanf( fin, "%s\n", txt );
		
		int		gear_nb = 0;
		int		gear[10][5];
		while (1)
		{
			fscanf( fin, "%s\n", txt );
			if (strcmp( txt, "[END]" ) == 0)
				break;
			// get gear config
			sscanf_s( txt, "%d", &gear[gear_nb][0] );

			float f;
			fscanf( fin, "%f %d %d %d\n", &f, &gear[gear_nb][2], &gear[gear_nb][3], &gear[gear_nb][4] );

			gear[gear_nb][1] = (int)(f * 100);

			gear_nb++;
		}

		// find SIZE
		while (strcmp( txt, "[SIZE]" ) )
			fscanf( fin, "%s\n", txt );
		int	size[3];
		fscanf( fin, "%d %d %d\n", &size[0], &size[1], &size[2] );

		// find BREAK
		while (strcmp( txt, "[BREAK]" ) )
			fscanf( fin, "%s\n", txt );
		int	breakforce[2];
		fscanf( fin, "%d %d\n", &breakforce[0], &breakforce[1] );

		// find SLIDING
		while (strcmp( txt, "[SLIDING]" ) )
			fscanf( fin, "%s\n", txt );
		int	sliding[4];
		fscanf( fin, "%d %d %d %d\n", &sliding[0], &sliding[1], &sliding[2], &sliding[3] );

		// find DIRECTION
		while (strcmp( txt, "[DIRECTION]" ) )
			fscanf( fin, "%s\n", txt );
		int	direction[2];
		float	f[2];
		fscanf( fin, "%f %f\n", &f[0], &f[1] );
		direction[0] = (int)(f[0] * 100 );
		direction[1] = (int)(f[1] * 100 );

		// find BREAKLIGHT
/*		while (strcmp( txt, "[BREAKLIGHT]" ) )
			fscanf( fin, "%s\n", txt );
		int	breaklight[8];
		fscanf( fin, "%d %d %d %d %d %d %d %d\n", &breaklight[0], &breaklight[1], &breaklight[2], &breaklight[3], &breaklight[4], &breaklight[5], &breaklight[6], &breaklight[7] );
*/

/*		while (strcmp( txt, "[BREAKLIGHT]" ) )
			fscanf( fin, "%s\n", txt );
		
		int		breaklight_nb = 0;
		int		breaklight[16][8];
		while (1)
		{
			fscanf( fin, "%s\n", txt );
			if (strcmp( txt, "[END]" ) == 0)
				break;
			sscanf( txt, "%d", &breaklight[breaklight_nb][0] );

			fscanf( fin, "%d %d %d %d %d %d %d\n",	
					&breaklight[breaklight_nb][1], &breaklight[breaklight_nb][2],
					&breaklight[breaklight_nb][3], &breaklight[breaklight_nb][4],
					&breaklight[breaklight_nb][5], &breaklight[breaklight_nb][6],
					&breaklight[breaklight_nb][7] );

			breaklight_nb++;
		}*/


		// find TEXTURES
		while (strcmp( txt, "[TEXTURES]" ) )
			fscanf( fin, "%s\n", txt );

		int		text_nb = 0;
		char	text[128][128];
		while (1)
		{
			fscanf( fin, "%s\n", text[text_nb] );
			if (strcmp( text[text_nb], "[END]" ) == 0)
				break;
			text_nb++;
		}

		// find MESHES
		while (strcmp( txt, "[MESHES]" ) )
			fscanf( fin, "%s\n", txt );

		int		mesh_nb = 0;
		char	mesh[128][128];
		int		mesh_mirror[128];
		while (1)
		{
			fscanf( fin, "%s", mesh[mesh_nb] );
			if (strcmp( mesh[mesh_nb], "[END]" ) == 0)
				break;
			fscanf( fin, "%d\n", &mesh_mirror[mesh_nb] );
			mesh_nb++;
		}

		// find SUSPENSION
		while (strcmp( txt, "[SUSPENSION]" ) )
			fscanf( fin, "%s\n", txt );
		int	suspension[4];
		fscanf( fin, "%d %d %d %d\n", &suspension[0], &suspension[1], &suspension[2], &suspension[3] );

		// find CAMERA
		while (strcmp( txt, "[CAMERA]" ) )
			fscanf( fin, "%s\n", txt );
		int	camera[9];
		fscanf( fin, "%d %d %d\n", &camera[0], &camera[1], &camera[2] );
		fscanf( fin, "%d %d %d\n", &camera[3], &camera[4], &camera[5] );
		fscanf( fin, "%d %d %d\n", &camera[6], &camera[7], &camera[8] );

		// find JUMP
		while (strcmp( txt, "[JUMP]" ) )
			fscanf( fin, "%s\n", txt );
		int	jump[2];
		fscanf( fin, "%d %d\n", &jump[0], &jump[1] );

		// find CLUTCH
		while (strcmp( txt, "[CLUTCH]" ) )
			fscanf( fin, "%s\n", txt );
		int	clutch[2];
		fscanf( fin, "%d %d\n", &clutch[0], &clutch[1] );

		// find SPEED_BOOST if any
		bool missing = false;
		while (strcmp( txt, "[SPEED_BOOST]" ) )
		{
			if (fscanf( fin, "%s\n", txt ) == EOF)
			{
				missing = true;
				break;
			}
		}

		int speed_boost_modifier;
		if (missing)
			speed_boost_modifier = 255;
		else
			fscanf( fin, "%d\n", &speed_boost_modifier);

		fclose( fin );

		fout = fopen( fileCfgName, "wb" );
		if (fout == NULL)
		{
			printf("== FILE : %s == ", fileCfgName );
			printf("Error opening output file\n\n");
			return -1;
		}

		unsigned char	idx;

		idx = gear_nb;
		fwrite( &idx, 1, 1, fout );
		for (int i = 0; i < gear_nb; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				unsigned short	s;
				s = gear[i][j];
				fwrite( &s, 2, 1, fout );
			}
		}

		fwrite( &speed_boost_modifier, 2, 1, fout );

		for (int i = 0; i < 3; i++)
		{
			unsigned short	s;
			s = size[i];
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 2; i++)
		{
			unsigned char	c;
			c = breakforce[i];
			fwrite( &c, 1, 1, fout );
		}

		for (int i = 0; i < 4; i++)
		{
			unsigned short	s;
			s = sliding[i];
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 2; i++)
		{
			unsigned short	s;
			s = direction[i];
			fwrite( &s, 2, 1, fout );
		}


/*		for ( i = 0; i < 8; i++)
		{
			unsigned short	s;
			s = breaklight[i];
			fwrite( &s, 2, 1, fout );
		}*/

/*		idx = breaklight_nb;
		fwrite( &idx, 1, 1, fout );
		for (i = 0; i < breaklight_nb; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				unsigned short	s;
				s = breaklight[i][j];
				fwrite( &s, 2, 1, fout );
			}
		}*/

		idx = text_nb;
		fwrite( &idx, 1, 1, fout );
		for (int i = 0; i < text_nb; i++)
		{
			idx = strlen( text[i] );
			fwrite( &idx, 1, 1, fout );
			fwrite( text[i], idx, 1, fout );
		}

		idx = mesh_nb;
		fwrite( &idx, 1, 1, fout );
		for (int i = 0; i < mesh_nb; i++)
		{
			idx = strlen( mesh[i] );
			fwrite( &idx, 1, 1, fout );
			fwrite( mesh[i], idx, 1, fout );
			idx = mesh_mirror[i];
			fwrite( &idx, 1, 1, fout );
		}

		for (int i = 0; i < 4; i++)
		{
			unsigned short	s;
			s = suspension[i];
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 9; i++)
		{
			signed short	s;
			s = camera[i];
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 2; i++)
		{
			unsigned short	s;
			s = jump[i];
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 2; i++)
		{
			unsigned char	c;
			c = clutch[i];
			fwrite( &c, 1, 1, fout );
		}

		fclose( fout );
	}
	else if ((options & OPTION_TRAFIC) != 0)
	{
		// trafic cars

		fin = fopen( fileConfigName, "rt" );

		if (fin == NULL)
		{
			printf("== FILE : %s == ", fileConfigName );
			printf("Input File Error!\n\n");
			return -1;
		}


		txt[0] = 0;

		// find TYPE
		while (strcmp( txt, "[TYPE]" ) )
			fscanf( fin, "%s\n", txt );
		int	type;
		fscanf( fin, "%d\n", &type );

		// find SIZE
		while (strcmp( txt, "[SIZE]" ) )
			fscanf( fin, "%s\n", txt );
		int	size[3];
		fscanf( fin, "%d %d %d\n", &size[0], &size[1], &size[2] );

		// find BREAKLIGHT
/*		while (strcmp( txt, "[BREAKLIGHT]" ) )
			fscanf( fin, "%s\n", txt );
		int	breaklight[8];
		fscanf( fin, "%d %d %d %d %d %d %d %d\n", &breaklight[0], &breaklight[1], &breaklight[2], &breaklight[3], &breaklight[4], &breaklight[5], &breaklight[6], &breaklight[7] );
*/

		// find TEXTURES
		while (strcmp( txt, "[TEXTURES]" ) )
			fscanf( fin, "%s\n", txt );

		int		text_nb = 0;
		char	text[128][128];
		while (1)
		{
			fscanf( fin, "%s\n", text[text_nb] );
			if (strcmp( text[text_nb], "[END]" ) == 0)
				break;
			text_nb++;
		}

		fclose( fin );

		fout = fopen( fileCfgName, "wb" );
		if (fout == NULL)
		{
			printf("== FILE : %s == ", fileCfgName );
			printf("Error opening output file\n\n");
			return -1;
		}

		unsigned char	idx;

		{
			unsigned short	s;
			s = type;
			fwrite( &s, 2, 1, fout );
		}

		for (int i = 0; i < 3; i++)
		{
			unsigned short	s;
			s = size[i];
			fwrite( &s, 2, 1, fout );
		}

/*		for ( i = 0; i < 8; i++)
		{
			unsigned short	s;
			s = breaklight[i];
			fwrite( &s, 2, 1, fout );
		}*/

		idx = text_nb;
		fwrite( &idx, 1, 1, fout );
		for (int i = 0; i < text_nb; i++)
		{
			idx = strlen( text[i] );
			fwrite( &idx, 1, 1, fout );
			fwrite( text[i], idx, 1, fout );
		}


		fclose( fout );
	}

	return 0;
}