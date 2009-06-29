import java.io.*;
import java.util.*;

import java.awt.*;
import java.awt.image.*;

import javax.swing.ImageIcon;

public class AuroraXP {
	static void writeShort(BufferedOutputStream stream, int valueToWrite) throws IOException {
		stream.write(valueToWrite);
		stream.write(valueToWrite >> 8);
	}

	static short readShort(BufferedInputStream stream) throws IOException {
        return (short) ((stream.read() & 0xFF) | ((stream.read() & 0xFF) << 8));
	}

	static void writeImage(BufferedOutputStream stream, AlphaImage image, int x, int y, int w, int h) throws Exception {
		writeShort(stream, w);
		writeShort(stream, h);

    byte typeMask = 0;

    if(image.indexedImage) {
      typeMask |= 0x01;
      if(image.lowColorImage) {
        typeMask |= 0x04;
      }
      //System.out.println("IND!");
    }

    // Check for alpha for this part of the image
    for(int yy=0; (yy<h) && ((typeMask & 0x02) != 0x02); yy++) {
        for(int xx=0; (xx<w) && ((typeMask & 0x02) != 0x02); xx++) {
    			if(image.alpha[(y+yy)*image.width+(x+xx)] != 255) {
    			   typeMask |= 0x02;
    			}
        }
    }

    //writeShort(stream, 0xBEEF);
    stream.write(typeMask);
    //writeShort(stream, 0xC0DE);

		// Writing the pixels
	if(image.lowColorImage) {
	//System.out.println("---IMAGE---");
        writeCompactArray(stream, image.pixels, x, y, w, h, image.width, false);
	} else {
        for(int yy=0; yy<h; yy++) {
            for(int xx=0; xx<w; xx++) {
                if(image.indexedImage) {
        			     stream.write(image.pixels[(y+yy)*image.width+(x+xx)]);
        	    } else {
        			     writeShort(stream, image.pixels[(y+yy)*image.width+(x+xx)]);
                }    			  
      		}
        }
    }

    //writeShort(stream, 0xDEAD);
            
    if((typeMask & 0x02) == 0x02) {
	//System.out.println("---ALPHA---");
  		writeCompactArray(stream, image.alpha, x, y, w, h, image.width, true);
    }
	}

    static void writeCompactArray(BufferedOutputStream stream, int[] buffer, int x, int y, int w, int h, int srcW, boolean highBits)
        throws Exception {
        int compact[] = new int[w*h];
        
        for(int yy=0; yy<h; yy++) {
            for(int xx=0; xx<w; xx++) {
                compact[yy*w+xx] = buffer[(y+yy)*srcW+(x+xx)];           
                //System.out.println(Integer.toString(compact[yy*w+xx], 16) +" - "+ Integer.toString(buffer[(y+yy)*srcW+(x+xx)], 16));           
            }
        }
        
        for(int o = 0; o<(w*h)>>1; o++) {
            if(highBits)
                stream.write((compact[o*2] & 0xF0) | ((compact[o*2+1] & 0xF0) >> 4));
            else
                stream.write(((compact[o*2] & 0x0F) << 4) | ((compact[o*2+1] & 0x0F)));
        }
        // Special case for odd size
        if(((w*h) & 1) == 1) {
            if(highBits)
                stream.write((compact[w*h-1] & 0xF0));
            else
                stream.write((compact[w*h-1] & 0x0F) << 4);
        }
    }


    static HashMap imageCache = new HashMap();

    static AlphaImage getImage(String fileName) {
        AlphaImage img = (AlphaImage)imageCache.get(fileName);
        if(img == null) {
            img = new AlphaImage(fileName);
            imageCache.put(fileName, img);
        }
        
        return img;
    }
    
    static boolean isValidName(String name)
    {
    	int n = name.length();
    	for (int i=0; i<n; i++)	
    	{
    		int c = name.charAt(i); 
    		if (!(c >= 'A' && c <= 'Z') &&
    			!(c >= 'a' && c <= 'z') &&
    			!(c >= '0' && c <= '9') &&
    			!(c >= '0' && c <= '9') &&
    			c != '_')
    			return false;
    	}
    	
    	return true;
    }

	public static void main(String[] args) throws Exception {
        if(args.length < 1) {
            System.err.println("USAGE: AuroraXP filename.sprite");
            System.exit(0);
        }
        
		ASprite a = new ASprite(args[0]);
		
		String fileName = new File(args[0]).getName();
		int idx = fileName.lastIndexOf(".");
		if(idx != -1) {
		  fileName = fileName.substring(0, idx);
		}
		
        System.out.println("///////////////////////////////////////////");
        System.out.println("// Sprite: " + fileName);
		
		BufferedOutputStream stream = new BufferedOutputStream(
				new FileOutputStream(fileName + ".bsp"));

        HashMap moduleMap   = new HashMap();
        HashMap frameMap    = new HashMap();

		try {
		  // Palette data first
		  short numPalettes = 0;
      Iterator it = a.imagesList.iterator();
      while(it.hasNext()) {
        ASprite.AImage i = (ASprite.AImage)a.images.get(it.next());
        numPalettes += i.palettes.size();
      } 
		  
		  writeShort(stream, numPalettes);
		
		  // After we have number of palettes, write them into the file
      it = a.imagesList.iterator();
      int num = 0;
      int pal_size = -1;
      while(it.hasNext()) {
        ASprite.AImage i = (ASprite.AImage)a.images.get(it.next());
        
        Iterator pit = i.palettes.iterator();
        while(pit.hasNext()) {
          ASprite.APalette p = (ASprite.APalette)pit.next();
          //System.err.println(p.data.length / 3);
          
          writeShort(stream, p.data.length / 3);
          stream.write(p.data, 0, p.data.length);

          String pName = p.fileName;
          if(pName == null) {
            pName = "IMAGE";
          } else {
            pName = pName.substring(0, pName.lastIndexOf('.'));
          }
          pName = pName.toUpperCase();
          System.out.println("#define "+fileName.toUpperCase()+"_P_"+pName+"\t"+num);
          
          num++;
        }
      } 
		
      writeShort(stream, a.modules.size());
      writeShort(stream, a.frames.size());
      writeShort(stream, a.anims.size());

      it = a.modulesList.iterator(); 
      //a.modules.keySet().iterator();
      num = 0;
      while(it.hasNext()) {
          ASprite.AModule m = (ASprite.AModule)a.modules.get(it.next());
          
          moduleMap.put(new Integer(m.id), new Integer(num));
          
          AlphaImage image = getImage(((ASprite.AImage)a.images.get(new Integer(m.imageId))).fileName);
          
          writeImage(stream, image, m.x, m.y, m.width, m.height);
          
          if (isValidName(m.name))
          	System.out.println("#define "+fileName.toUpperCase()+"_M_"+m.name.toUpperCase()+"\t"+num);
          else
          	System.out.println("//#define "+fileName.toUpperCase()+"_M_"+m.name.toUpperCase()+"\t"+num);
          
          num++;
      }

      // Iterate all frames and generate id -> num mappings, because of Hyperframes
      num = 0;
      it = a.framesList.iterator();
      while(it.hasNext()) {
          ASprite.AFrame f = (ASprite.AFrame)a.frames.get(it.next());
          
          frameMap.put(new Integer(f.id), new Integer(num));
          //System.out.println("frameMap: " + f.id + " " + num);
          
          num++;
      }            
      
      num = 0;
      it = a.framesList.iterator();
      //a.frames.keySet().iterator();
      while(it.hasNext()) {
          ASprite.AFrame f = (ASprite.AFrame)a.frames.get(it.next());
          
          int numFModules = f.modules.size();
          writeShort(stream, numFModules);
          
          for(int i = 0; i < numFModules; i++) {
              ASprite.AFModule afm = (ASprite.AFModule)f.modules.get(i);
              
              int modNum;

              if((afm.flags & ASprite.FM_FLAG_HYPER) != 0) {
                  modNum = ((Integer)frameMap.get(new Integer(afm.moduleId))).intValue();
              } else {
                  modNum = ((Integer)moduleMap.get(new Integer(afm.moduleId))).intValue();
              }
              
              writeShort(stream, modNum);
              writeShort(stream, afm.xOffset);
              writeShort(stream, afm.yOffset);
              writeShort(stream, afm.flags);
          }
          
          if (isValidName(f.name))
          	System.out.println("#define "+fileName.toUpperCase()+"_F_"+f.name.toUpperCase()+"\t"+num);
          else
          	System.out.println("//#define "+fileName.toUpperCase()+"_F_"+f.name.toUpperCase()+"\t"+num);
          
          num++;
      }
      
      num = 0;
      it = a.animsList.iterator();
      //a.frames.keySet().iterator();
      while(it.hasNext())
      {
          ASprite.Anim anim = (ASprite.Anim)a.anims.get(it.next());
          
          int numAFrames = anim.aframes.size();
          writeShort(stream, numAFrames);
          
          for(int i = 0; i < numAFrames; i++)
          {
              ASprite.AAFrame af = (ASprite.AAFrame)anim.aframes.get(i);
              
              //System.out.println(i + " af: " + af +   " " + frameMap + " " + af.frameId);
              
              int afNum = ((Integer)frameMap.get(new Integer(af.frameId))).intValue();
              
              writeShort(stream, afNum);
              writeShort(stream, af.time);
              writeShort(stream, af.xOffset);
              writeShort(stream, af.yOffset);
              writeShort(stream, af.flags);
          }
          
          if (isValidName(anim.name))
          	System.out.println("#define "+fileName.toUpperCase()+"_A_" + anim.name.toUpperCase() + "\t"+num);
          else
         	System.out.println("//#define "+fileName.toUpperCase()+"_A_" + anim.name.toUpperCase() + "\t"+num);
          
          num++;
      }
	
	} finally {
			stream.flush();
			stream.close();
		}
		
        System.out.println();
		
		//check(fileName + ".bsp");
    }
    
    static void check(String fileName) throws Exception {
		BufferedInputStream stream = new BufferedInputStream(
				new FileInputStream(fileName));

		System.err.println("Check  : " + fileName);

		int nPalettes = readShort(stream);
		System.err.println("Palettes: " + nPalettes);
    for(int i=0; i<nPalettes; i++) {
        int numColors = readShort(stream); 
    		System.err.println( "\t" + i + ". : [" + numColors + "]");
    		
    		// Skip palette
    		for(int j=0;j<numColors;j++) {
    		  stream.read();
    		  stream.read();
    		  stream.read();
    		}
    }
		
		int nModules  = readShort(stream);
		int nFrames   = readShort(stream);

		System.err.println("Modules: " + nModules);
		
		for(int i=0; i<nModules; i++) {
            int w = readShort(stream);
            int h = readShort(stream);
            int t = stream.read();
            
    		System.err.print( "\t" + i + ". : (" + w + "," + h + ") " + t + " ");
    		
    		if((t & 0x01) == 0x01) {
      		    System.err.print( "INDEXED ");
      		    if((t & 0x04) == 0x04)
          		    System.err.print( "LOWCOLOR ");
    		} else {
      		    System.err.print( "TRUE COLOR ");
    		}
    		
    		int size = w*h;
    		if((t & 0x04) == 0x04) {
    		  size = size >> 1;
    		  if(((w*h) & 1) == 1)
    		      size++; // one more byte for odd sizes
    		}
    		  
    		for(int j=0; j<size; j++) {
    		  // skip pixels
    		  if((t & 0x01) == 0x01) {
    		    stream.read(); // read bytes for Indexed images
    		  } else {
    		    readShort(stream); // read shorts for True color images
    		  }
    		}

    		if((t & 0x02) == 0x02) {
      		System.err.print( "ALPHA:YES");
      		size = (w*h) >> 1;
      		if(((w*h) & 1) == 1)
		      size++; // one more byte for odd sizes
      		
      		
      		for(int j=0; j<size; j++) {
      		  // skip alpha
      		  stream.read();
      		}
    		} else {
      		System.err.print( "ALPHA:NO");
    		}

    		System.err.println();
      } 

		System.err.println("Frames : " + nFrames);

        for(int i=0; i<nFrames; i++) {
            int numFModules = readShort(stream);
    		System.err.println("\t" + i + " (" + numFModules + ")");
            
            for(int j=0;j<numFModules; j++) {
        		System.err.println(
                    "\t\t" + j + ". : " + readShort(stream) + " " +
                    "(" + readShort(stream) + "," + readShort(stream) + ") " +
                    readShort(stream));
            }
        }
        
        stream.close();
    }
}
