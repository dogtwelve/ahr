import java.io.*;
import java.util.*;
import java.lang.*;


// ------------------------------------------------------------------------------------------------
// parse string XML file
// ------------------------------------------------------------------------------------------------
public class SoundConvertor
{
	public final static String STRINGS_CHARSET_OUT 		= "Windows-1252";
	public final static String FOURCC_RIFF = "RIFF";
	public final static String FOURCC_WAVE = "WAVE";
	public final static String FOURCC_DATA = "data";
	
	public final static String INPUT_EXT = ".wav";
	public final static String OUTPUT_EXT = ".znd";	
	
	/// Find wav data tag and return its size
	public static int FindWavDataTag( FileInputStream wav_file ) throws Exception
	{
	    byte[]     tag = new byte[4];
	    byte[]     size = new byte[4];
		int 		riff_size = 0;
	    boolean    fFmt = false, fData = false;
		int     tmp_pos = 0;
		int     rv;
		String  str = "";

		//RIFF TAG
		rv = wav_file.read( tag );
		if (rv<0)
		{
			System.out.println(" ERROR: Read file ");
			return 0;
		}
		str = new String( tag );
	    if( str.compareTo( FOURCC_RIFF ) != 0 )		
		{
			System.out.println(" ERROR: First tag error ");
	        return 0;
		}
	
		//RIFF SIZE
		rv = wav_file.read( size );
		if (rv<0)
		{
			System.out.println(" ERROR: Read size ");		
			return 0;
		}
		riff_size = ((size[3]&0xFF)<<24) + ((size[2]&0xFF)<<16) + ((size[1]&0xFF)<<8) + size[0];
//		System.out.println("Riff size: " + riff_size);		
		
		
		//WAV TAG
		rv = wav_file.read( tag );
		if (rv<0)
		{
			System.out.println(" ERROR: Read file wave tag");
			return 0;
		}
		str = new String( tag );
	    if( str.compareTo( FOURCC_WAVE ) != 0 )				
		{
			System.out.println(" ERROR: Second tag error ");
	        return 0;
		}

		//look for data & format
		while(riff_size >= 0)
		{
			byte[]  chunkSizeData = new byte[4];
			int 	chunkSize;
			
			rv = wav_file.read( tag );
			if (rv<0)
			{
				System.out.println(" ERROR: Read " + riff_size);
				return 0;
			}
			
			rv = wav_file.read( chunkSizeData );
			if( rv<0 )
			{
				System.out.println(" ERROR: Read " + riff_size);
				return 0;
			}
			chunkSize = ((chunkSizeData[3]&0xFF)<<24) + ((chunkSizeData[2]&0xFF)<<16) + ((chunkSizeData[1]&0xFF)<<8) + chunkSizeData[0];			
			
			str = new String( tag );
//			System.out.println("chunk: " + str);
			if( str.compareTo( FOURCC_DATA ) == 0 )							
			{
//				System.out.println("data chunk size: " + chunkSize );
				return chunkSize;
			}
		
			riff_size -= chunkSize;
			
			while( chunkSize-- > 0 ) 
				wav_file.read();
		}
		
		return 0; // unless we got both the format and data it's not going to work
	}
	
	//// Remove WAV file header
	public static void Convert( String input_file, String output_file )
	{
		
		if( input_file.toLowerCase().endsWith( INPUT_EXT ) )
		{
			input_file = input_file.substring( 0, input_file.length()-INPUT_EXT.length() ); 
		}
		
		input_file.replaceAll( "/", "\\" );
		int start = input_file.lastIndexOf( "\\" );		
		if( start < 0 )
			start = 0;
		
		output_file += input_file.substring( start ) + OUTPUT_EXT;
		input_file += INPUT_EXT;

		System.out.println( "Convert sound " + input_file + " to " + output_file );
		
		try
		{
			FileInputStream input = new FileInputStream( input_file );		
			FileOutputStream output = new FileOutputStream( output_file );		
			
			int size = FindWavDataTag( input );
			
			if( size != 0 )
			{
				byte[] buffer = new byte[size];
				
				input.read( buffer );
				output.write( buffer ); 
			}
			
			input.close();
			output.close();
		}
		catch(Exception e) { System.out.println("SoundConvertor.Convert() error: "+e.toString()); e.printStackTrace(); }
	}
	
	public static void main(String args[])
	{
	
		if(args.length < 2 )
		{
			System.out.println("Usage: java SoundConvertor [WAV  file] [output dir (no final \\)] ");
			System.exit(0);			
		}
		
		Convert( args[0], args[1] );
	}
	

 }
