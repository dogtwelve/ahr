
import java.io.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.Graphics2D;
import javax.imageio.*;

public class CBuildBitmapFont
{
    public static int _BITS_PER_PIXEL_ = 1;
    
    private static int m_iFontHeight = 11;
    private static int m_iTextPosY = 10;
    private static int m_iMaxCharWidth = 30;
    private static int m_iMaxCharPerLine = 8;
    private static int[] m_aiCharMatrix = null;
    private static int buffImageW;
    private static int buffImageH;
    
    public CBuildBitmapFont ()
    {
        // TODO Auto-generated constructor stub
    }
    
    public static boolean CreateFontPackage (Object[] objCharArrayToPack, Font usingFont, String strOutName, int spaceOffsetX, int spaceWidth)
    {
        int iTotalCharsToPack = objCharArrayToPack.length;
        
        int posx = CMain.m_addedWidth;
        int posy = CMain.m_addedWidth;
        
        int[] letterWidths = new int[objCharArrayToPack.length];
        int[] letterHeights = new int[objCharArrayToPack.length];
        
        //Open unique characters file to write and sprite file
        FileOutputStream outStream = null;
        FileOutputStream spriteStream = null;
        DataOutputStream spriteDos = null;
        OutputStreamWriter outDos = null;
        try
        {
            outStream = new FileOutputStream (strOutName + ".txt");
            outDos = new OutputStreamWriter (outStream, "Unicode");
            spriteStream = new FileOutputStream (strOutName + ".sprite");
            spriteDos = new DataOutputStream (spriteStream);
        }
        catch (Exception ex)
        {
            System.out.println ("Can't create file : " + strOutName + ".txt or " + strOutName + ".sprite for writing unique chars.");
            return false;
        }
        
        
        //Specify font height in pixel and Y drawing offset
        SetHeightAndDrawOffset (objCharArrayToPack, usingFont);
        
        //Create buffer image
        BufferedImage imgBuffer = null;
        BufferedImage letterBuffer = null;
        BufferedImage finalBuffer = null;
        Graphics2D currGraph = null;
        Graphics2D imgGraph = null;
        
        buffImageW = (m_iMaxCharWidth + CMain.m_addedMargin +  CMain.m_addedWidth) * m_iMaxCharPerLine + CMain.m_addedWidth + CMain.m_addedMargin;
        buffImageH = (m_iFontHeight + CMain.m_addedWidth) * (iTotalCharsToPack/m_iMaxCharPerLine + 2) + CMain.m_addedWidth;
        
        try
        {
            imgBuffer = new BufferedImage (buffImageW, buffImageH, BufferedImage.TYPE_INT_ARGB);
            imgGraph = imgBuffer.createGraphics ();
            letterBuffer = new BufferedImage (m_iMaxCharWidth, m_iFontHeight, BufferedImage.TYPE_INT_ARGB);
            currGraph = letterBuffer.createGraphics ();
            currGraph.setFont (usingFont);
            if (CMain.m_useAntialiasing)
            {
             currGraph.setRenderingHint( RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON );
             currGraph.setRenderingHint( RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY );
            }
        }
        catch (Exception ex)
        {
            System.out.println ("Error in create image.");
            return false;
        }
        
        //Create font package
        try
        {
            byte[] 	byteUnicode = new byte[2];

            //byteUnicode[0] = (byte)0xFF;
            //byteUnicode[1] = (byte)0xFE;
            //outDos.write (byteUnicode);
            
            outDos.write ("Size: " + objCharArrayToPack.length + "\r\n");
            
            m_aiCharMatrix = new int[m_iFontHeight * m_iMaxCharWidth];
            Character currObjChar = null;
            char currCharToPack = ' ';           
            
            
            imgGraph.setColor (new Color (CMain.m_bgR, CMain.m_bgG, CMain.m_bgB));
            imgGraph.fillRect (0, 0, buffImageW, buffImageH);
            
            int maxLineHeight = 0;
            
            for (int i=0;i<objCharArrayToPack.length;i++)
            {
                currObjChar = (Character)objCharArrayToPack[i];
                currCharToPack = currObjChar.charValue ();
                
                //Write Unicode of char to output file
                byteUnicode[0] = (byte)(currCharToPack & 0xFF);
                byteUnicode[1] = (byte)(currCharToPack >> 8);
                outDos.write ("0x");          
                if ((byteUnicode[1] & 0xFF) < 16)             
	                outDos.write ("0" + Integer.toHexString(byteUnicode[1] & 0xFF));
	        else                                                                 
	        	outDos.write (Integer.toHexString(byteUnicode[1] & 0xFF).toUpperCase());
	        if ((byteUnicode[0] & 0xFF) < 16)
	                outDos.write ("0" + Integer.toHexString(byteUnicode[0] & 0xFF));
	        else                                                                 
	        	outDos.write (Integer.toHexString(byteUnicode[0] & 0xFF).toUpperCase());
                outDos.write(", // ");
                if (currCharToPack == '\\')
                	outDos.write("backslash");
                else
                	outDos.write(currObjChar.charValue());
                outDos.write("\r\n");
                	
                
                // patch to display the chinese '.' and ',' a bit lower not centered on row
                int offsetY = 0;
                /*if (currCharToPack == 0x3002 || currCharToPack == 0xFF0C )
                {
                    offsetY += usingFont.getSize () / 4;
                }*/
                
                //Draw char to image buf
                if (_BITS_PER_PIXEL_ == 2)
                {
                    currGraph.setColor (new Color (CMain.m_bgR, CMain.m_bgG, CMain.m_bgB));
                    currGraph.fillRect (0, 0, m_iMaxCharWidth, m_iFontHeight);
                    currGraph.setColor (new Color (CMain.m_outlineR, CMain.m_outlineG, CMain.m_outlineB));
                    //Border top
                    currGraph.drawString (Character.toString (currCharToPack), 0, m_iTextPosY+offsetY);
                    currGraph.drawString (Character.toString (currCharToPack), 1, m_iTextPosY+offsetY);
                    currGraph.drawString (Character.toString (currCharToPack), 2, m_iTextPosY+offsetY);
                    
                    //Border bottom
                    currGraph.drawString (Character.toString (currCharToPack), 0, m_iTextPosY+offsetY+2);
                    currGraph.drawString (Character.toString (currCharToPack), 1, m_iTextPosY+offsetY+2);
                    currGraph.drawString (Character.toString (currCharToPack), 2, m_iTextPosY+offsetY+2);
                    
                    //Border left, right
                    currGraph.drawString (Character.toString (currCharToPack), 0, m_iTextPosY+offsetY+1);
                    currGraph.drawString (Character.toString (currCharToPack), 2, m_iTextPosY+offsetY+1);
                    
                    //Main char
                   currGraph.setColor (new Color (CMain.m_innerR, CMain.m_innerG, CMain.m_innerB));
                   currGraph.drawString (Character.toString (currCharToPack), 1, m_iTextPosY+offsetY+1);
                }
                else
                {
                    currGraph.setColor (new Color (CMain.m_bgR, CMain.m_bgG, CMain.m_bgB));
                    currGraph.fillRect (0, 0, m_iMaxCharWidth, m_iFontHeight);
                    currGraph.setColor (new Color (CMain.m_innerR, CMain.m_innerG, CMain.m_innerB));
                    currGraph.drawString (Character.toString (currCharToPack), 0, m_iTextPosY+offsetY);
                }
                
                //Encode image of character
                CompressTextImage (letterBuffer);
                
                if (currObjChar.charValue () == ' ')
                {
                    letterWidths[i] = spaceWidth;
                    letterHeights[i] = m_iFontHeight;
                }
                else
                {
                    letterWidths[i] = m_crtCharWidth;
                    letterHeights[i] = m_crtCharHeight;
                }
                
                if (letterHeights[i] > maxLineHeight)
                	maxLineHeight = letterHeights[i];
                
                if ((posx + letterWidths[i] +  CMain.m_addedWidth) >= buffImageW)
                {
                    posx = CMain.m_addedWidth;
                    //posy += (m_iFontHeight + CMain.m_addedWidth);
                    posy += (maxLineHeight + CMain.m_addedMargin + CMain.m_addedWidth);
                    
                    maxLineHeight = 0;
                }
                
                try
                {
                    imgBuffer.setRGB (posx, posy, (int)letterWidths[i], m_iFontHeight, m_aiCharMatrix, 0, m_iMaxCharWidth);
                
                }
                catch (Exception e)
                {
                    System.out.println ("Current pos " + posx + " " + posy+ " w " + imgBuffer.getWidth () + " h " + imgBuffer.getHeight ());
                    System.out.println ("Exception was : " + e);
                }
               
         
                posx += letterWidths[i] + CMain.m_addedWidth;
      
            }
            
            //write the sprite header file
            boolean ret;
            ret = WriteSprite (spriteDos, strOutName, m_iMaxCharWidth*m_iMaxCharPerLine, posy+m_iFontHeight, objCharArrayToPack.length, letterWidths, letterHeights, objCharArrayToPack, spaceOffsetX);
            if (!ret)
                return false;
            
            outDos.flush();
            outDos.close();
            
            outStream.flush();
            outStream.close ();
            
            spriteDos.flush ();
            spriteDos.close ();
            
            spriteStream.flush ();
            spriteStream.close ();
            
     
        }
        catch (Exception ex)
       {
            System.out.println ("Error in processing JP text... :" + ex);
            ex.printStackTrace();
            return false;
        }
        
        // generate a smaller image
        try
        {
            finalBuffer = new BufferedImage (buffImageW, posy+m_iFontHeight + CMain.m_addedWidth, BufferedImage.TYPE_INT_ARGB);
        }
        catch (Exception ex)
        {
            System.out.println ("Error in create final image.");
            return false;
        }
        
        //write the image
        try
        {
            int[] bitmapArray = new int[buffImageW * (posy+m_iFontHeight + CMain.m_addedWidth)];
            
            imgBuffer.getRGB (0, 0, buffImageW, posy+m_iFontHeight + CMain.m_addedWidth, bitmapArray, 0, buffImageW);
            finalBuffer.setRGB (0, 0, buffImageW, posy+m_iFontHeight + CMain.m_addedWidth, bitmapArray, 0, buffImageW);
            
            boolean ret = ImageIO.write (finalBuffer, "png", new File (strOutName+".png"));
            if (!ret)
            {
                System.out.println ("ImageIO.write: No appropiate write found!");
                return false;
            }
            
        }
        catch (Exception ex)
        {
            System.out.println ("Error in writing the image :" + ex);
            return false;
        }
        
        System.out.println ("Build font package successfully !");
        return true;
    }
    
    public static int m_crtCharWidth;
    public static int m_crtCharHeight;
    
    private static void CompressTextImage (BufferedImage inputImgBuffer)
    {
        inputImgBuffer.getRGB (0, 0, m_iMaxCharWidth, m_iFontHeight, m_aiCharMatrix, 0, m_iMaxCharWidth);
        
        //Delete unnecessary space before character
        DeleteSpaceBeforeChar (m_aiCharMatrix);
        
        m_crtCharWidth  = GetMaxWidth (m_aiCharMatrix);
        m_crtCharHeight = GetMaxHeight (m_aiCharMatrix);
    }
    
    private static void DeleteSpaceBeforeChar (int[] charArray)
    {
        //Find space width
        int iSpaceW = 0;
        boolean isSpace = false;
        int color = ((0xFF<<24) | ((CMain.m_bgR&0xFF) << 16) | ((CMain.m_bgG & 0xFF) << 8) | (CMain.m_bgB & 0xFF));
        // For each column
        for (int i = 0; i < m_iMaxCharWidth; i++)
        {
            isSpace = true;
            // For each row
            for (int j = 0; j < m_iFontHeight; j++)
            {
                if (charArray[j*m_iMaxCharWidth + i] != color)
                {
                    isSpace = false;
                    break;
                }
            }
            
            if (isSpace)
            {
                iSpaceW++;
            }
            else
            {
                break;
            }
        }
        
        //Del space
        if (iSpaceW > 0)
        {
            // For each row
            for (int i = 0; i < m_iFontHeight; i++)
            {
                // Shift left
                for (int j = 0; j < (m_iMaxCharWidth-iSpaceW); j++)
                {
                    charArray[m_iMaxCharWidth*i + j] =  charArray[m_iMaxCharWidth*i + j + iSpaceW];
                }
                for (int j = (m_iMaxCharWidth-iSpaceW); j < m_iMaxCharWidth; j++)
                {
                    charArray[m_iMaxCharWidth*i + j]  = color;
                }
            }
        }
    }
    
    private static byte GetMaxWidth (int[] charArray)
    {
        byte byteMaxWidth = 0;
        byte byteLineWidth = 0;
        int color = ((0xFF<<24) | ((CMain.m_bgR&0xFF) << 16) | ((CMain.m_bgG & 0xFF) << 8) | (CMain.m_bgB & 0xFF));
        for (int j = 0; j < m_iFontHeight; j++)
        {
            for (int i = (m_iMaxCharWidth - 1); i >= 0; i--)
            {
                if (charArray[i + j*m_iMaxCharWidth] != color)
                {
                    byteLineWidth = (byte)i;
                    break;
                }
            }
            
            if (byteLineWidth > byteMaxWidth)
            {
                byteMaxWidth = byteLineWidth;
            }
        }
        
        return (byte)(byteMaxWidth+1);
    }
    
    private static byte GetMaxHeight (int[] charArray)
    {
        byte byteMaxHeight = 0;
        byte byteLineHeight = 0;
        int color = ((0xFF<<24) | ((CMain.m_bgR&0xFF) << 16) | ((CMain.m_bgG & 0xFF) << 8) | (CMain.m_bgB & 0xFF));
        for (int j = 0; j < m_iMaxCharWidth; j++)
        {
            for (int i = (m_iFontHeight - 1); i >= 0; i--)
            {
                if (charArray[j + i*m_iMaxCharWidth] != color)
                {
                    byteLineHeight = (byte)i;
                    break;
                }
            }
            
            if (byteLineHeight > byteMaxHeight)
            {
                byteMaxHeight = byteLineHeight;
            }
        }
        
        return (byte)(byteMaxHeight+1);
    }
    
    private static boolean SetHeightAndDrawOffset (Object[] objCharArrayToPack, Font usingFont)
    {
        //Create buffer image
        BufferedImage imgBuffer = null;
        Graphics2D currGraph = null;
        int iFontHeight = usingFont.getSize ();
        int iImgSize = iFontHeight * 2;
        try
        {
            imgBuffer = new BufferedImage (iImgSize,iImgSize,BufferedImage.TYPE_INT_ARGB);
            currGraph = imgBuffer.createGraphics ();
            currGraph.setFont (usingFont);
        }
        catch (Exception ex)
        {
            System.out.println ("Error in create image.");
            return false;
        }
        
        //Draw all char in to one image
        int int_lY = iFontHeight + iFontHeight/3;
        Character currObjChar = null;
        char currCharToPack = ' ';
        currGraph.setColor (Color.WHITE);
        currGraph.fillRect (0, 0, iImgSize, iImgSize);
        currGraph.setColor (Color.BLACK);
        for (int i = 0; i < objCharArrayToPack.length; i++)
        {
            currObjChar = (Character)objCharArrayToPack[i];
            currCharToPack = currObjChar.charValue ();
            
            // patch to display the chinese '.' and ',' a bit lower not centered on row
            int offsetY = 0;
            if (currCharToPack == 0x3002 || currCharToPack == 0xFF0C )
            {
                offsetY += usingFont.getSize () / 4;
            }
            currGraph.drawString (Character.toString (currCharToPack), 1, int_lY + offsetY);
        }
        
        //Get RGB data of this image
        int[] imgArrayRGB = new int[iImgSize * iImgSize];
        imgBuffer.getRGB (0, 0, iImgSize, iImgSize, imgArrayRGB, 0, iImgSize);
        
        int iMinOffset = 0;
        int iMaxOffset = iImgSize - 1;
        boolean hasBreak = false;
        
        //Scan for min offset
        for (int j = 0; j < iImgSize; j++)
        {
            for (int i = (iImgSize - 1); i >= 0; i--)
            {
                if (imgArrayRGB[i + j * iImgSize] != -1)
                {
                    hasBreak = true;
                    break;
                }
            }
            
            if (hasBreak)
            {
                iMinOffset = j;
                break;
            }
        }
        
        //Scan for max offset
        hasBreak = false;
        for (int j = (iImgSize - 1); j >= 0; j--)
        {
            for (int i = (iImgSize - 1); i >= 0; i--)
            {
                if (imgArrayRGB[i + j * iImgSize] != -1)
                {
                    hasBreak = true;
                    break;
                }
            }
            
            if (hasBreak)
            {
                iMaxOffset = j;
                break;
            }
        }
        
        //Set font height
        m_iFontHeight = (iMaxOffset-iMinOffset) + 1;
        if (_BITS_PER_PIXEL_ == 2)
        {
            m_iFontHeight += 2;
        }
        m_iMaxCharWidth = m_iFontHeight * 2;
        //Set value Y of drawString func
        m_iTextPosY = int_lY - iMinOffset;
        
        return true;
    }
    
    public static boolean WriteSprite (DataOutputStream dos, String strOutName, int width, int height, int modules, int[] letterWidths,int[] letterHeights, Object[] objCharArrayToPack, int spaceOffsetX)
    {
        try
        {
            dos.writeBytes ("// saved by ChineseFontGenerator v1.0.0\n");
            dos.writeBytes ("////////////////////////////////////////////////////////////////////////////////\n");
            dos.writeBytes ("/*SPRITE*/ {\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    VERSION 0001\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    // Images:  1\n");
            dos.writeBytes ("    // Modules: " + modules + "\n");
            dos.writeBytes ("    // Frames:  1\n");
            dos.writeBytes ("    // Anims:   0\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("////////////////////////////////////////////////////////////////////////////////\n");
            dos.writeBytes ("// Images...\n");
            dos.writeBytes ("// <Image> := IMAGE [id] \"file\" [TRANSP transp_color]\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    IMAGE 0x0000 \"" + strOutName + ".png\" TRANSP 0x00FF00FF // 0  size: " + width + " x " + height + "  palettes: 1\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("////////////////////////////////////////////////////////////////////////////////\n");
            dos.writeBytes ("// Modules...\n");
            dos.writeBytes ("// <Modules> := MODULES { <MD1> <MD2> ... }\n");
            dos.writeBytes ("// <MDi>     := MD id Type [params] [\"desc\"]\n");
            dos.writeBytes ("// Type      := MD_IMAGE | MD_RECT | ...\n");
            dos.writeBytes ("// [params]  := if (Type == MD_IMAGE)     -> image x y width height\n");
            dos.writeBytes ("//              if (Type == MD_RECT)      -> color width height\n");
            dos.writeBytes ("//              if (Type == MD_FILL_RECT) -> color width height\n");
            dos.writeBytes ("//              if (Type == MD_ARC)       -> color width height\n");
            dos.writeBytes ("//              if (Type == MD_FILL_ARC)  -> color width height\n");
            dos.writeBytes ("//              if (Type == MD_MARKER)    -> color width height\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    MODULES\n");
            dos.writeBytes ("    {\n");
            
            int posx = CMain.m_addedWidth;
            int posy = CMain.m_addedWidth;
            
            int maxLineHeight = 0;
            
            for (int i=0; i<modules; i++)
            {
                Character currObjChar = (Character)objCharArrayToPack[i];
                
                if (letterHeights[i] > maxLineHeight)
                	maxLineHeight = letterHeights[i];
                
                if (posx + letterWidths[i] + CMain.m_addedWidth >= buffImageW)
                {
                    posx = CMain.m_addedWidth;
                    //posy += m_iFontHeight +  CMain.m_addedWidth;
                    posy += maxLineHeight +  CMain.m_addedMargin + CMain.m_addedWidth;
                    
                    maxLineHeight = 0;
                }
                
                dos.writeBytes ("        MD\t0x" + Integer.toHexString (0x1000+i).toUpperCase () + "\tMD_IMAGE\t0\t"
                        + String.valueOf(posx - (CMain.m_addedMargin))+ "\t" + String.valueOf(posy - (CMain.m_addedMargin)) + "\t" + String.valueOf(letterWidths[i] + (CMain.m_addedMargin * 2)) + "\t" + String.valueOf(letterHeights[i] + (CMain.m_addedMargin * 2))/*m_iFontHeight*/
                        + "\t\"0x" + Integer.toHexString (currObjChar.charValue ()).toUpperCase () + "\"\n");
                
                 posx += letterWidths[i] + CMain.m_addedWidth;
              
            }
            
            dos.writeBytes ("    }\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("////////////////////////////////////////////////////////////////////////////////\n");
            dos.writeBytes ("// Frames...\n");
            dos.writeBytes ("// <Frame> := FRAME [\"desc\"] { id <RC1> [<RC2> ...] <FM1> [<FM2> ...] }\n");
            dos.writeBytes ("// <RCi>   := RC x1 y1 x2 y2\n");
            dos.writeBytes ("// <FMi>   := FM module_or_frame_id ox oy [FLAGS hex_flags] [+Flags]\n");
            dos.writeBytes ("// Flags   := HYPER_FM | FLIP_X | FLIP_Y | ROT_90\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    FRAME \"font\" // Index = 0, FModules = " + modules + "\n");
            dos.writeBytes ("    {\n");
            dos.writeBytes ("        0x2000\n");
            dos.writeBytes ("        RC\t0\t0\t0\t0\n");
            for (int i=0; i<modules; i++)
            {
                int offsetX = 0;
                Character currObjChar = (Character)objCharArrayToPack[i];
                if (currObjChar.charValue () == ' ')
                {
                    offsetX = spaceOffsetX;
                }

		int index = i;
                if (CMain.m_hasUppercase)
                {
                	char c = currObjChar.charValue();
                	
                	c = Character.toUpperCase(c);
                	
                	index = FindModuleIndex(c, modules, objCharArrayToPack);
                	
                	if (index < 0) // check
                		index = i;
                }

               	dos.writeBytes ("        FM\t0x" + Integer.toHexString (0x1000 + index).toUpperCase () + "\t" + offsetX + "\t0\n");
            }
            dos.writeBytes ("    }\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("    SPRITE_END\n");
            dos.writeBytes ("\n");
            dos.writeBytes ("} // SPRITE\n");
            dos.writeBytes ("////////////////////////////////////////////////////////////////////////////////\n");
        }
        catch (Exception ex)
        {
            System.out.println ("Error in writing the sprite file :" + ex);
            return false;
        }
        
        return true;
    }    
    
    	public static int FindModuleIndex(char c, int modules, Object[] objCharArrayToPack)
    	{
    		for (int i=0; i<modules; i++)
            	{
	                int offsetX = 0;
	                Character currObjChar = (Character)objCharArrayToPack[i];
	                if (currObjChar.charValue () == c)
	                {
	                    return i;
	                }
	        }
	        
	        return -1;
	}
}

