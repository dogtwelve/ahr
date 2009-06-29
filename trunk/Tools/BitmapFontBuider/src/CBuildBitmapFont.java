
import java.io.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.Graphics2D;

public class CBuildBitmapFont
{
	public static int _BITS_PER_PIXEL_ = 1;
	
	private static int m_iFontHeight = 11;
	private static int m_iTextPosY = 10;
	private static int m_iMaxCharWidth = 20;
	private static int[] m_aiCharMatrix = null;
	
	public CBuildBitmapFont() 
	{
		// TODO Auto-generated constructor stub
	}
	
	public static boolean CreateFontPackage(Object[] objCharArrayToPack, Font usingFont, String strPackName)
	{
		//Open output file to write
		FileOutputStream outStream = null;
		try
		{
			outStream = new FileOutputStream(strPackName);
		}
		catch (Exception ex)
		{
			System.out.println("Can't create file : " + strPackName + " for writing output data.");
			return false;
		}
		
		//Specify font height in pixel and Y drawing offset
		SetHeightAndDrawOffset(objCharArrayToPack, usingFont);
		
		//Create buffer image
		BufferedImage imgBuffer = null;
		Graphics2D currGraph = null;
		try
		{
			imgBuffer = new BufferedImage(m_iMaxCharWidth,m_iFontHeight,BufferedImage.TYPE_INT_ARGB);
			currGraph = imgBuffer.createGraphics();
			currGraph.setFont(usingFont);
		}
		catch (Exception ex)
		{
			System.out.println("Error in create image.");
			return false;
		}
		
		//Create font package
		try
		{
			//Write number of JP text
			int iTotalCharsToPack = objCharArrayToPack.length;
			byte[] 	byteUnicode = new byte[2];
			byteUnicode[0] = (byte)(iTotalCharsToPack&0xFF);
			byteUnicode[1] = (byte)(iTotalCharsToPack>>8);
			outStream.write(byteUnicode);
			
			//Write font Height
			outStream.write(m_iFontHeight);
			
			//Write font encoded:
			outStream.write(_BITS_PER_PIXEL_);
			
			
			byte 	byteFontWidth = 0;
			byte[] 	byteJPFont = new byte[((m_iMaxCharWidth * m_iFontHeight) >> (4-_BITS_PER_PIXEL_)) + 1];
			m_aiCharMatrix = new int[m_iFontHeight * m_iMaxCharWidth];
			Character currObjChar = null;
			char currCharToPack = ' ';
			//int int_lTmp = 0;
			
			for (int i=0;i<objCharArrayToPack.length;i++)
			{
				currObjChar = (Character)objCharArrayToPack[i];
				currCharToPack = currObjChar.charValue();
				//int_lTmp = currCharToPack;
				
				//Write Unicode of char to output file
				byteUnicode[0] = (byte)(currCharToPack & 0xFF);
				byteUnicode[1] = (byte)(currCharToPack >> 8);
				outStream.write(byteUnicode);

				// patch to display the chinese '.' and ',' a bit lower not centered on row
				int offsetY = 0;
				if (currCharToPack == 0x3002 || currCharToPack == 0xFF0C )
				{
					offsetY += usingFont.getSize() / 4;
				}
				
				//Draw char to image buf
				if (_BITS_PER_PIXEL_ == 2)
				{
					currGraph.setColor(Color.WHITE);
					currGraph.fillRect(0, 0, m_iMaxCharWidth, m_iFontHeight);
					currGraph.setColor(Color.BLUE);
					//Border top
					currGraph.drawString(Character.toString(currCharToPack), 0, m_iTextPosY+offsetY);
					currGraph.drawString(Character.toString(currCharToPack), 1, m_iTextPosY+offsetY);
					currGraph.drawString(Character.toString(currCharToPack), 2, m_iTextPosY+offsetY);
					
					//Border bottom
					currGraph.drawString(Character.toString(currCharToPack), 0, m_iTextPosY+offsetY+2);
					currGraph.drawString(Character.toString(currCharToPack), 1, m_iTextPosY+offsetY+2);
					currGraph.drawString(Character.toString(currCharToPack), 2, m_iTextPosY+offsetY+2);
					
					//Border left, right
					currGraph.drawString(Character.toString(currCharToPack), 0, m_iTextPosY+offsetY+1);
					currGraph.drawString(Character.toString(currCharToPack), 2, m_iTextPosY+offsetY+1);
					
					//Main char
					currGraph.setColor(Color.BLACK);
					currGraph.drawString(Character.toString(currCharToPack), 1, m_iTextPosY+offsetY+1);
				}
				else
				{
					currGraph.setColor(Color.WHITE);
					currGraph.fillRect(0, 0, m_iMaxCharWidth, m_iFontHeight);
					currGraph.setColor(Color.BLACK);
					currGraph.drawString(Character.toString(currCharToPack), 0, m_iTextPosY+offsetY);
				}
				
				//Encode image of character
				byteFontWidth = CompressTextImage(byteJPFont, imgBuffer);
				
				//Write char width
				outStream.write(byteFontWidth);
				//Write encoded data
				outStream.write(byteJPFont,0,((byteFontWidth*m_iFontHeight)>>(4-_BITS_PER_PIXEL_)) + 1);
				
			}
			
			outStream.close();
		}
		catch (Exception ex)
		{
			System.out.println("Error in processing JP text... :" + ex);
			return false;
		}
		
		System.out.println("Build font package successfully !");
		return true;
	}
	
	private static byte CompressTextImage(byte[] outputArray, BufferedImage inputImgBuffer)
	{
		byte byteWidth = 0;
		
		inputImgBuffer.getRGB(0, 0, m_iMaxCharWidth, m_iFontHeight, m_aiCharMatrix, 0, m_iMaxCharWidth);
		
		//Delete unnecessary space before character
		DeleteSpaceBeforeChar(m_aiCharMatrix);
		
		byteWidth = GetMaxWidth(m_aiCharMatrix);
		
		int iTotalPixel = byteWidth * m_iFontHeight;
		int iTotalBytes = (iTotalPixel >> (4 - _BITS_PER_PIXEL_)) + 1;
		
		byte byteTemp = 0;
		byte bytePack = 0;
		int iPixelIndex1 = 0;
		int iPixelIndex2 = 0;
		int iBitToShift = 0;
		int iPixelPerByte = 8 / _BITS_PER_PIXEL_;
		// for each pack byte
		for (int i = 0; i < iTotalBytes; i++)
		{
			bytePack = 0;
			byteTemp = 0;
			// for each pixel in 4 pixel of pack byte
			for (int j = 0; j < iPixelPerByte; j++)
			{
				// if out of pixel number : break
				iPixelIndex1 = i * iPixelPerByte + j;
				if ( iPixelIndex1 >= iTotalPixel)
					break;
				
				//else get value of pixel
				iPixelIndex2 = (iPixelIndex1 / byteWidth) * m_iMaxCharWidth + (iPixelIndex1 % byteWidth);
				iBitToShift = j * _BITS_PER_PIXEL_;
				
				if (m_aiCharMatrix[iPixelIndex2] == 0xFF000000)	//if main area of char
				{
					byteTemp = 0x01;
					bytePack = (byte)(bytePack|(byteTemp << iBitToShift));
				}
				else if (m_aiCharMatrix[iPixelIndex2] != -1)//if border of char
				{
					byteTemp = 0x02;
					bytePack = (byte)(bytePack|(byteTemp << iBitToShift));
				}
			}
			
			outputArray[i] = bytePack;
		}
		
		return byteWidth;
	}
	
	private static void DeleteSpaceBeforeChar(int[] charArray)
	{
		//Find space width
		int iSpaceW = 0;
		boolean isSpace = false;
		// For each column
		for (int i = 0; i < m_iMaxCharWidth; i++)
		{
			isSpace = true;
			// For each row
			for (int j = 0; j < m_iFontHeight; j++)
			{
				if (charArray[j*m_iMaxCharWidth + i] != -1)
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
					charArray[m_iMaxCharWidth*i + j]  = -1;
				}
			}
		}
	}
	
	private static byte GetMaxWidth(int[] charArray)
	{
		byte byteMaxWidth = 0;
		byte byteLineWidth = 0;
		
		for (int j = 0; j < m_iFontHeight; j++)
		{
			for (int i = (m_iMaxCharWidth - 1); i >= 0; i--)
			{
				if (charArray[i + j*m_iMaxCharWidth] != -1)
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
	
	private static boolean SetHeightAndDrawOffset(Object[] objCharArrayToPack, Font usingFont)
	{
		//Create buffer image
		BufferedImage imgBuffer = null;
		Graphics2D currGraph = null;
		int iFontHeight = usingFont.getSize();
		int iImgSize = iFontHeight * 2;
		try
		{
			imgBuffer = new BufferedImage(iImgSize,iImgSize,BufferedImage.TYPE_INT_ARGB);
			currGraph = imgBuffer.createGraphics();
			currGraph.setFont(usingFont);
		}
		catch (Exception ex)
		{
			System.out.println("Error in create image.");
			return false;
		}
		
		//Draw all char in to one image
		int int_lY = iFontHeight + iFontHeight/3;
		Character currObjChar = null;
		char currCharToPack = ' ';
		currGraph.setColor(Color.WHITE);
		currGraph.fillRect(0, 0, iImgSize, iImgSize);
		currGraph.setColor(Color.BLACK);
		for (int i = 0; i < objCharArrayToPack.length; i++)
		{
			currObjChar = (Character)objCharArrayToPack[i];
			currCharToPack = currObjChar.charValue();

			// patch to display the chinese '.' and ',' a bit lower not centered on row
			int offsetY = 0;
			if (currCharToPack == 0x3002 || currCharToPack == 0xFF0C )
			{
				offsetY += usingFont.getSize() / 4;
			}
			currGraph.drawString(Character.toString(currCharToPack), 1, int_lY + offsetY);
		}
		
		//Get RGB data of this image
		int[] imgArrayRGB = new int[iImgSize * iImgSize];
		imgBuffer.getRGB(0, 0, iImgSize, iImgSize, imgArrayRGB, 0, iImgSize);
		
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
	
}
