
import java.io.*;
import java.util.*;
import java.awt.Font;

public class CEnumChar 
{

	public static final int _MIN_CHAR_EN_ = 33;
	public static final int _MIN_CHAR_JP = '\u3001' & 0xFFFF;
	
	public static Vector m_vEnumText = null;
	public static int _MIN_CHAR_ = _MIN_CHAR_EN_;
	
	public CEnumChar() 
	{
		// TODO Auto-generated constructor stub
	}

	public static boolean EnumCharacter(String strFileName, Font usingFont)
	{
		//Open JP text file
		FileInputStream inStream = null;
		try
		{
			inStream = new FileInputStream(strFileName);
		}
		catch (Exception ex)
		{
			System.out.println("File not found: " + strFileName);
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
		
		//Processing file
		try
		{
			int currChar =  readerStream.read();
			int currCharPos = 0;
			Character currObjChar = null;
			m_vEnumText = new Vector();
			
			while (currChar != -1)
			{
				//Skip Controlling-Chars or UTF-8 Flag Char (3 first bytes of UTF-8 text file)
				if (currChar < _MIN_CHAR_ || (currCharPos == 0 && currChar == 0xfeff))
				{
					currChar = readerStream.read();
					currCharPos++;
					continue;
				}
				
				currObjChar = new Character((char)currChar);
				
				// If character is not in enum:
				if (!IsInEnum(currObjChar, m_vEnumText))
				{
					// If character can display by font and not a white space
					if (usingFont.canDisplay(currObjChar.charValue()) && !Character.isWhitespace(currObjChar.charValue()) && !Character.isISOControl(currObjChar.charValue()))
					{
						m_vEnumText.addElement(currObjChar);
					}
					else
					{
						if (usingFont.canDisplay(currObjChar.charValue()) == false)
						{
							System.out.println("Warning: Font " + usingFont.getName() + " can't display character: \\u" + Integer.toHexString(currChar) + ". Char position: " + currCharPos);
						}
					}	
				}
				currChar = readerStream.read();
				currCharPos++;
			}
			readerStream.close();
		}
		catch (Exception ex)
		{
			System.out.println("Error in processing file...");
			return false;
		}
		
		return true;
	}
	
	public static boolean WriteOutput(String strOutFileName)
	{
		int iTotalEnumChar = m_vEnumText.size();
		Object[] currObjCharArray = m_vEnumText.toArray();
		Arrays.sort(currObjCharArray);
		//Open output file to write
		String strPackageName = strOutFileName;
		FileOutputStream outStream = null;
		try
		{
			outStream = new FileOutputStream(strPackageName);
		}
		catch (Exception ex)
		{
			System.out.println("Can't create file : " + strPackageName + " for writing output data.");
			return false;
		}
		
		//Open file writer
		OutputStreamWriter writerStream = null;
		try
		{
			writerStream = new OutputStreamWriter(outStream, "UTF-8");
		}
		catch (Exception ex)
		{
			return false;
		}
		
		Character currObjChar = null;
		try
		{	
			writerStream.write((char)iTotalEnumChar);
			writerStream.write("\n");
			for (int i = 0; i < iTotalEnumChar; i++)
			{
				currObjChar = (Character)currObjCharArray[i];
				writerStream.write(currObjChar.charValue());
				writerStream.write("\n");
			}
			writerStream.close();
		}
		catch (Exception ex)
		{
			return false;
		}
		
		return true;
	}
	
	private static boolean IsInEnum(Character objChar, Vector vEnum)
	{
		if (vEnum.isEmpty() == true)
			return false;
		
		return vEnum.contains(objChar);
	}
	
}
