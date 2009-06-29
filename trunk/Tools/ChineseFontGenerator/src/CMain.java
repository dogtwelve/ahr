import java.awt.Font;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.Arrays;

public class CMain 
{
	//Vars to store input params
	public static String m_strFontDefFileName = "";
	public static String m_strTextFileName = "";
	public static String m_strOutputFileName = "";
	public static boolean m_hasBorder = false;
	public static boolean m_hasUppercase = false;
	public static String m_strFontName = "";
	public static int m_iFontType = 0;
	public static int m_iFontSize = 12;
	public static int m_iSpaceOffsetX = 0;
	public static int m_iSpaceWidth = 1;
        public static int m_addedWidth = 0;
        public static int m_addedMargin = 0;
        
        public static int m_outlineR = 0;
        public static int m_outlineG = 0;
        public static int m_outlineB = 0;
        
        public static int m_bgR = 0;
        public static int m_bgG = 0;
        public static int m_bgB = 0;
        
        public static int m_innerR = 255;
        public static int m_innerG = 255;
        public static int m_innerB = 255;
        
        public static boolean m_useAntialiasing = false;

	public static void QuickSort(Font []fonts, int l, int r)
        {
                int i = l;
                int j = r;
                int x = fonts[(l+r)/2].getNumGlyphs();
                do
                {
                        while ((i < r) && (fonts[i].getNumGlyphs() < x)) i++;
                        while ((j > l) && (x < fonts[j].getNumGlyphs())) j--;
                        if (i <= j) 
                        {
                                Font tmp = fonts[i]; 
                                fonts[i] = fonts[j]; 
                                fonts[j] = tmp;
                                i++;
                                j--;
                        }
                } while (i <= j);
                if (l < j) QuickSort(fonts, l, j);
                if (i < r) QuickSort(fonts, i, r);
        }
        
        
	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
		// TODO Auto-generated method stub
		
		//Info
		System.out.println();
		System.out.println();
		System.out.println();
		System.out.println();
		System.out.println("           Chinese Font Generator           ");
		System.out.println("               Version 1.0.0                ");
		System.out.println("        Copyright (c) 2007 by Gameloft.     ");
		System.out.println();
		System.out.println();
		System.out.println("Author(s): Nguyen Trung Hung, Do Gia Cuong, Constantin-Ionut Ene. ");
		System.out.println("Email: hungtrung.nguyen@gameloft.com, cuonggia.do@gameloft.com, constantinionut.ene@gameloft.com.");
		System.out.println();
		System.out.println();
		System.out.println();
		System.out.println();
		
		//Help
		if (args.length <= 0 || (args.length == 1 && args[0].compareTo("/?") == 0))
		{
			//Description and title
			System.out.println("Generate chinese font for the unique characters in a text file (encoded by UTF-16).");
			System.out.println("Input params: -f fontdefine [-border] -t textfile -o outname");
			System.out.println();
			System.out.println("  Params\t\t\tDescription");
			
			//Font define param
			System.out.println("  fontdefine\t\t\tFile contains font define. File structure:");
			System.out.println("\t\t\t\tthe first line is font NAME. Ex: MS Gothic.");
			System.out.println("\t\t\t\tthe second line is font TYPE. Ex: N (for normal), B (for Bold) and I (for Italic).");
			System.out.println("\t\t\t\tthe third line is font SIZE. Ex: 12.");
			System.out.println("\t\t\t\tthe forth line is the X offset of space character. Ex: -2.");
			System.out.println("\t\t\t\tthe fifth line is the width of the space character. Ex: 10.");
			
			//Border param
			System.out.println();
			System.out.println("  -border\t\t\tIf enter this param, character will have a border.");
			System.out.println();
			
			//text param
			System.out.println();
			System.out.println("  textfile\t\t\tFilename containing the localization texts of game (encoded by UTF-16).");
			System.out.println();
						
			//out param
			System.out.println();
			System.out.println("  outname\t\t\tFilename without extension. ");
                        System.out.println("         \t\t\t\toutname.png will be the image");
                        System.out.println("         \t\t\t\toutname.sprite will be the sprite");
                        System.out.println("         \t\t\t\toutname.txt will be the text file with the unique characters ordered as in the image (UTF-16)");
			System.out.println();
			
			return;
		}
			
				
		//Check input params
		for (int j = 0; j<args.length; j++)
		{
			if (args[j].compareTo("-f") == 0)
			{
				if (j + 1 == args.length)
				{
					System.out.println("Please enter font define filename.");
					System.exit(1);
					
				}
				
				if (args[j + 1].compareTo("-o") == 0 || args[j + 1].compareTo("-t") == 0 || args[j + 1].compareTo("-border") == 0 ||
					args[j + 1].compareTo("-uppercase") == 0)
				{
					System.out.println("Please enter font define filename.");
					System.exit(1);
					
				}
				else
				{
					m_strFontDefFileName = args[j + 1];
					j++;
				}
			}
			
			if (args[j].compareTo("-t") == 0)
			{
				if (j + 1 == args.length)
				{
					System.out.println("Please enter text file.");
					System.exit(1);
					
				}
				
				if (args[j + 1].compareTo("-f") == 0 || args[j + 1].compareTo("-o") == 0 || args[j + 1].compareTo("-border") == 0)
				{
					System.out.println("Please enter text file.");
					System.exit(1);
					
				}
				else
				{
					m_strTextFileName = args[j + 1];
					j++;
				}
			}
			
			if (args[j].compareTo("-o") == 0)
			{
				if (j + 1 == args.length)
				{
					System.out.println("Please enter text file.");
					System.exit(1);
				}
				
				if (args[j + 1].compareTo("-f") == 0 || args[j + 1].compareTo("-p" ) == 0 || args[j + 1].compareTo("-border") == 0)
				{
					System.out.println("Please enter output filename.");
					System.exit(1);
				}
				else
				{
					m_strOutputFileName = args[j + 1];
					j++;
				}
			}

                        if (args[j].compareTo("-border") == 0)
			{
				m_hasBorder = true;
			}
			
			if (args[j].compareTo("-uppercase") == 0)
			{
				m_hasUppercase = true;
			}
                        
                        if (args[j].compareTo("-antialias") == 0)
			{
				m_useAntialiasing = true;
			}
		}
			
		if (m_strFontDefFileName.length() == 0 || m_strTextFileName.length() == 0 || m_strOutputFileName.length() == 0)
		{
			System.out.println("Please enter all params.");
			System.exit(1);
		}
		
		if (m_hasBorder)
		{
			CBuildBitmapFont._BITS_PER_PIXEL_ = 2;
		}
		else
		{
			CBuildBitmapFont._BITS_PER_PIXEL_ = 1;
		}
		
		//Load font info
		Font usingFont = null;
		if (m_strFontDefFileName.length() > 0)
		{
			if (LoadFontInfo(m_strFontDefFileName))
			{	
                                Font []fff = java.awt.GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts();
                                //QuickSort(fff, 0, fff.length - 1);
                                System.out.println("Fonts Available.... : " + fff.length);
                                //for(int i = 0; i < fff.length; i++)
                                //       System.out.println("::" + fff[i].getFontName() + "::" + fff[i].getNumGlyphs());
                                System.out.println("END Fonts Available.... \n");
                                
                                System.out.println("Font selected: " + m_strFontName + "    Size: " + m_iFontSize);
                                
				try
				{
					usingFont = new Font(m_strFontName, m_iFontType, m_iFontSize);
                                        System.out.println("Font used: " + usingFont.getFontName());
				}
				catch (Exception ex)
				{
					System.out.println("Font not found...");
					System.exit(1);
				}
			}
			else
			{
				System.out.println("Load font define file failed...");
				System.exit(1);
			}
		}
		
		//Enumerate character
		boolean isEnumOK = true;
		isEnumOK = CEnumChar.EnumCharacter(m_strTextFileName, usingFont);
		if (!isEnumOK)
		{
			System.exit(1);
		}
		
		//Sort char list
		Object[] objCharArray = CEnumChar.m_vEnumText.toArray();
		Arrays.sort(objCharArray);
		
		System.out.println("Total char = " + objCharArray.length);
		
		//Build font package
		CBuildBitmapFont.CreateFontPackage(objCharArray, usingFont, m_strOutputFileName, m_iSpaceOffsetX, m_iSpaceWidth);
	}
	
	private static boolean LoadFontInfo(String strFontFileName)
	{
		String strFontDefLine[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
		
		//	Open file
		FileInputStream inStream = null;
		try
		{
			inStream = new FileInputStream(strFontFileName);
		}
		catch (Exception ex)
		{
			System.out.println("File not found: " + strFontFileName);
			return false;
		}

		//Open file reader
		InputStreamReader readerStream = null;
		try
		{
			readerStream = new InputStreamReader(inStream, "UTF-8");
		}
		catch (Exception ex)
		{
			return false;
		}
		
		//Read file
		try
		{
			char currChar = ' ';
			int i = 0;
			int nextChar =  readerStream.read();
			
			while (nextChar != -1)
			{
				currChar = ((char)nextChar);
				if (nextChar == 10 || nextChar == 13)
				{
					if (nextChar == 13)
					{
						i++;
						if (i >= strFontDefLine.length)
						{
							break;
						}
					}
				}
				else
				{
					strFontDefLine[i] = strFontDefLine[i] + currChar;
				}
				nextChar = readerStream.read();
			}
			readerStream.close();
			inStream.close();
		}
		catch (Exception ex)
		{
			System.out.println("Error in processing file...");
			return false;
		}
		
		m_strFontName = strFontDefLine[0];
		//Get font size and font height in pixel
		try
		{
			m_iFontSize = Integer.valueOf(strFontDefLine[2]).intValue();
			m_iSpaceOffsetX = Integer.valueOf(strFontDefLine[3]).intValue();
			m_iSpaceWidth = Integer.valueOf(strFontDefLine[4]).intValue();
		}
		catch (Exception ex)
		{
			return false;
		}
		
		m_iFontType = 0;
		for (int i = 0; i < strFontDefLine[1].length(); i++)
		{
			if (strFontDefLine[1].charAt(i) == 'B' || strFontDefLine[1].charAt(i) == 'b')
			{
				if ((m_iFontType&Font.PLAIN) == 1)
				{
					m_iFontType = m_iFontType&(~Font.PLAIN);
				}
				
				m_iFontType = m_iFontType|Font.BOLD;
				
			}
			else if (strFontDefLine[1].charAt(i) == 'N' || strFontDefLine[1].charAt(i) == 'n')
			{
				if ((m_iFontType&Font.BOLD) == 1)
				{
					m_iFontType = m_iFontType&(~Font.BOLD);
				}
				if ((m_iFontType&Font.ITALIC) == 1)
				{
					m_iFontType = m_iFontType&(~Font.ITALIC);
				}
				m_iFontType = m_iFontType|Font.PLAIN;
			}
			else if (strFontDefLine[1].charAt(i) == 'I' || strFontDefLine[1].charAt(i) == 'i')
			{
				if ((m_iFontType&Font.PLAIN) == 1)
				{
					m_iFontType = m_iFontType&(~Font.PLAIN);
				}
				m_iFontType = m_iFontType|Font.ITALIC;
                                
			}
			else
			{
				return false;
			}
		}
                
                  m_addedWidth = Integer.valueOf(strFontDefLine[5]).intValue();
                  
                  m_addedMargin = Integer.valueOf(strFontDefLine[6]).intValue();
               
                  if (strFontDefLine[7] != "")
                  {
                    m_innerR = Integer.valueOf(strFontDefLine[7]).intValue();
                  }
                  
                  if (strFontDefLine[8] != "")
                  {
                   m_innerG = Integer.valueOf(strFontDefLine[8]).intValue();
                  }
                  
                   if (strFontDefLine[9] != "")
                  {
                      
                    m_innerB = Integer.valueOf(strFontDefLine[9]).intValue();
                  }
                  
                   if (strFontDefLine[10] != "")
                  {
                    m_bgR = Integer.valueOf(strFontDefLine[10]).intValue();
                  }
                  
                  if (strFontDefLine[11] != "")
                  {
                   m_bgG = Integer.valueOf(strFontDefLine[11]).intValue();
                  }
                  
                   if (strFontDefLine[12] != "")
                  {
                      
                    m_bgB = Integer.valueOf(strFontDefLine[12]).intValue();
                  }
                  
                 if (strFontDefLine[13] != "")
                  {
                    m_outlineR= Integer.valueOf(strFontDefLine[13]).intValue();
                  }
                  
                  if (strFontDefLine[14] != "")
                  {
                      
                    m_outlineG = Integer.valueOf(strFontDefLine[14]).intValue();
                  }
                  
                   if (strFontDefLine[15] != "")
                  {
                   m_outlineB = Integer.valueOf(strFontDefLine[15]).intValue();
                  }
                
		return true;
	}

}
