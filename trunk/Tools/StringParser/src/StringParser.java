import java.io.*;
import java.util.*;
import java.text.*;


// ------------------------------------------------------------------------------------------------
// parse string XML file
// ------------------------------------------------------------------------------------------------
public class StringParser
{
	public final static String STRINGS_CHARSET_OUT 		= "Windows-1252";
	public final static String STRINGS_CHARSET_OUT_CHS 	= "Unicode";	
	
	
	public static final String CHS_LANG[] = {"CHT", "CHS", "JP", "AR"}; 
	
	// command-line parameters
	public final static int ARGUMENT_XML            = 0;	
	public final static int ARGUMENT_OUT_DIR        = 1;

	
	// columns in Excel table
	public final static int COL_STR_ID        = 0;
	public final static int LANG_LINE_ID    = 0;
	
	/// ID string
	public final static String ID_STRINT  		= "::ID::";
	
	/// Export file ext.
	public final static String EXPORT_EXT		= ".hpp";
	public final static String TEXT_EXT			= ".txt";
	
	/// Special symbol
	public final static byte FAKE_SYMBOL = (byte)0x01;
	public final static byte FAKE_SYMBOL_2 = (byte)0xFF;
	
	public static void main(String args[])
	{
	
		if(args.length < 2 )
		{
			System.out.println("Usage: java StringParser [XML  file] [output dir (no final \\)] ");
			System.exit(0);			
		}
		
		ParseStrings( args[ARGUMENT_XML], args[ARGUMENT_OUT_DIR], "intl_" );
	}
	
	// ------------------------------------------------------------------------------------------------
	/// @brief Parse 
	// ------------------------------------------------------------------------------------------------	
	public static void ParseStrings( String XML_filename, String OutputDirName, String ExportFilePrefix )
	{
		
		try
		{
			System.out.println("Parse " + XML_filename + " file starting ");
			
			if( OutputDirName == null )
				OutputDirName = "";
			if( !OutputDirName.endsWith( "\\" ) );
				OutputDirName = OutputDirName + "\\"; 
			
			/// Load all sheets names	from XML file
			String[] all_packs_names = FileProcessor.GetXMLSheetsNames( XML_filename );
			
			for(int pack_id = 0; pack_id < all_packs_names.length; pack_id++)
			{					
				/// Load XML file to strings
				String[][] strings = FileProcessor.GetXMLSheet( XML_filename, all_packs_names[pack_id], null);
				
				FileProcessor.FormatSheetStrings(strings, FileProcessor.FORMAT_NO_NULL  | FileProcessor.FORMAT_TRIM | FileProcessor.FORMAT_NO_EXCEL_CHARS | FileProcessor.FORMAT_DELETE_QUOTES);
				
				
				/// Check is ID col is needed
				int start_col = 0;				
				boolean exportID = false;
				if( strings[LANG_LINE_ID][COL_STR_ID].compareTo( ID_STRINT ) == 0)
				{
					exportID = true;
					start_col = COL_STR_ID + 1;
				}

					
				/// Export text, from current sheet, for all languages 				
				int max_col_num = strings[LANG_LINE_ID].length;
				int languages_num = max_col_num - start_col;
				int max_texts_num = strings.length;							
				
				
				
				/// Go through all languages
				for( int lang=start_col; lang<max_col_num; lang++ )
				{
					String current_lang = strings[LANG_LINE_ID][lang];
					String current_output_filename = all_packs_names[pack_id] + current_lang;

					current_output_filename = OutputDirName + current_output_filename;
					
										
					/// Export all texts
					int out_text_lines = 0;					
					String str[] = new String[max_texts_num - 1];
					
					for( int txt=LANG_LINE_ID+1; txt<max_texts_num; txt++ )
					{
						if( start_col == 0 )
						{
							/// Don't need ID string 
							if( strings[txt][lang].length() > 0 )
								str[out_text_lines++] =  "" + strings[txt][lang];
						}
						else
						{
							/// Need ID string 
							if( strings[txt][COL_STR_ID].length() > 0 )	
							{
								/// Fix bug with empty data
								//if( strings[txt][lang].length() <= 0 )
								//{								
								//	strings[txt][lang] = strings[txt][COL_STR_ID]; /// Set to language ID
								//}
								
								str[out_text_lines++] = strings[txt][lang];
							}						
							else 						
							{
								/// Check for incorrect or missing ID's
								for( int j=start_col; j<max_col_num; j++ )
									if( strings[txt][j].length() > 0 )																	
										throw new Exception( "ERROR: NO ID sheet=" + all_packs_names[pack_id] + " lang=" + strings[LANG_LINE_ID][j] + " line=" + (txt+1) ); 
							}
						}
						
					}

					/// Save exported text to file
					if( IsChsLang( strings[LANG_LINE_ID][lang] ) )	
					{
						if(strings[LANG_LINE_ID][lang].compareTo("AR") == 0)
							SaveStringsData( current_output_filename, false, str, out_text_lines, STRINGS_CHARSET_OUT_CHS, exportID, true );
						else
							SaveStringsData( current_output_filename, false, str, out_text_lines, STRINGS_CHARSET_OUT_CHS, exportID, false );
					}
					else
						//export latin as unicode
						SaveStringsData( current_output_filename, false, str, out_text_lines, STRINGS_CHARSET_OUT_CHS, exportID, false );
					
					//ExportStringsAsText(OutputDirName + all_packs_names[pack_id] + "_strs_" + strings[LANG_LINE_ID][lang] + ".txt", strings, lang);
				}
				
				/// Export ID's if needed
				if( exportID && ExportFilePrefix!=null)
				{
					ExportFilePrefix = OutputDirName + ExportFilePrefix;
						
					ExportID( ExportFilePrefix + all_packs_names[pack_id], strings, COL_STR_ID );
				}
			}
		}
		catch(Exception e) { System.out.println("StringParser.ParseStrings() error: "+e.toString()); e.printStackTrace(); }
	}
	
	// ------------------------------------------------------------------------------------------------
	/// @brief Saves an array of strings into a text file (each string will be on a new line)
	/// @param append tells whether to append to the file or reset (delete) the file before starting to write
	/// @note For convenience the <lines> parameter is declared as Object[] (not String[]); each line is cast to (String)
	// ------------------------------------------------------------------------------------------------
	public static void SaveStringsData( String filename, boolean append, Object[] lines, int size, String encoding, boolean exportID, boolean isArabic)
	{	
		if( lines == null || size < 1 ) return;

		try
		{			
			FileOutputStream file = new FileOutputStream(filename + TEXT_EXT, append);
			
			int offset = 0;
			if( encoding.compareTo( STRINGS_CHARSET_OUT_CHS ) == 0 )
			{
				offset = 2;
			}
			
			/// Export ID's and if needed
			if( exportID )
			{				
				/// Write stirngs size
				FileProcessor.WriteToFile_U32( file, size );
							
				/// Write strings offsets
				int cur_offset = 4 + size * 4; /// 4 byte for C pointer
				for(int i = 0; i < size; i++)			
				{
					FileProcessor.WriteToFile_U32( file, cur_offset );			
					cur_offset += GetBytes( (String)lines[i] + "\0", encoding).length - offset;
				}			
			}
			
			
			/// Write strings			
			for(int i = 0; i < size; i++)
			{										
				byte tmp[] = GetBytes((String)lines[i] + "\0", encoding);

				if(isArabic)
				{
					Bidi b = new Bidi((String)lines[i], Bidi.DIRECTION_DEFAULT_RIGHT_TO_LEFT );
					
					if (b.isRightToLeft())	//all arabic text
					{
						int tmplen = tmp.length;
						byte arStr[] = new byte[tmplen];
						for(int k = 0; k < tmplen - 2; k+=2)
						{
							arStr[k    ] = tmp[tmplen - k - 2];
							arStr[k + 1] = tmp[tmplen - k - 1];
						}
						arStr[tmplen-2] = 0;
						arStr[tmplen-1] = 0;
						tmp = arStr;
					}
					else if (b.isMixed())
					{
						StringBuffer result = new StringBuffer("");
						
						StringBuffer reversedString = new StringBuffer((String)lines[i]);
						reversedString.reverse();
						
						//StringTokenizer stok = new StringTokenizer(reversedString.toString(), " ");
						int startIndex = 0;
						int endIndex = reversedString.toString().indexOf(' ', startIndex);
						
						while (endIndex != -1)
						{
							String substr = reversedString.substring(startIndex, endIndex);
							Bidi bsub = new Bidi (substr, Bidi.DIRECTION_DEFAULT_RIGHT_TO_LEFT);
							
							if( ! (bsub.isRightToLeft()) && ! (bsub.isMixed()) )
							{
								StringBuffer strbuf = new StringBuffer(substr);
								strbuf.reverse();
								substr = strbuf.toString();
							}
							
							result.append(substr + " ");
							
							startIndex = endIndex + 1;
							endIndex = reversedString.toString().indexOf(' ', startIndex);
						}
						
						String substr = reversedString.substring(startIndex);
						Bidi bsub = new Bidi (substr, Bidi.DIRECTION_DEFAULT_RIGHT_TO_LEFT);
							
						if( ! (bsub.isRightToLeft()) && ! (bsub.isMixed()) )
						{
							StringBuffer strbuf = new StringBuffer(substr);
							strbuf.reverse();
							substr = strbuf.toString();
						}
							
						result.append(substr );
						
						tmp = GetBytes(result.toString() + "\0", encoding);
					}					
				}
				
				file.write( tmp, offset, tmp.length-offset );
			}
		
			file.close();
		}
		catch(Exception e) { System.out.println("StringParser.SaveStringsData() error: "+e.toString()); e.printStackTrace(); }
	}

	public static void ExportID( String filename, String[][] lines, int col )
	{
		if( lines == null ) return;
		
		try
		{
			FileOutputStream file 		= new FileOutputStream( filename+EXPORT_EXT, false );
			FileOutputStream file_arm 	= new FileOutputStream( filename+"_arm"+EXPORT_EXT, false );
			
			
			/// Write all lines of exported file		
			file.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "// WARNING: This file was generated by 'StringParser' tool!\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "BEGIN_STRINGS\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );

			file_arm.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "// WARNING: This file was generated by 'StringParser' tool!\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "BEGIN_STRINGS\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			
			String str = "";
			String str_arm = "";
			int j = 0;
			for(int i = LANG_LINE_ID+1; i < lines.length; i++)
			{
				if( lines[i][col].length() > 0 && !lines[i][col].startsWith("//") )
				{
					str = lines[i][col] /*+ " = " + (j) */+ "," + "\n";				
					str_arm = "STRING_( " + (j++) + ",\t" + lines[i][col] + ",\t\"" + lines[i][col+1].replace('\n', '#') + "\" );\n";					
					
					file.write( str.getBytes(STRINGS_CHARSET_OUT) );
					file_arm.write( str_arm.getBytes(STRINGS_CHARSET_OUT) );
				}
			}

			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "END_STRINGS".getBytes(STRINGS_CHARSET_OUT) );
			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );			
			file_arm.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );			
			file_arm.write( "\n".getBytes(STRINGS_CHARSET_OUT) );			
			file_arm.close();
			
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "END_STRINGS".getBytes(STRINGS_CHARSET_OUT) );
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );			
			file.write( "///////////////////////////////////////////////////////////\n".getBytes(STRINGS_CHARSET_OUT) );			
			file.write( "\n".getBytes(STRINGS_CHARSET_OUT) );			
			file.close();
		}
		catch(Exception e) { System.out.println("StringParser.ExportID() error: "+e.toString()); e.printStackTrace(); }		
	}
	
	public static void ExportStringsAsText( String filename, String[][] lines, int col )
	{
		if( lines == null ) return;
		
		try
		{
			FileOutputStream file 		= new FileOutputStream( filename, false );
			
			
			/// Write all lines of exported file
			file.write(0xFF);
			file.write(0xFE);
			
			String str = new String();
			int j = 0;
			for(int i = LANG_LINE_ID+1; i < lines.length; i++)
			{
				if( lines[i][col].length() > 0 && !lines[i][col].startsWith("//") )
				{
					str = lines[i][col] + "\r\n";
					
					//file.write( str.getBytes(STRINGS_CHARSET_OUT_CHS) );
					for (int k = 0; k < str.length(); k++)
					{
						int ch = str.charAt(k);
						file.write(ch & 0xFF);
						file.write((ch >> 8) & 0xFF);
					}
				}
			}
			
			file.close();
		}
		catch(Exception e) { System.out.println("StringParser.ExportID() error: "+e.toString()); e.printStackTrace(); }		
	}

	
	public static boolean IsChsLang( String lang )
	{
		for( int i=0; i<CHS_LANG.length; i++ )
		{
			if( lang.compareTo( CHS_LANG[i] ) == 0 )
			{
				return true;
			}
		}
		
		return false;
	}
	
	public static byte[] GetBytes( String str, String encoding) throws Exception
	{
		byte ret[] = str.getBytes( encoding );
		
		if( encoding.compareTo( STRINGS_CHARSET_OUT_CHS ) == 0 )
		{
//			for( int i=1; i < ret.length-2; i += 2 )
//				if( ret[i] == 0)
//					ret[i] = FAKE_SYMBOL;
//
//			for( int i=0; i < ret.length-2; i += 2 )
//				if( ret[i] == 0)
//					ret[i] = FAKE_SYMBOL_2;
			
			byte tmp;
			for (int i = 0; i < ret.length; i += 2)
			{
				tmp = ret[i];
				ret[i] = ret[i+1];
				ret[i+1] = tmp;
			}
		}
				
		return ret;
	}
	
 }
