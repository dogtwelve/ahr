import java.awt.Image;
import java.awt.Toolkit;
import java.awt.image.ImageObserver;
import java.awt.image.PixelGrabber;
import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import javax.swing.ImageIcon;

import sun.nio.cs.ISO_8859_2;


class ModuleCoordinates {
	int x;
	int y;
	int width;
	int height;
	public ModuleCoordinates(int x, int y, int width, int height) {
		super();
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}
	boolean isEqualToModule(ModuleCoordinates moduleToCompare)
	{
		if (this.width==moduleToCompare.width &&
			this.height==moduleToCompare.height)
		{
			return true;			
		}
		return false;
	}
	public int getHeight() {
		return height;
	}
	public int getWidth() {
		return width;
	}
	public int getX() {
		return x;
	}
	public int getY() {
		return y;
	}
	
	
}

public class FontGenerator {
	private static final int CHAR_SPACE = 32;


	private static final int MAX_MODULES = 5000;	
	int fPixelsBuffer[];
	int fNumberColorsInPalette;

	int fPalette[];

	int fWidth;

	int fHeight;
	
	int fCurrentModule;
	

	int[] fFontCharactersString;
	
	ModuleCoordinates fModuleCoordinates[];
	

	public FontGenerator() {
		super();		

		fWidth = 0;
		
	}

	int getAlpha(int color24) {
		int alpha = ((color24 >> 24) & 0xff);
		return alpha;
	}

	int convert24to16(int color24) {

		int red = ((color24 >> 16) & 0xff);
		int green = ((color24 >> 8) & 0xff);
		int blue = ((color24) & 0xff);
		int result = 0;

		result = result | ((red >> 3) << 11);
		result = result | ((green >> 2) << 5);
		result = result | ((blue >> 3));

		return result;
	}



	void writeIntToStream(BufferedOutputStream stream, int valueToWrite)
			throws IOException {
		stream.write(valueToWrite);
		stream.write(valueToWrite >> 8);
		stream.write(valueToWrite >> 16);
		stream.write(valueToWrite >> 24);
	}

	void writeShortToStream(BufferedOutputStream stream, int valueToWrite)
			throws IOException {
		stream.write(valueToWrite);
		stream.write(valueToWrite >> 8);
	}


	
	boolean isPixelEmpty(int pixel24)
	{
		return (fPixelsBuffer[0] == pixel24);
	}
	boolean isPixelColumnEmpty(int columnX)
	{
		for (int y=0;y<fHeight;y++)
		{
			if (!isPixelEmpty(fPixelsBuffer[y*fWidth + columnX]))
			{
				return false;
			}			
		}
		return true;
	}
	boolean isPixelRowEmpty(int y,int startX,int endX)
	{
		for (int x=startX;x<=endX;x++)
		{
			if (!isPixelEmpty(fPixelsBuffer[y*fWidth + x]))
			{
				return false;
			}			
		}
		
		return true;
	}
	int findTopBorderOfCharacter(int startX,int endX)
	{

		for (int i=0;i<fHeight;i++)
		{
			if (!isPixelRowEmpty(i,startX,endX))
			{
				return i;
			}
		}
		return -1;
	}
	
	int findBottomBorderOfCharacter(int startX,int endX)
	{

		for (int i=fHeight-1;i>=0;i--)
		{
			if (!isPixelRowEmpty(i,startX,endX))
			{
				return i;
			}
		}
		return -1;
	}
	
	void findCharacters()
	{
		int lastNonEmptyColumn = -1;
		boolean shapeStarted=false;
		int shapeStartedX = -1;
		int shapesCount = 0;
		for (int i=0;i<fWidth;i++)
		{
			boolean currentColumnEmpty = isPixelColumnEmpty(i);
			if (shapeStarted)
			{
				if (currentColumnEmpty)
				{
					if (i - lastNonEmptyColumn > fHorizontalThreshold)
					{
						int startX = shapeStartedX;						
						int endX = (i-1);
						int startY = findTopBorderOfCharacter(shapeStartedX,endX);
						int endY = findBottomBorderOfCharacter(shapeStartedX,endX);
						
						fModuleCoordinates[fCurrentModule] = new ModuleCoordinates(startX,startY,endX-startX,endY-startY+1);
						fCurrentModule++;
						shapeStarted = false;	
						shapesCount++;
					}
				}			
			}
			else
			{
				if (!currentColumnEmpty)
				{
					shapeStarted = true;
					shapeStartedX = i;
				}				
			}
			if (!currentColumnEmpty)
			{
				lastNonEmptyColumn = i;
			}
		}
		System.out.println("Shapes count:" + shapesCount);
	}
	void checkFoundModuleCoordinates() throws Exception
	{
		if (fCurrentModule<1) return;
		if (fCurrentModule!=fFontCharactersString.length)
				throw new Exception("Number of modules different in the characters string. Modules found:"+fCurrentModule+" .Characters in the string:"+fFontCharactersString.length);
		ModuleCoordinates firstModule = fModuleCoordinates[0];
		boolean differentSeparatorsMessagePrinted=false;
		for (int i = 2;i<fCurrentModule; i +=2)
		{
			if (!firstModule.isEqualToModule(fModuleCoordinates[i])){
				if (!differentSeparatorsMessagePrinted)
				{
					System.out.println("Warning:Some separator modules are with different sizes!");
					differentSeparatorsMessagePrinted = true;
				}
				//throw new Exception("Separator modules not ok.");				
			}
		}
		System.out.println("Module coordinates check passed successfully.");
		
	}
	boolean isEmptyCharacter(byte character)
	{
		if (character==10 ||  
				character==13 || //enter
				character==32 //space
				)
		{
			return true;
		}
		return false;
	}
	int[] readFontCharacterStringFromFile(String fileName) throws IOException
	{
		FileInputStream inputStream = new FileInputStream(fileName);
		byte[] fileContent = new byte[inputStream.available()];
		
		inputStream.read(fileContent);
		
		//pass 1 , count the special characters which will be removed
		int count = 0;
		for (int i=0;i<fileContent.length;i++)
		{
			byte current = fileContent[i];
			if (isEmptyCharacter(current))
			{
				count++;
			}
		}
		//pass 2, generating the result
		int[] result = new int[fileContent.length - count];
		int resultIndex=0;
		for (int i=0;i<fileContent.length;i++)
		{
			if (!isEmptyCharacter(fileContent[i]))
			{
				//conversion to unsigned
				result[resultIndex] = ((int)(fileContent[i] & 0xFF));
//				if (fileContent[i]<0)
//					result[resultIndex] = (127-Math.abs(fileContent[i])) +127;					
//				else
//					result[resultIndex] = fileContent[i];
				
				//if (result[resultIndex] != (int)(fileContent[i] & 0xFF)) 
					//System.out.println(result[resultIndex] + " != " + ((int)(fileContent[i] & 0xFF)));

					
				resultIndex++;
			}
		}
		fileContent = null;
		return result;
	}
	void writeSpriteFile(String imageFile,String outputSpriteFile, String imageFileNameForSpriteFile) throws IOException
	{
		FileWriter fwriter = new FileWriter(outputSpriteFile,false);
		try
		{
			fwriter.write("{\n");
			fwriter.write("VERSION 0001\n");			
			
			fwriter.write("IMAGE 0x0000 \""+imageFileNameForSpriteFile+"\" TRANSP 0x00FF00FF\n");
			writeModules(fwriter);
			writeFrames(fwriter);
			
			fwriter.write("SPRITE_END\n");		    
			fwriter.write("}\n");			

		}
		finally
		{
			fwriter.close();
			fwriter = null;
		}
	}

	private void writeModules(FileWriter fwriter) throws IOException {
		fwriter.write("\tMODULES\n");
		fwriter.write("\t\t{\n");

		int maxModuleHeight=0;
		int sumWidths = 0;
		int countModules = 0;
		for (int i = 1;i<fCurrentModule; i +=2)
		{
			ModuleCoordinates module = fModuleCoordinates[i];
			if (module.height>maxModuleHeight) maxModuleHeight = module.height;
			sumWidths+=module.width;
			countModules++;
		}
		int averageModuleWidth = sumWidths/countModules;
		writeModuleToSpriteFile(fwriter, 0, new ModuleCoordinates(0,0,averageModuleWidth,maxModuleHeight+1), CHAR_SPACE); //space
		
		for (int i = 1;i<fCurrentModule; i +=2)
		{
			ModuleCoordinates module = fModuleCoordinates[i];
			int currentModuleCode = fFontCharactersString[i];
			writeModuleToSpriteFile(fwriter, i, module, currentModuleCode);				
		}
		fwriter.write("\t\t}\n");
	}
	int getHCharSpacing(int moduleIdx)
	{
		return fModuleCoordinates[moduleIdx].getX() 
				- (fModuleCoordinates[moduleIdx-1].getX()+fModuleCoordinates[moduleIdx-1].getWidth());
	}
	int getVCharSpacing(int moduleIdx)
	{
		return fModuleCoordinates[moduleIdx].getY(); 		
	}
	private void writeFrames(FileWriter fwriter) throws IOException {
		int minHCharSpacing=99999;
		int minVCharSpacing=99999;		
		for (int moduleIdx = 1;moduleIdx<fCurrentModule; moduleIdx +=2)
		{
			int currentModuleHorizontalCharSpacing = getHCharSpacing(moduleIdx);
			if (currentModuleHorizontalCharSpacing < minHCharSpacing)
			{
				minHCharSpacing = currentModuleHorizontalCharSpacing;
			}
			
			int currentModuleVerticalCharSpacing = getVCharSpacing(moduleIdx);			
			if (currentModuleVerticalCharSpacing < minVCharSpacing)
			{
				minVCharSpacing = currentModuleVerticalCharSpacing;
			}
			
		}

		fwriter.write("\tFRAME \"ALL\"\n");
		
		
		fwriter.write("\t\t{\n");
		fwriter.write("\t\t\t0x2000\n");			
		fwriter.write("\t\t\tFM	0x1000	0	0\n");

		for (int frameIdx=1;frameIdx<256;frameIdx++)
		{
			int corespondingCharCode = CHAR_SPACE;
			//Find what character must be here
			
			
			for (int charCode=0;charCode<FontMap.MAP.length;charCode++)
			{
				if (FontMap.MAP[charCode]==frameIdx) 
					{
						//Note that for correct font generation the FontMap.MAP
						//should not contain repeating module indexes except for the space
						//which is 0
						corespondingCharCode=charCode;
						break;
					}
				
			}
			if (corespondingCharCode == CHAR_SPACE)
			{
				writeSpaceFrameModule(fwriter);					
			}
			else
			{
				int moduleIndex=0;
				for (int i = 1;i<fCurrentModule; i +=2)
				{
					if (corespondingCharCode == fFontCharactersString[i])
						moduleIndex = i;
				}
				if (moduleIndex==0)
				{
					writeSpaceFrameModule(fwriter);
				}
				else
				{
					/*
					fwriter.write("\t\t\tFM	0x"+Integer.toHexString(0x1000+moduleIndex)+"	" +
							(getHCharSpacing(moduleIndex)  - minHCharSpacing + fOptionalHorizontalOffset) + "	"+
							+(getVCharSpacing(moduleIndex) - minVCharSpacing + fOptionalVerticalOffset) +
							"\n");
					*/
					fwriter.write("\t\t\tFM	0x"+Integer.toHexString(0x1000+moduleIndex)+"	" +
							 "0	"+
							+(getVCharSpacing(moduleIndex) - minVCharSpacing + fOptionalVerticalOffset) +
							"\n");
					
				}
			}
		}
		fwriter.write("\t\t}\n");
	}

	private void writeSpaceFrameModule(FileWriter fwriter) throws IOException {
		fwriter.write("\t\t\tFM	0x1000	0	0\n");
	}

	private void writeModuleToSpriteFile(FileWriter fwriter, int moduleID, ModuleCoordinates module, int currentModuleCode) throws IOException {
		fwriter.write("\t\t\tMD\t0x" + Integer.toHexString(moduleID+0x1000) + 
					  "\tMD_IMAGE\t0\t"+module.getX()+
					  "\t"+module.getY()+
					  "\t"+module.getWidth()+							  
					  "\t"+module.getHeight()+
					  "\t\"ASCII_"+currentModuleCode+"\""+							  
					  "\n");
	}
	void generateFontSpriteFile(String fontCharactersStringFile,String inputImageFile, String outputSpriteFileName, String imageFileNameForSpriteFile)
			throws Exception {
		ImageIcon icon = new ImageIcon(inputImageFile);
		
		fWidth = icon.getIconWidth();
		fHeight = icon.getIconHeight();
		
		
		System.out.println("Processing "+inputImageFile);
		System.out.println("Width:"+fWidth);		
		System.out.println("Height:"+fHeight);		
		fPixelsBuffer = getPixels(icon.getImage(), 0, 0, fWidth, fHeight);
		try {
			fCurrentModule = 0;
			fModuleCoordinates = new ModuleCoordinates[MAX_MODULES];
			try
			{
				fFontCharactersString = readFontCharacterStringFromFile(fontCharactersStringFile);
				try
				{
					findCharacters();
					checkFoundModuleCoordinates();
					writeSpriteFile(inputImageFile,outputSpriteFileName,imageFileNameForSpriteFile);					
					System.out.println("File " + outputSpriteFileName + " created successfully.");
				}
				finally
				{
					fFontCharactersString = null;					
				}
			} 
			finally
			{
				fPixelsBuffer = null;				
			}
		} finally {
			fPixelsBuffer = null;
			fPalette = null;
		}
	}

	public int[] getPixels(Image img, int x, int y, int w, int h) {
		int[] pixels = new int[w * h];
		PixelGrabber pg = new PixelGrabber(img, x, y, w, h, pixels, 0, w);
		try {
			pg.grabPixels();
		} catch (InterruptedException e) {
			System.err.println("interrupted waiting for pixels!");
			return null;
		}
		if ((pg.getStatus() & ImageObserver.ABORT) != 0) {
			System.err.println("image fetch aborted or errored");
			return null;
		}
		return pixels;
	}
	private static void writeChar(FileOutputStream output,int charToWrite,boolean unicode) throws IOException
	{
		if (unicode) output.write(0x00);
		output.write(charToWrite);
	}
	private static int fOptionalVerticalOffset=0;
	private static int fOptionalHorizontalOffset=0;
	private static int fHorizontalThreshold = 1;
	private static final int  ARG_INDEX_TEXT_FILE_PATH = 0;	
	private static final int  ARG_INDEX_IMAGE_FILE_PATH = ARG_INDEX_TEXT_FILE_PATH+1;	
	private static final int  ARG_INDEX_SPRITE_FILE_PATH = ARG_INDEX_IMAGE_FILE_PATH+1;	
	private static final int  ARG_INDEX_IMAGE_FILE_PATH_FOR_SPRITE = ARG_INDEX_SPRITE_FILE_PATH+1;	
	private static final int  ARG_INDEX_OPTIONAL_VERTICAL_OFFSET = ARG_INDEX_IMAGE_FILE_PATH_FOR_SPRITE + 1; 
	private static final int  ARG_INDEX_OPTIONAL_HORIZONTAL_OFFSET = ARG_INDEX_OPTIONAL_VERTICAL_OFFSET + 1;	
	private static final int  ARG_INDEX_OPTIONAL_HORIZONTAL_THRESHOLD = ARG_INDEX_OPTIONAL_HORIZONTAL_OFFSET + 1;	
	public static void main(String[] args) throws Exception {
		System.out.println("FontGenerator ver. 0.0.1");					
		if (args.length>=3)
		{
			System.out.println("Warning, horizontal offset system is disabled in this build.All modules are written with x offset 0!");			
			
			String textFilePath = args[ARG_INDEX_TEXT_FILE_PATH];
			String imageFilePath = args[ARG_INDEX_IMAGE_FILE_PATH];			
			String spriteFilePath = args[ARG_INDEX_SPRITE_FILE_PATH];			
			System.out.println("Text file path:"+textFilePath);
			System.out.println("Image file path:"+imageFilePath);
			System.out.println("Sprite file path:"+spriteFilePath);
			String imageFileNameForSpriteFile=imageFilePath;


			//Optionally the file name which is written in the sprite file for the image can be overriden
			if (args.length > ARG_INDEX_IMAGE_FILE_PATH_FOR_SPRITE)
				imageFileNameForSpriteFile = args[ARG_INDEX_IMAGE_FILE_PATH_FOR_SPRITE];
 
			System.out.println("Image file name to be written in the sprite file:"+imageFileNameForSpriteFile);			
 
			if (args.length>ARG_INDEX_OPTIONAL_VERTICAL_OFFSET)
			{
				System.out.println("Optional parameter vertical offset:"+args[ARG_INDEX_OPTIONAL_VERTICAL_OFFSET]);
				fOptionalVerticalOffset = Integer.parseInt(args[ARG_INDEX_OPTIONAL_VERTICAL_OFFSET]);
			}
			if (args.length>ARG_INDEX_OPTIONAL_HORIZONTAL_OFFSET)
			{			
				System.out.println("Optional parameter horizontal spacing offset:"+args[ARG_INDEX_OPTIONAL_HORIZONTAL_OFFSET]);
				fOptionalHorizontalOffset = Integer.parseInt(args[ARG_INDEX_OPTIONAL_HORIZONTAL_OFFSET]);
			}
			if (args.length>ARG_INDEX_OPTIONAL_HORIZONTAL_THRESHOLD)
			{			
				System.out.println("Optional parameter horizontal threshold:"+args[ARG_INDEX_OPTIONAL_HORIZONTAL_THRESHOLD]);
				fHorizontalThreshold = Integer.parseInt(args[ARG_INDEX_OPTIONAL_HORIZONTAL_THRESHOLD]);
			}
			
			FontGenerator convert = new FontGenerator();
			convert.generateFontSpriteFile(textFilePath,imageFilePath,spriteFilePath,imageFileNameForSpriteFile);
		} else
		{
			if (args.length==1 && 
			   (args[0].toLowerCase().compareTo("generate_maps") == 0))
			{
				writeFontTextMaps("c:\\font_desc_unicode_5spaces.txt",true);
				writeFontTextMaps("c:\\font_desc_ascii_5spaces.txt",false);								
			}
			else
			{
				System.out.println("Usage: java FontGenerator <txt_file> <image_file> <output_sprite_file> <optional_image_file_name_for_sprite> <optional_vertical_offset> <optional_horizontal_spacing_offset> <optional_horizontal_threshold>");
			}
			
		}

	}
	private static void writeFontTextMaps(String fileName,boolean unicode) throws FileNotFoundException,
			IOException {
		
		FileOutputStream output = new FileOutputStream(fileName);
		//Unicode signature
		if (unicode)
		{
			output.write(0xFE);
			output.write(0xFF);
		}
		writeChar(output,124,unicode);
		
		writeSpaces(unicode, output);			
				
		
		for (int i=33;i<255;i++)
		{
			if (FontMap.MAP[i]!=0 )
			{
				writeChar(output,i,unicode);
				
				writeSpaces(unicode, output);			
				writeChar(output,124,unicode);				
				writeSpaces(unicode, output);
			}
		}
//		String text = new String(fontTextMap,0,m_pointer,"ISO-8859-1");
	}

	private static void writeSpaces(boolean unicode, FileOutputStream output)
			throws IOException {
		writeChar(output,32,unicode);
		writeChar(output,32,unicode);
		writeChar(output,32,unicode);
		writeChar(output,32,unicode);		
		writeChar(output,32,unicode);		
		
	}
	 
}
