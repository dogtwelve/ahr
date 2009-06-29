import java.io.*;
import java.util.*;

import java.awt.*;
import java.awt.image.*;

import javax.swing.ImageIcon;

public class AlphaImage {
    int     width;
    int     height;
    int[]   pixels;
    int[]   alpha;
    byte[]  palette;
    int[]   compactPalette;
    
    boolean grayScaleImage     = false; 
    boolean trueColorImage     = false; 
    boolean indexedImage       = false;
    boolean lowColorImage      = false;

    
    public AlphaImage(String aFileName) {
      //System.err.println("Loading: "+aFileName);
          File file = new File(aFileName);
          
  		String filePath = file.getAbsolutePath();
  		int idx = filePath.lastIndexOf(".");
  		if(idx != -1) {
  		  filePath = filePath.substring(0, idx);
  		}
      
  		String fileName = file.getName();
          idx = fileName.lastIndexOf(".");
  		if(idx != -1) {
  		  filePath = fileName.substring(0, idx);
  		}
  
      PNGLoader png = new PNGLoader(aFileName);
      byte[] hdr = png.getChunkData("ihdr");
      
      switch(hdr[9]) {
        case 0: grayScaleImage = true; break;
        case 2: trueColorImage = true; break;
        case 3: indexedImage   = true; break;
      }
      
      int     bitDepth           = hdr[8];
  
      byte[] pal = null; 
      palette = null;
      if (indexedImage)
      {        
          pal = png.getChunkData("plte");
          if(pal == null) {
              System.err.println("ERROR: Indexed image without palette: " + aFileName);
              System.exit(1);
          }
          
          compactPalette = new int[pal.length / 3];
          palette = new byte[pal.length];
          
          for(int i=0; i<compactPalette.length; i++) {
            compactPalette[i] = getColor16(
                  ((pal[i*3] & 0xFF)   << 16) | 
                  ((pal[i*3+1] & 0xFF) <<  8) | 
                  ((pal[i*3+2] & 0xFF)));
                  
              palette[i*3]   = (byte)((compactPalette[i] >> 11) & 0xFF); 
              palette[i*3+1] = (byte)((compactPalette[i] >>  6) & 0xFF); 
              palette[i*3+2] = (byte)((compactPalette[i]      ) & 0xFF); 
          }
          
          if(compactPalette.length == 16) {
            lowColorImage = true;
          }
      }
      
  		ImageIcon icon = new ImageIcon(aFileName);
  		
  		width     = icon.getIconWidth();
  		height    = icon.getIconHeight();
  		
  		pixels    = new int[width*height];
  		alpha     = new int[width*height];
  		
  		PixelGrabber pg = new PixelGrabber(icon.getImage(), 0, 0, width, height, pixels, 0, width);
  		try {
  			pg.grabPixels();
  		} catch (InterruptedException e) {
  			System.err.println("interrupted waiting for pixels!");
  		}
  		if ((pg.getStatus() & ImageObserver.ABORT) != 0) {
  			System.err.println("image fetch aborted or errored");
  		}
  		
  		// Read pixels data as true color data from PixelGrabber
  		for(int i=0; i<pixels.length; i++)
  		{
          		int color24 = pixels[i];
          		pixels[i] = getColor16(color24);
          		alpha[i] = getAlpha(color24);
  		}
  		
  		if(indexedImage)
  		{
  		    //System.out.println("Indexed: " + compactPalette.length);
  		    
  		    // Convert pixels data from true color to palette indexes
      			for(int i=0; i<pixels.length; i++)
      			{
      				int color = pixels[i];
      				boolean found = false;
      		  		
      		  		for(int j=0; j<compactPalette.length; j++)
      		  		{
					if(color == compactPalette[j])
					{
      		          			pixels[i] = j;
      		          			found = true;
      		      			}
      		  		}
      		  		
      		  		if(!found)
      		  		{
      		      			System.err.println("ERROR: Color not in palette. " + i + " - 0x" + Integer.toString(color, 16));
      		      			System.exit(1);
      		  		}
      		  		
      		  		if (compactPalette[pixels[i]] == 0xF81F) // 0xFF00FF
      		  			alpha[i] = 0;
      			}
  		    
			// Try to load separate _mask file for Indexed image
			icon = new ImageIcon(filePath + "_mask.png");
      		
			int aWidth     = icon.getIconWidth();
			int aHeight    = icon.getIconHeight();

          		if(aWidth != -1)
          		{ // we have a valid image      		
      		
            			if (aWidth != width || aHeight != height)
            			{
                			System.err.println("ERROR: Alpha image ("+filePath+"_mask.png) is not with the same dimentions as base image.");
                			System.exit(1);
            			}    		
        		
        			pg = new PixelGrabber(icon.getImage(), 0, 0, width, height, alpha, 0, width);
	        		try {
	        			pg.grabPixels();
	        		} catch (InterruptedException e) {
	        			System.err.println("interrupted waiting for pixels!");
	        		}
	        		if ((pg.getStatus() & ImageObserver.ABORT) != 0) {
	        			System.err.println("image fetch aborted or errored");
	        		}
	        		
	        		for(int i=0; i<alpha.length; i++)
	        		{
                			int color24 = alpha[i];
                			alpha[i] = getRed(color24);
        			}
        		}
  		}
    	}

	int getAlpha(int color24) {
		return ((color24 >> 24) & 0xff);
	}

	int getRed(int color24) {
		return ((color24) & 0xff);
	}
    
	int getColor16(int color24) {

		int red = ((color24 >> 16) & 0xff);
		int green = ((color24 >> 8) & 0xff);
		int blue = ((color24) & 0xff);
		int result = 0;

		result = result | ((red >> 3) << 11);
		result = result | ((green >> 2) << 5);
		result = result | ((blue >> 3));

		return result;
	}
	
}

class PNGLoader {
    byte[]  data;
    int     idx;

    public PNGLoader(String fileName) {
        try {
            FileInputStream fis = new FileInputStream(fileName);
            data = new byte[fis.available()];
            fis.read(data);
            fis.close();
        } catch(Exception e) {
            System.err.println("ERROR: Can't open file: " + fileName);
        }
    }
    
	public byte[] getChunkData(String chunk) {
	   idx = 0;
	   idx += 8; // Skip PNG identification bytes
	   
	   while(idx < data.length) {
	       int chunkLength = getChunkLength();  idx += 4;
	       String chunkType = getChunkType();   idx += 4;
	       
	       //System.out.println(chunkType + " " + chunkLength);
	       
	       if(chunk.compareTo(chunkType) == 0) {
	           byte[] res = new byte[chunkLength];

	           System.arraycopy(data, idx, res, 0, chunkLength);
	           
	           return res;
	       } else {
	           idx += (chunkLength + 4); // data + CRC
	       }
	   }
	   
	   return null;
	}
	
	private int getChunkLength() {
        return ((int)(data[idx] & 0xFF) << 24) | ((int)(data[idx+1] & 0xFF) << 16) | ((int)(data[idx+2] & 0xFF) << 8) | ((int)(data[idx+3] & 0xFF));
	}

	private String getChunkType() {
	   StringBuffer sb = new StringBuffer();
	   sb.append((char)data[idx]);
	   sb.append((char)data[idx+1]);
	   sb.append((char)data[idx+2]);
	   sb.append((char)data[idx+3]);
	
	   return sb.toString().toLowerCase();
	}
}
