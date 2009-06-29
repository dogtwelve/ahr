import java.awt.Font;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.Arrays;

public class CMain 
{
	//Vars to store input params
	private static String m_strFontDefFileName = "";
	private static String m_strTextFileName = "";
	private static String m_strPackFileName = "";
	private static boolean m_hasBorder = false;
	private static String m_strFontName = "";
	private static int m_iFontType = 0;
	private static int m_iFontSize = 12;
		
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
		System.out.println("             BitmapFontBuilder              ");
		System.out.println("               Version 1.0.2                ");
		System.out.println("        Copyright (c) 2006 by Gameloft.     ");
		System.out.println();
		System.out.println();
		System.out.println("Author(s): Nguyen Trung Hung, Do Gia Cuong. ");
		System.out.println("Email: hungtrung.nguyen@gameloft.com, cuonggia.do@gameloft.com.");
		System.out.println();
		System.out.println();
		System.out.println();
		System.out.println();
		
		//Help
		if (args.length <= 0 || (args.length == 1 && args[0].compareTo("/?") == 0))
		{
			//Description and title
			System.out.println("Make font package for characters in text file (encoded by UTF-8).");
			System.out.println("Input params: -f fontdefine [-border] -t textfile -o outfile");
			System.out.println();
			System.out.println("  Params\t\t\tDescription");
			
			//Font define param
			System.out.println("  fontdefine\t\t\tFile contains font define. File structure:");
			System.out.println("\t\t\t\tthe first line is font NAME. Ex: MS Gothic.");
			System.out.println("\t\t\t\tthe second line is font TYPE. Ex: N (for normal), B (for Bold) and I (for Italic).");
			System.out.println("\t\t\t\tthe third line is font SIZE. Ex: 12.");
			
			//Border param
			System.out.println();
			System.out.println("  -border\t\t\tIf enter this param, character will have a border.");
			System.out.println();
			
			//text param
			System.out.println();
			System.out.println("  textfile\t\t\tFilename of localization text of game (encoded by UTF-8).");
			System.out.println();
			
			
			//out param
			System.out.println();
			System.out.println("  outfile\t\t\tFilename of output font package.");
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
				
				if (args[j + 1].compareTo("-o") == 0 || args[j + 1].compareTo("-t") == 0 || args[j + 1].compareTo("-border") == 0)
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
					m_strPackFileName = args[j + 1];
					j++;
				}
			}
			
			if (args[j].compareTo("-border") == 0)
			{
				m_hasBorder = true;
			}
		}
			
		if (m_strFontDefFileName.length() == 0 || m_strTextFileName.length() == 0 || m_strPackFileName.length() == 0)
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
				try
				{
					usingFont = new Font(m_strFontName, m_iFontType, m_iFontSize);
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
		CBuildBitmapFont.CreateFontPackage(objCharArray, usingFont, m_strPackFileName);
	
	}
	
	private static boolean LoadFontInfo(String strFontFileName)
	{
		String strFontDefLine[] = {"", "", "", ""};
		
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
						if (i >= 4)
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
		return true;
	}

}
