
#include "CarExporter.h" 


#include "UnifyReader.h"
#include <fstream>
#include "TgaReader.h"
#include "TextureWriter.h"
#include "Surface.h"

void CCarExporter::Export(const std::string& iFile, const std::string& oFile){
	Unify::CUnifyReader reader;
	Unify::CParameter * param;

	std::string inName=m_InPath+iFile;
	std::string outName=m_OutPath+oFile;
	std::string carName;
	Unify::CNode* node = reader.Read(inName.c_str());

	if(!node){
		const Unify::CUnifyException& e = reader.GetException();
		printf( "Error: Unify exception<%s> on line<%u> in file<%s>\n", e.Description.c_str(), e.Line, iFile.c_str() );
		return;
	}
	std::ofstream out(outName.c_str(),std::ios::out|std::ios::binary);

	
	// Name
	param = node->GetParameter("name");

	if(param){
		carName=param->GetString();
	}else{
		printf("Warning: No name specified for car in file<%s>\n",iFile.c_str());
		carName=iFile;
	}
	// Wheels
	Unify::CNode* wheels = node->GetChildByName("wheels"); 
	if(wheels){

		// Write number of wheels
		int integer = wheels->GetChildCount();
		out.write((char*)&integer,sizeof(int));
		
		
		for(int w=0;w<wheels->GetChildCount();w++){
			Unify::CNode* wheel = wheels->GetChild(w);

			std::string stop;


			
			// Stopped
			param = wheel->GetParameter("stop");  
			if(param){
				stop=param->GetString();
			}else{
				printf("Warning: No texture specified for car<%s>.wheel<%u>.stop\n",carName.c_str(),w+1);
				stop="";
			}
			
			// + Write Stopped
			WriteString(out,stop);   
			
			// + Export the texture
			//if(!ExportTexture(stop,stop, ELayerType_Overlay)){
			//	printf("Warning: Texture export failed for car<%s>.wheel<%u>.stop\n",carName.c_str(),w+1);
			//}



			// Fast
			param = wheel->GetParameter("fast");
			for(int f=0;f<2;f++){
				std::string fast="";

				if(param && param->GetValue(f)){
					fast=param->GetValue(f)->GetString();
				}else{
					printf("Warning: No texture specified for car<%s>.wheel<%u>.fast[%u]\n",carName.c_str(),w+1,f+1);
					fast="";
				}

				// + Write Fast
				WriteString(out,fast);
				
				// + Export the texture
				//if(!ExportTexture(fast,fast, ELayerType_Overlay)){
				//	printf("Warning: Texture export failed for car<%s>.wheel<%u>.fast[%u]\n",carName.c_str(),w+1,f+1);
				//}
			}
		
		}

	}else{
		printf("Error: No wheels in car<%s>\n",carName.c_str());
		int integer=0;
		out.write((char*)&integer,sizeof(int));
	}

	// Bodies
	Unify::CNode* bodies = node->GetChildByName("bodies");

	if(bodies){
		// Write number of bodies
		int integer = bodies->GetChildCount();
		out.write((char*)&integer,sizeof(int));
		
		for(int b=0;b<bodies->GetChildCount();b++){
			Unify::CNode* body = bodies->GetChild(b);

			std::string lightmap;
			std::string accessories;
			std::string mesh;

			
			// Lightmap
			param = body->GetParameter("lightmap");
			if(param){
				lightmap=param->GetString();
			}else{
				printf("Warning: No lightmap specified for car<%s>.body<%u>\n",carName.c_str(),b+1);
				lightmap="";
			}
			
			// + Write lighting
			WriteString(out,lightmap);
			
			// + Export the texture
			if(!ExportTexture(lightmap,lightmap, ELayerType_Overlay)){
				printf("Warning: Lightmap export failed for car<%s>.body<%u>\n",carName.c_str(),b+1);
			}




			// Accessories
			param = body->GetParameter("accessories");
			if(param){
				accessories=param->GetString();
			}else{
				printf("Warning: No accessories specified for car<%s>.body<%u>\n",carName.c_str(),b+1);
				accessories="";
			}
	
			// + Write accessories
			WriteString(out,accessories);
			
			// + Add to texture exporte list
			if(!ExportTexture(accessories,accessories, ELayerType_Normal)){
				printf("Warning: Accessories export failed for car<%s>.body<%u>\n",carName.c_str(),b+1);
			}

			


			// mesh
			param = body->GetParameter("mesh");
			if(param){
				mesh=param->GetString();
			}else{
				printf("Warning: No mesh specified for car<%s>.body<%u>\n",carName.c_str(),b+1);
				mesh="";
			}
	
			// + Write accessories
			WriteString(out,mesh);
			


			// Vinyls
			// Write number of vinyls
			integer = body->GetChildCount();
			out.write((char*)&integer,sizeof(int));

			for(int v=0;v<body->GetChildCount();v++){
				Unify::CNode* vinyl = body->GetChild(v);

				// Layers

				// Write number of layers
				integer = vinyl->GetChildCount();
				out.write((char*)&integer,sizeof(int));

				for(int l=0;l<vinyl->GetChildCount();l++){
					Unify::CNode* layer = vinyl->GetChild(l);
					
					// Type
					std::string type;
					param = layer->GetParameter("type");
					if(param){
						type=param->GetString();
					}else{
						printf("Warning: No type specified for car<%s>.body<%u>.vinyl<%u>.layer<%u>\n",carName.c_str(),b+1,v+1,l+1);
						type="normal";
					}

					// + Write type
					ELayerType t = GetLayerTypeFromName(type);
					out.write((char*)&t,sizeof(t));


					// File
					std::string file;
					param = layer->GetParameter("texture");
					if(param){
						file=param->GetString();
					}else{
						printf("Warning: No file specified for car<%s>.body<%u>.vinyl<%u>.layer<%u>\n",carName.c_str(),b+1,v+1,l+1);
					}

					// + Write file
					WriteString(out,file);

					// + Add to texture export list
					if(!ExportTexture(file,file,t)){
						printf("Warning: Layer export failed for car<%s>.body<%u>.vinyl<%u>.layer<%u>\n",carName.c_str(),b+1,v+1,l+1);
					}

					
					
				}
			}
			
		}

	}else{
		printf("Error: No body in car<%s>\n",carName.c_str());
		int integer=0;
		out.write((char*)&integer,sizeof(int));
	}

}



void CCarExporter::WriteString(std::ofstream& file, const std::string& str)
{
	int len=strlen(str.c_str());
	if(len>255)len=255;
	char lenc=len;
	file.write(&lenc,sizeof(char));
	file.write(str.c_str(),(char)len);
}

CCarExporter::ELayerType CCarExporter::GetLayerTypeFromName(const std::string& str)
{
	if(str=="normal"){
		return ELayerType_Normal;
	}else if(str=="mask"){
		return ELayerType_Mask;
	}else if(str=="overlay"){
		return ELayerType_Overlay;
	}else if(str=="set"){
		return ELayerType_Set;
	}else{
		return ELayerType_Unknown;
	}
}


bool CCarExporter::ExportTexture(const std::string& iFile, const std::string& oFile, ELayerType type)
{
	CTgaReader reader;
	CTextureWriter writer;
	CSurface surface;

	std::string inFile= m_InPath+iFile+".tga";
	std::string outFile= m_OutPath+oFile+".tga";

	if(!reader.Read(inFile,surface)){
		printf("Warning: Texture export failed, read error on file<%s>\n",inFile.c_str());
		return false;
	}

	// TODO : Check format consistency
	CSurface::EFormat format= surface.GetFormat();
	if(type==ELayerType_Normal && format==CSurface::EFormat_A8R8G8B8){
	}else if((type==ELayerType_Overlay || type==ELayerType_Mask) && format==CSurface::EFormat_S8){
	}else{
		printf("Warning: Pixel format in file<%s> doesn't match car definition\n",oFile.c_str());
	}

	if(!writer.Write(outFile,surface)){
		printf("Warning: Texture export failed, write error on file<%s>\n",outFile.c_str());
		return false;
	}

	return true;
}
