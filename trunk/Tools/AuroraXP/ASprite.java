import java.io.*;
import java.util.*;
import java.util.regex.*;

public class ASprite {
    final static int FM_FLAG_FLIP_X = 1 << 0;
    final static int FM_FLAG_FLIP_Y = 1 << 1;
    final static int FM_FLAG_ROT_90 = 1 << 2;
    final static int FM_FLAG_HYPER  = 1 << 3;

    final static int PATTERN_IMAGE          = 0;
    final static int PATTERN_MODULE         = 1;
    final static int PATTERN_MODULE_SHAPE   = 2;
    final static int PATTERN_FMODULE        = 3;
    final static int PATTERN_PALETTE        = 4;
    final static int PATTERN_MODULE_NO_NAME = 5;
    final static int PATTERN_AFRAME 	    = 6;

    Pattern linePatterns[] = new Pattern[] {
        Pattern.compile("IMAGE\\s+0x(\\p{XDigit}+)\\s+\"(.+)\"\\s+TRANSP\\s+0x\\p{XDigit}+"),
        Pattern.compile("MD\\s+0x(\\p{XDigit}+)\\s+(MD_IMAGE)\\s+(?:0x)?(\\p{XDigit}+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+\"(.+)\""),
        Pattern.compile("MD\\s+0x(\\p{XDigit}+)\\s+(MD_RECT|MD_FILL_RECT)\\s+(?:0x)?(\\p{XDigit}+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+\"(.+)\""),
        Pattern.compile("FM\\s+0x(\\p{XDigit}+)\\s+([+-]?\\d+)\\s+([+-]?\\d+)(.*)"),
        Pattern.compile("PALETTE\\s+(\\p{XDigit}+)\\s+\"(.+)\""),
        Pattern.compile("MD\\s+0x(\\p{XDigit}+)\\s+(MD_IMAGE)\\s+(?:0x)?(\\p{XDigit}+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+([-\\d]+)\\s+([-\\d]+)"),
        Pattern.compile("AF\\s+0x(\\p{XDigit}+)\\s+([+-]?\\d+)\\s+([+-]?\\d+)\\s+([+-]?\\d+)(.*)"),
    };

    HashMap     images  = new HashMap();
    HashMap     modules = new HashMap();
    HashMap     frames  = new HashMap();
    HashMap     anims  = new HashMap();

    List   imagesList   = new ArrayList();
    List   modulesList = new ArrayList();
    List   framesList  = new ArrayList();
    List   animsList  = new ArrayList();

    AImage  lastImage = null;
    
    int		moduleNoNameId;
    int		frameNoNameId;
    int		animNoNameId;

    int    pLine;
	
    ASprite(String fileName) throws Exception    
    {
		BufferedReader b = new BufferedReader(new FileReader(fileName));
		
		String line;
		pLine = 0;
		boolean haveBlockComment = false;

        Stack   blockStack  = new Stack();
        String  blockType   = ""; 
        
        AFrame  frame       = null;
        Anim  anim       = null;
        
        	moduleNoNameId	= -1;
        	frameNoNameId = 0;
        	animNoNameId = 0;

		while((line = b.readLine()) != null) {
		    pLine++;
		    
            String tline = line.trim();

	// System.out.println("LINE: "+pLine + " | " + tline);
            
            // Strip comments

            // part 1: Line comments

            int commentIndex = tline.indexOf("//");
    
            if(commentIndex != -1) {
                if(commentIndex == 0) {
                    tline = "";
                } else {        
                    tline = tline.substring(0, commentIndex-1);
                }
            }
                
            // part 2: Block comments
            
            int blockCommentBegin   = tline.indexOf("/*");
            int blockCommentEnd     = tline.lastIndexOf("*/");
            
            if(blockCommentBegin != -1) {
                if(blockCommentEnd != -1) {
                    // Block comment ends on current line
                    String cline1;
                    String cline2;
                    
                    if(blockCommentBegin == 0) {
                        cline1 = "";
                    } else {
                        cline1 = tline.substring(0, blockCommentBegin - 1);
                    }
                    
                    if(blockCommentEnd == tline.length() - 2) {
                        cline2 = "";
                    } else {
                        cline2 = tline.substring(blockCommentEnd + 2);
                    }
                    
                    tline = cline1 + cline2; 
                    
                    // Block comment processed
                    blockCommentBegin   = -1;
                    blockCommentEnd     = -1;
                } else {
                    // Block comment ends on some other line
                    haveBlockComment = true;
                    
                    if(blockCommentBegin == 0) {
                        tline = "";
                    } else {
                        tline = tline.substring(0, blockCommentBegin - 1);
                    }
                }
            }
            
            if(blockCommentEnd != -1) {
                if(!haveBlockComment) {
                    System.err.println("PARSE ERROR: No matching /* for */ on line " + pLine);
                    System.exit(1);
                } else {
                    // Block comment started on previous line, end here

                    if(blockCommentEnd == tline.length() - 2) {
                        tline = "";
                    } else {
                        tline = tline.substring(blockCommentEnd + 2);
                    }
                }
            }

            // Remove all leading/trailing white space left after comments
            tline = tline.trim();

            // If we have empty line at this point, then line was empty or commented
            if(tline.length() == 0)
              continue;
            
            // Begin of block
            if(tline.startsWith("{")) {
                if(blockType.length() == 0 && !blockStack.empty()) {
                    System.err.println("PARSE ERROR: Block begins without name registered on line " + pLine);                                    
                    System.exit(1);
                }
                
                blockStack.push(blockType);
                
                // Nothing more here, let's check the next line
                continue;
            }
            
            if(tline.startsWith("}"))
            {
                if(blockStack.empty()) {
                    System.err.println("PARSE ERROR: Block ends without matching { on line " + pLine);                                    
                    System.exit(1);
                }
                
                String lastBlockType = (String)blockStack.pop();

                if("FRAME".compareTo(lastBlockType) == 0)
                {
                    Integer frameId = new Integer(frame.id);
                    frames.put(frameId, frame);
                    framesList.add(frameId);
                    frame = null;
                }
                else
                if("ANIM".compareTo(lastBlockType) == 0)
                {
                    Integer animId = new Integer(anim.id);
                    anims.put(animId, anim);
                    animsList.add(animId);
                    anim = null;
                }
                

                // Nothing more here, let's check the next line
                continue;
            }
            
            if(tline.startsWith("VERSION")) {
                // Skip version, who cares :)
            } else if(tline.startsWith("IMAGE")) {
                parseImageLine(tline);
            } else if(tline.startsWith("PALETTE")) {
                parsePaletteLine(tline);
            } else if(tline.startsWith("MODULES")) {
                blockType = "MODULES";            
            } else if(tline.startsWith("FRAME")) {
                blockType = "FRAME";
                
                int frameNameBegin   = tline.indexOf("\"");
                int frameNameEnd     = tline.lastIndexOf("\"");

                if(frameNameBegin == -1 || frameNameEnd == -1) {
                    System.err.println("PARSE ERROR: FRAME names are required! Line: " + pLine);                                    
                    System.exit(1);
                }
                
                frame = new AFrame();
                
                if (frameNameBegin + 1 == frameNameEnd)
                {
                	frame.name = "FRAME_" + frameNoNameId;
                	frameNoNameId ++;
                }
                else
                {
                	frame.name = tline.substring(frameNameBegin+1, frameNameEnd);
                }
            }
            else if(tline.startsWith("ANIM"))
            {
                blockType = "ANIM";
                
                int animNameBegin   = tline.indexOf("\"");
                int animNameEnd     = tline.lastIndexOf("\"");

                /*if(animNameBegin == -1 || animNameEnd == -1)
                {
                    System.err.println("PARSE ERROR: ANIM names are required! Line: " + pLine);                                    
                    System.exit(1);
                }
                */
                
                anim = new Anim();
                
                if (animNameBegin == -1 || animNameEnd == -1 || animNameBegin + 1 == animNameEnd)
                {
                	anim.name = "ANIM_" + animNoNameId;
                	animNoNameId ++;
                }
                else
                {
                	anim.name = tline.substring(animNameBegin + 1, animNameEnd);
                }
            }
            else
            {
                if("MODULES".compareTo("" + blockStack.peek()) == 0) {
                    if(tline.startsWith("MD")) {
                        parseModuleLine(tline);
                    } else {
                        System.err.println("PARSE ERROR: MODULES section must contain only MD elements. Line: " + pLine);                                    
                        System.exit(1);
                    }
                }
                else if("FRAME".compareTo("" + blockStack.peek()) == 0)
                {
			if(tline.startsWith("RC"))
			{
				// Skip RC data 
			} else if(tline.startsWith("FM"))
			{
                        	parseFrameLine(frame, tline);
                    	}
                    	else if (frame != null)
			{
                        	// Remove leading 0x from frame id
                        	frame.id = Integer.parseInt(tline.substring(2), 16);
                    	}
                }
		else if("ANIM".compareTo("" + blockStack.peek()) == 0)
		{
			if(tline.startsWith("AF"))
			{
                        	parseAnimLine(anim, tline);
                    	}
                    	else if (anim != null)
			{
                        	// Remove leading 0x from frame id
                        	anim.id = Integer.parseInt(tline.substring(2), 16);
                    	}
                }
                else
                {
                    // Unknown line skip ... or better throw a PARSE ERROR ?    
                    //System.err.println("Error: " + tline);
                }                
            }
	}
	
	Object[] temp = new Object[modulesList.size()];
        modulesList.toArray(temp);
        Arrays.sort(temp);
        modulesList = Arrays.asList(temp); 

	temp = new Object[framesList.size()];
        framesList.toArray(temp);
        Arrays.sort(temp);
        framesList = Arrays.asList(temp); 
    }
    
    void parseImageLine(String line) {
		Matcher m = linePatterns[PATTERN_IMAGE].matcher(line);
		if(m.matches()) {
      AImage image    = new AImage();
      image.id        = Integer.parseInt(m.group(1).trim(), 16);
      image.fileName  = m.group(2).trim();
      
      AlphaImage  imageData = new AlphaImage(image.fileName);
      
      image.pixels    = imageData.pixels;
      image.alpha     = imageData.alpha;
            
			images.put(new Integer(image.id), image);
  		imagesList.add(new Integer(image.id));
			
			
			image.palettes  = new ArrayList();
			
			// Loaded image is indexed, so get first palette
			if(imageData.indexedImage) {
			   APalette pal = new APalette();
			   pal.fileName = null;
			   pal.data = imageData.palette;
			   
         image.palettes.add(pal);
      }
			
			lastImage = image;
		} else {
            System.err.println("PARSE ERROR: Image Line: " + pLine);                                    
            System.exit(1);
		}
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

    void parsePaletteLine(String line) {
        if(lastImage == null) {
            System.err.println("PARSE ERROR: No Image Line before palette: " + pLine);                                    
            System.exit(1);
        }

		Matcher m = linePatterns[PATTERN_PALETTE].matcher(line);
		if(m.matches()) {
            APalette pal    = new APalette();
            pal.fileName    = m.group(2).trim();
            pal.data        = new byte[768];

            try {            
                FileInputStream fis = new FileInputStream(pal.fileName);
                fis.read(pal.data);  
                fis.close();
                
                for(int i=0; i<256; i++) {
                  int temp = getColor16(
                        ((pal.data[i*3] & 0xFF)   << 16) | 
                        ((pal.data[i*3+1] & 0xFF) <<  8) | 
                        ((pal.data[i*3+2] & 0xFF)));
                        
                    pal.data[i*3]   = (byte)((temp >> 11) & 0xFF); 
                    pal.data[i*3+1] = (byte)((temp >>  6) & 0xFF); 
                    pal.data[i*3+2] = (byte)((temp      ) & 0xFF); 
                }
            } catch(IOException e) {
                System.err.println("ERROR: Can't open palette file: " + pal.fileName);
                System.exit(1);
            }

            lastImage.palettes.add(pal);
		} else {
            System.err.println("PARSE ERROR: Palette Line: " + pLine);                                    
            System.exit(1);
		}
    }

    void parseModuleLine(String line)
    {
		Matcher m = linePatterns[PATTERN_MODULE].matcher(line);
		boolean moduleNoName = false;

		if(m.matches())
		{
		  // cool, we have an IMAGE module
		}
		else
		{
			m = linePatterns[PATTERN_MODULE_NO_NAME].matcher(line);
			if(m.matches())
			{
				// cool, we have an IMAGE module with no name
				
				moduleNoName = true;
				moduleNoNameId++;
			}
			else
			{
			    // It's not Image module, let's try if it's Shape (RECT|FILL_RECT)
			    m = linePatterns[PATTERN_MODULE_SHAPE].matcher(line);
			    if(m.matches())
			    {
	    		  	// cool, we have a SHAPE module
			    }
			    else
			    {
	                	System.err.println("PARSE ERROR: Module Line: " + pLine);                                    
	                	System.exit(1);
			    }
			}
		}
		
		AModule module = new AModule();
        module.id           = Integer.parseInt(m.group(1).trim(), 16);
        module.type         = m.group(2).trim();
		
		if("MD_IMAGE".compareTo(module.type) == 0) {
            module.imageId      = Integer.parseInt(m.group(3).trim(), 16);
            
            module.x            = Integer.parseInt(m.group(4).trim());
            module.y            = Integer.parseInt(m.group(5).trim());
            module.width        = Integer.parseInt(m.group(6).trim());
            module.height       = Integer.parseInt(m.group(7).trim());
             			
            if (!moduleNoName)
            	module.name         = m.group(8).trim();
            else module.name = "MODULE_" + moduleNoNameId;
			
		} else {
		    module.color        = Integer.parseInt(m.group(3).trim(), 16);

            module.width        = Integer.parseInt(m.group(4).trim());
            module.height       = Integer.parseInt(m.group(5).trim());

		if (!moduleNoName)
            		module.name         = m.group(6).trim();
            	else module.name = "MODULE_" + moduleNoNameId;
		}
        		
		Integer moduleId = new Integer(module.id);
		modules.put(moduleId, module);
		modulesList.add(moduleId);
    }

    void parseFrameLine(AFrame frame, String line) {
		AFModule fmodule = new AFModule();
		
		Matcher m = linePatterns[PATTERN_FMODULE].matcher(line);
		if(m.matches()) {
			fmodule.moduleId = Integer.parseInt(m.group(1).trim(), 16);
			fmodule.xOffset  = Integer.parseInt(m.group(2).trim());
			fmodule.yOffset  = Integer.parseInt(m.group(3).trim());
			
		} else {
            System.err.println("PARSE ERROR: Frame Line: " + pLine);                                    
            System.exit(1);
		}
		
		String flags = m.group(4).trim();
        
        //System.out.println(flags);
        
        if(flags.indexOf("FLIP_X") != -1) {
            fmodule.flags |= FM_FLAG_FLIP_X;
        }		

        if(flags.indexOf("FLIP_Y") != -1) {
            fmodule.flags |= FM_FLAG_FLIP_Y;
        }		

        if(flags.indexOf("ROT_90") != -1) {
            fmodule.flags |= FM_FLAG_ROT_90;
        }

        if(flags.indexOf("HYPER_FM") != -1) {
            fmodule.flags |= FM_FLAG_HYPER;
        }
        
        frame.modules.add(fmodule);		
    }
    
    void parseAnimLine(Anim anim, String line)
    {
	AAFrame aframe = new AAFrame();
		
	Matcher m = linePatterns[PATTERN_AFRAME].matcher(line);
	if(m.matches())
	{
		aframe.frameId  = Integer.parseInt(m.group(1).trim(), 16);
		aframe.time     = Integer.parseInt(m.group(2).trim());
		aframe.xOffset  = Integer.parseInt(m.group(3).trim());
		aframe.yOffset  = Integer.parseInt(m.group(4).trim());
	}
	else
	{
            System.err.println("PARSE ERROR: AFrame Line: " + pLine);                                    
            System.exit(1);
	}
		
	String flags = m.group(5).trim();
        
        //System.out.println(flags);
        
        if(flags.indexOf("FLIP_X") != -1) {
            aframe.flags |= FM_FLAG_FLIP_X;
        }		

        if(flags.indexOf("FLIP_Y") != -1) {
            aframe.flags |= FM_FLAG_FLIP_Y;
        }		

        if(flags.indexOf("ROT_90") != -1) {
            aframe.flags |= FM_FLAG_ROT_90;
        }

//        if(flags.indexOf("HYPER_FM") != -1) {
//            aframe.flags |= FM_FLAG_HYPER;
//        }
        
        anim.aframes.add(aframe);
    }

    class APalette {
        int     id;
        String  fileName;
        byte[]  data;
    }

    class AImage {
        int         id;
        String      fileName;
        int[]       pixels;
        int[]       alpha;
        ArrayList   palettes;
    }
    
    class AModule {
        int     id;
        String  type;
        int     imageId;
        int     color;
        int     x;
        int     y;
        int     width;
        int     height;
        String  name;
    }
    
    class AFrame {
        int         id;
        String      name;        
        ArrayList   modules = new ArrayList();  
    }
    
    class AFModule {
        int         moduleId;
        int         xOffset;
        int         yOffset;
        int         flags;
    }
    
    class AAFrame    
    {
    	int	frameId;
    	int	xOffset;
        int	yOffset;
        int	flags;
        int	time;
    }
    
    class Anim
    {
        int         id;
        String      name;
        ArrayList   aframes = new ArrayList();
    }
}
