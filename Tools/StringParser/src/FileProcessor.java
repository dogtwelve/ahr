import java.io.*;
import java.util.*;

import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import org.xml.sax.InputSource;



// ------------------------------------------------------------------------------------------------
// common file processing functions
// ------------------------------------------------------------------------------------------------
//
// public static String[] GetFileAsLinesArray (String filename, boolean trimmed)
// public static void WriteLinesToFile (String filename, boolean append, Object[] lines)
// public static void WriteToFile_U32 (FileOutputStream file, int value) throws Exception
// public static void WriteToFile_U16 (FileOutputStream file, int value) throws Exception
// public static void AppendFileToFile (String filename_part, String filename_base)
//
// public static String[] GetXMLSheetsNames (String filename)
// public static String[][] GetXMLSheet (String filename, String sheetname, Hashtable ifdef_defines)
//
// public static Hashtable GetJavaConstants (String filename, int which_constants, Hashtable existing_constants)
// public static Hashtable GetCDefines (String filename, int which_constants, Hashtable existing_constants)
//
// public static void MakeConstantsFile (Hashtable constants, String filename, boolean java_interface)
//
// public static boolean IsConstantDefined (Hashtable h, String const_name)
// public static int     GetConstant_Int (Hashtable h, String const_name) throws Exception
// public static String  GetConstant_String (Hashtable h, String const_name) throws Exception
// public static boolean GetConstant_Define (Hashtable h, String const_name) throws Exception
//
// static void    Ifdef_InitParsing ()
// static boolean Ifdef_LineIsActive (Hashtable ifdef_defines, String current_line)
//
// static void FormatSheetStrings (String[][] strings, int format)
// static String RemoveComment (String s)
// static String correctExcelString (String s)
// static String DeleteStartEndQuotes (String s)
//
// ------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------
public class FileProcessor
// ------------------------------------------------------------------------------------------------
{
	
	public final static String STRINGS_CHARSET = "UTF-8";//"Unicode";//"Windows-1252";//"UTF-8"; //"ISO-8859-1"; // charset for all created strings


	
	// FORMAT_xxx are used in <format> parameter of FormatSheetStrings()
	public final static int FORMAT_NO_NULL        = 1 << 0; // null strings are replaced by ""
	public final static int FORMAT_NO_COMMENTS    = 1 << 1; // strings starting with "//" are replaced by ""
	public final static int FORMAT_NO_EXCEL_CHARS = 1 << 2; // strings are corrected with correctExcelString()
	public final static int FORMAT_TRIM           = 1 << 3; // whitespaces removed from strings' beginnings and ends
	public final static int FORMAT_DELETE_QUOTES  = 1 << 4; // some strings was bounded with quotes so we got to remove them
	
	// CONSTANTS_xxx are used in <which_constants> parameter of GetJavaConstants() / GetCDefines()
	public final static int CONSTANTS_INT         = 1 << 0;
	public final static int CONSTANTS_BYTE        = 1 << 1;
	public final static int CONSTANTS_STRING      = 1 << 2;
	public final static int CONSTANTS_DEFINE      = 1 << 3;


	
	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Returns a file as an array of strings; each string is 1 line (no \n at its end)
	/// @param trimmed tells whether the strings should be trimmed (removed spaces and tabs in beginning and end of each string)
	// ------------------------------------------------------------------------------------------------
	public static String[] GetFileAsLinesArray (String filename, boolean trimmed)
	{
		if (filename.toLowerCase().compareTo("null") == 0) return null;
		
		try
		{
			FileInputStream file = new FileInputStream(filename);
			
			byte strByte[] = new byte[file.available()];
			file.read(strByte);
			
			file.close();
			
			int lines_counter = 1;

			// count the lines
			for(int i = 0; i < strByte.length; i++)
				if (strByte[i] == '\n' || strByte[i] == '\r')
				{
					lines_counter++;
					if (i < strByte.length - 1)
						if (strByte[i+1] == '\n' || strByte[i+1] == '\r')
							i++;
					
					if (i >= strByte.length - 1)
						lines_counter--;
				}
			
			String[] strings = new String[lines_counter];

			int line_start = 0;
			int current_string = 0;
			for(int i = 0; i < strByte.length; i++)
			{
				if (strByte[i] == '\n' || strByte[i] == '\r')
				{
					strings[current_string] = new String(strByte, line_start, i - line_start);
					if (trimmed)
						strings[current_string] = strings[current_string].trim();
						
					current_string++;
					
					line_start = i + 1;
					
					if (i < strByte.length - 1)
						if (strByte[i+1] == '\n')
						{
							i++;
							line_start++;
						}
				}
			}
			
			return strings;
		}
		catch(Exception e) { System.out.println("FileProcessor.GetFileAsLinesArray() error: "+e.toString()); e.printStackTrace(); }
		
		return null;
	}


	// ------------------------------------------------------------------------------------------------
	/// @brief Saves an array of strings into a text file (each string will be on a new line)
	/// @param append tells whether to append to the file or reset (delete) the file before starting to write
	/// @note For convenience the <lines> parameter is declared as Object[] (not String[]); each line is cast to (String)
	// ------------------------------------------------------------------------------------------------
	public static void WriteLinesToFile (String filename, boolean append, Object[] lines)
	{
		if (lines == null) return;

		try
		{
			FileOutputStream file = new FileOutputStream(filename, append);
			
			for(int i = 0; i < lines.length; i++)
			{
				file.write(((String)lines[i]).getBytes(STRINGS_CHARSET));
				file.write('\n');
			}
		
			file.close();
		}
		catch(Exception e) { System.out.println("FileProcessor.WriteLinesToFile() error: "+e.toString()); e.printStackTrace(); }
	}

	
// ------------------------------------------------------------------------------------------------
	/// @brief Appends file <filename_part> to <filename_base> (binary)
	// ------------------------------------------------------------------------------------------------
	public static void AppendFileToFile (String filename_part, String filename_base)
	{
		try
		{
			FileInputStream file_part = new FileInputStream(filename_part);
			
			byte file_contents[] = new byte[file_part.available()];
			file_part.read(file_contents);

			file_part.close();
			
			FileOutputStream file_base = new FileOutputStream(filename_base, true);
			file_base.write(file_contents);
			file_base.close();
		}
		catch(Exception e) { System.out.println("FileProcessor.AppendFileToFile() error: "+e.toString()); e.printStackTrace(); }
	}
	
	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Reads a java file and returns a Hashtable with all constants of wanted types in <which_constants>
	/// @param which_constants can be CONSTANTS_INT | CONSTANTS_BYTE | CONSTANTS_STRING
	/// @param existing_constants gives a table of already defined constants; currently loaded constants will be added to these; null == no existing constants
	/// @note Identifiable constants are declared as "final static TYPE CONSTANT_NAME = CONSTANT_VALUE;"
	/// @note Returned hashtable has the following format:  key: (String)CONSTANT NAME  ;  value: (Integer)/(Byte)/(String)CONSTANT VALUE
	// ------------------------------------------------------------------------------------------------
	public static OrderedHashtable GetJavaConstants (String filename, int which_constants, OrderedHashtable existing_constants)
	{
		try
		{
			String[] file_lines = GetFileAsLinesArray(filename, true);
			
			if (existing_constants == null) existing_constants = new OrderedHashtable();
			
			if (filename.toLowerCase().compareTo("null") == 0 || file_lines == null) return existing_constants;
			
			Ifdef_InitParsing();
			
			String constant_regular_prefix = "[static|final";
			
			if ((which_constants & CONSTANTS_INT) != 0) constant_regular_prefix += "|int";
			if ((which_constants & CONSTANTS_BYTE) != 0) constant_regular_prefix += "|byte";
			if ((which_constants & CONSTANTS_STRING) != 0) constant_regular_prefix += "|String";			
			
			constant_regular_prefix = constant_regular_prefix + "]+";
			
			for(int i = 0; i < file_lines.length; i++)
			{
				if (!Ifdef_LineIsActive(existing_constants, file_lines[i])) continue;
				
				String[] line_parts = file_lines[i].split("[\\s;]+");
				
				int constant_name_pos = 0;
			
				while (line_parts[constant_name_pos].matches(constant_regular_prefix))
					constant_name_pos++;
				
				if (constant_name_pos < 3) continue;
				
				String const_type = line_parts[constant_name_pos - 1];

				if (const_type.compareTo("int") == 0)
					existing_constants.put(line_parts[constant_name_pos], new Integer(line_parts[line_parts.length - 1]));
				else if (const_type.compareTo("byte") == 0)
					existing_constants.put(line_parts[constant_name_pos], new Byte(Byte.parseByte(line_parts[line_parts.length - 1])));
				else if (const_type.compareTo("String") == 0)
					existing_constants.put(line_parts[constant_name_pos], line_parts[line_parts.length - 1]);
				
				//System.out.println("Constant: '"+line_parts[constant_name_pos]+"'  Type: '"+const_type+"'   Value: '"+line_parts[line_parts.length - 1]+"'");
			}
			
			return existing_constants;			
		}
		catch(Exception e) { System.out.println("FileProcessor.GetJavaConstants_Int() error: "+e.toString()); e.printStackTrace(); }
		
		return null;
	}


// ------------------------------------------------------------------------------------------------
	/// @brief Reads a C definitions file (.h) and returns a Hashtable with all constants of wanted types in <which_constants>
	/// @param which_constants can be CONSTANTS_INT | CONSTANTS_STRING | CONSTANTS_DEFINE
	/// @param existing_constants gives a table of already defined constants; currently loaded constants will be added to these; null == no existing constants
	/// @note Identifiable constants are declared as "#define CONSTANT_NAME CONSTANT_VALUE"
	/// @note Returned hashtable has the following format:  key: (String)CONSTANT NAME  ;  value: (Integer)/(String)CONSTANT VALUE
	// ------------------------------------------------------------------------------------------------
	public static OrderedHashtable GetCDefines (String filename, int which_constants, OrderedHashtable existing_constants)
	{
		try
		{
			String[] file_lines = GetFileAsLinesArray(filename, true);

			if (existing_constants == null) existing_constants = new OrderedHashtable();
			
			if (filename.toLowerCase().compareTo("null") == 0 || file_lines == null) return existing_constants;
			
			Ifdef_InitParsing();
			
			for(int i = 0; i < file_lines.length; i++)
			{
				if (!Ifdef_LineIsActive(existing_constants, file_lines[i])) continue;
				
				String line = file_lines[i].replaceFirst("\\#define[\\s]+", ""); // the line without "#define "
				
				if (line.length() == file_lines[i].length()) continue;
				
				int const_name_len = line.length() - line.replaceFirst("[\\w]+", "").length();
				
				String const_value = line.substring(const_name_len).replaceFirst("[\\s]+", "");
				
				if ((which_constants & CONSTANTS_DEFINE) != 0 && const_name_len == line.length())
					existing_constants.put(line.substring(0, const_name_len), new Boolean(true));
				else
				{
					if ((which_constants & CONSTANTS_INT) != 0 && const_value.matches("[0-9]+[0-9x]*"))
					{
						if (const_value.startsWith("0x"))
							existing_constants.put(line.substring(0, const_name_len), new Integer(Integer.parseInt(const_value.substring(2), 16)));
						else
							existing_constants.put(line.substring(0, const_name_len), new Integer(const_value));
					}
					else if ((which_constants & CONSTANTS_STRING) != 0 && const_value.startsWith("\""))
						existing_constants.put(line.substring(0, const_name_len), const_value);
				}
				//System.out.println("Constant: '"+line.substring(0, const_name_len)+"'    Value: '"+const_value+"'");
			}
			
			return existing_constants;			
		}
		catch(Exception e) { System.out.println("FileProcessor.GetCDefines() error: "+e.toString()); e.printStackTrace(); }
		
		return null;
	}

	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Creates a text file (either a java interface or a list of C defines) that contains the <constants>
	/// @param filename should be without extension (it is added automatically depending on whether creating a java interface (.java) or C defines (.h))
	/// @param java_interface tells whether we want a java interface file or a file with C defines
	/// @param interface_name tells the name of created interface; if <java_interface> is false then this param doesn't matter
	// ------------------------------------------------------------------------------------------------
	public static void MakeConstantsFile (OrderedHashtable constants, String filename, boolean java_interface) throws Exception
	{
		ArrayList lines = new ArrayList();
		
		lines.add(new String("// ------------ AUTOGENERATED FILE --------------"));
		lines.add(new String(" "));
		
		if (java_interface)
		{
			String name = filename.substring(filename.lastIndexOf("\\") + 1, filename.length());
			lines.add(new String("interface "+name));
			lines.add(new String("{"));
			
			Enumeration names = constants.enumerateKeys();
		
			while(names.hasMoreElements())
			{
				Object const_name = names.nextElement();

				if(((String)const_name).startsWith("//"))
				{
					lines.add(new String(GetConstant_String(constants, (String)const_name)));
					continue;
				}
				
				String line = "\tstatic final ";
				
				if (constants.get((String)const_name).getClass().getName() == "java.lang.String")
				{
					line += "String " + (String)const_name;
					line += AddNString(" ", 50 - line.length());
					line += " = " + GetConstant_String(constants, (String)const_name);
				}
				else
				{
					line += "int " + (String)const_name;
					line += AddNString(" ", 50 - line.length());
					line += " = " + GetConstant_Int(constants, (String)const_name);
				}
				
				line += ";";

				lines.add(line);
			}
			
			lines.add(new String("}"));
			
			WriteLinesToFile(filename+".java", false, lines.toArray());
		}
		else
		{
			Enumeration names = constants.enumerateKeys();
			
			while(names.hasMoreElements())
			{
				Object const_name = names.nextElement();
				
				if(((String)const_name).startsWith("//"))
				{
					lines.add(new String(GetConstant_String(constants, (String)const_name)));
					continue;
				}
				
				String line = "#define " + (String)const_name + "   ";
				
				Object value = constants.get((String)const_name);
				
				//System.out.println("Const: "+(String)const_names[i]+" Type: "+value.getClass().getName());
				
				line += AddNString(" ", 50 - line.length());
				if (value.getClass().getName() == "java.lang.String")
					line += GetConstant_String(constants, (String)const_name);
				else if (value.getClass().getName() == "java.lang.Boolean")
				{
					if (!GetConstant_Define(constants, (String)const_name)) line = "";
				}
				else
					line += "" + GetConstant_Int(constants, (String)const_name);
					
				if (line.compareTo("") != 0)
					lines.add(line);
			}
			
			WriteLinesToFile(filename+".h", false, lines.toArray());
		}
	}
	
	// ------------------------------------------------------------------------------------------------
	/// @brief Creates a list text file which contains the elemets in the hashtable
	/// @param constants list of constants to add
	/// @param filename should be without extension - it is added automatically (.list)
	// ------------------------------------------------------------------------------------------------
	public static void MakeListFile (OrderedHashtable constants, String filename) throws Exception
	{
		ArrayList lines = new ArrayList();

		//String name = filename.substring(filename.lastIndexOf("\\") + 1, filename.length());
		//lines.add(new String("interface "+name));
		//	lines.add(new String("{"));
			
		Enumeration names = constants.enumerateKeys();
		
		while(names.hasMoreElements())
		{
			Object const_name = names.nextElement();
			
			String line = "" + GetConstant_Int(constants, (String)const_name) + "=\"" + const_name + "\"";

			lines.add(line);
		}
			
		WriteLinesToFile(filename + ".list", false, lines.toArray());
	}
	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Returns the value of the constant with <constant_name> as (int)
	// ------------------------------------------------------------------------------------------------
	public static int GetConstant_Int (OrderedHashtable h, String const_name) throws Exception
	{
		Object value = h.get(const_name);
		
		if (value == null)
			throw new Exception("Cannot find java constant '"+const_name+"' in constants hashtable");
		else
			return ((Integer)value).intValue();
	}


// ------------------------------------------------------------------------------------------------
	/// @brief Returns the value of the constant with <constant_name> as String with a leading and trailing quote(")
	// ------------------------------------------------------------------------------------------------
	public static String GetConstant_String (OrderedHashtable h, String const_name) throws Exception
	{
		Object value = h.get(const_name);
		
		if (value == null)
			throw new Exception("Cannot find java constant '"+const_name+"' in constants hashtable");
		else
			return (String)value;
	}	

	
// ------------------------------------------------------------------------------------------------
	/// @brief Returns true if a define with <constant_name> in present in <h> (i.e. if a constant with that name (and of type Boolean) is present)
	// ------------------------------------------------------------------------------------------------
	public static boolean GetConstant_Define (OrderedHashtable h, String const_name) throws Exception
	{
		Object value = h.get(const_name);
		
		if (value == null)
			return false;
		else
			return (value.getClass().getName() == "Boolean");
	}	


// ------------------------------------------------------------------------------------------------
	/// @brief Returns true if a define with <constant_name> in present in <h> (i.e. if a constant with that name (and of type Boolean) is present)
	// ------------------------------------------------------------------------------------------------
	public static boolean IsConstantDefined (OrderedHashtable h, String const_name)
	{
		Object value = h.get(const_name);
		
		return (value != null);
	}

	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Writes to an already open file the int as little-endian
	// ------------------------------------------------------------------------------------------------
	public static void WriteToFile_U32 (FileOutputStream file, int value) throws Exception
	{
		file.write((byte)((value) & 0xFF));
		file.write((byte)((value >> 8) & 0xFF));
		file.write((byte)((value >> 16) & 0xFF));
		file.write((byte)((value >> 24) & 0xFF));
	}

	
// ------------------------------------------------------------------------------------------------
	/// @brief Writes to an already open file the int as little-endian
	// ------------------------------------------------------------------------------------------------
	public static void WriteToFile_U16 (FileOutputStream file, int value) throws Exception
	{
		file.write((byte)((value) & 0xFF));
		file.write((byte)((value >> 8) & 0xFF));
	}



// ------------------------------------------------------------------------------------------------
	/// @brief Returns an array with all worksheets' names in an XML Excel file
	// ------------------------------------------------------------------------------------------------
	public static String[] GetXMLSheetsNames (String filename)
	{
		try
		{
			File xmlFile = new File(filename);
			Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(xmlFile);
			
			NodeList worksheets = document.getElementsByTagName("Worksheet");
			
			String[] sheets_names = new String[worksheets.getLength()];
			
			for(int i = 0; i < worksheets.getLength(); i++)
				sheets_names[i] = worksheets.item(i).getAttributes().getNamedItem("ss:Name").getNodeValue();
			
			return sheets_names;
		}
		catch(Exception e) { System.out.println("FileProcessor.GetXMLSheets() error: "+e.toString()); e.printStackTrace(); }
		
		return null;
	}

	

// ------------------------------------------------------------------------------------------------
	/// @brief Parses an XML file generated by MS Excel and returns a sheet (with <sheetname>) as String[row_id][columns_id]
	/// @param sheetname is the case-sensitive name of the worksheet within the file
	/// @param ifdef_defines is a Hashtable containing defines according to which #ifdef/#else/#endif in the XML are parsed; null == no #ifdef/#else/#endif parsing
	/// @note In the returned array the strings corresponding to empty cells are == null (not empty strings!)
	// ------------------------------------------------------------------------------------------------
	public static String[][] GetXMLSheet (String filename, String sheetname, OrderedHashtable ifdef_defines)
	{
		try
		{
			InputSource input_source = new InputSource(new FileInputStream(filename));
			input_source.setEncoding(STRINGS_CHARSET);
			
			Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(input_source);
			
			NodeList worksheets = document.getElementsByTagName("Worksheet");
			Node table = null;
			for(int i = 0; i < worksheets.getLength(); i++)
				if (worksheets.item(i).getAttributes().getNamedItem("ss:Name").getNodeValue().compareTo(sheetname) == 0)
				{
					NodeList worksheet_children = worksheets.item(i).getChildNodes();
					
					for(int ii = 0; ii < worksheet_children.getLength(); ii++)
						if (worksheet_children.item(ii).getNodeName() == "Table")
							table = worksheet_children.item(ii);

					break;
				}

			if (table == null)
			{
				System.out.println("FileProcessor.GetXMLSheet(): cannot find sheet '"+sheetname+"'");
				return null;
			}
			
			NodeList table_children = table.getChildNodes();
			
			int table_height = 0;
			
			for(int i = 0; i < table_children.getLength(); i++)
			{
				if (table_children.item(i).getNodeName().compareTo("Row") == 0)
					table_height++;
			}
			
			int table_width = Integer.parseInt(table.getAttributes().getNamedItem("ss:ExpandedColumnCount").getNodeValue());
			
			ArrayList all_lines = new ArrayList();
			
			Ifdef_InitParsing();
			
			for(int i = 0; i < table_children.getLength(); i++)
			{
				if (table_children.item(i).getNodeName().compareTo("Row") == 0)
				{
					NamedNodeMap attrib = table_children.item(i).getAttributes();
					if( attrib!=null )
					{
						Node ss_Index = attrib.getNamedItem("ss:Index");
						if( ss_Index!=null )
						{
							int cell_counter = Integer.parseInt(ss_Index.getNodeValue()) - 1;
							
							while( all_lines.size() < cell_counter )
								all_lines.add( new String[table_width] );
						}
					}
										
					NodeList row_children = table_children.item(i).getChildNodes();
					
					String[] line = new String[table_width];
					
					boolean line_is_content = true;
					
					int cell_counter = 0;
					for(int ii = 0; ii < row_children.getLength(); ii++)
					{
						if (row_children.item(ii).getNodeName().compareTo("Cell") == 0)
						{
							Node ss_Index = row_children.item(ii).getAttributes().getNamedItem("ss:Index");
							
							if (ss_Index != null)
								cell_counter = Integer.parseInt(ss_Index.getNodeValue()) - 1;
								
							if (cell_counter >= table_width) break;
							
							if (row_children.item(ii).hasChildNodes())
							{
								String cell_val = row_children.item(ii).getFirstChild().getFirstChild().getNodeValue();
								
								line[cell_counter] = cell_val;
								
								if (ifdef_defines != null)
								{
									line_is_content = Ifdef_LineIsActive(ifdef_defines, cell_val);
									
									if (!line_is_content) break; // no parsing till the end of row
								}
							
								//System.out.println("CELL: '"+cell_val+"'");
							}
							
							cell_counter++;
						}
					}
					
					if (line_is_content)
					{
						//System.out.println("ADD LINE ---------" + i);
						all_lines.add(line);
					}
				}
			}
			
			String[][] strings = new String[all_lines.size()][];
			for(int i = 0; i < all_lines.size(); i++)
			{
				strings[i] = (String[])all_lines.get(i);
			}
			
			return strings;
		}
		catch(Exception e) { System.out.println("FileProcessor.GetXMLSheet() error: "+e.toString()); e.printStackTrace(); }
		
		return null;
	}


	
// ------------------------------------------------------------------------------------------------
	/// @brief Initializes engine for parsing content with #ifdef/#else/#endif
	// ------------------------------------------------------------------------------------------------
	static void Ifdef_InitParsing ()
	{
		Ifdef_inside_counter = 0;
		Ifdef_is_skipping = false;
		Ifdef_is_skipping_inside = 1;
	}
	

// ------------------------------------------------------------------------------------------------
	/// @brief Parses a document line for #ifdef/#else/#endif structure
	/// @note Call this with every line of parsed document
	/// @return true if the current line should be included in the parsed document
	// ------------------------------------------------------------------------------------------------
	static int Ifdef_inside_counter;
	static boolean Ifdef_is_skipping;
	static int Ifdef_is_skipping_inside;

	static boolean Ifdef_LineIsActive (OrderedHashtable ifdef_defines, String current_line)
	{
		if (ifdef_defines == null) return true;
		
		boolean line_is_content = true;
		
		String line = RemoveComment(current_line).trim();
		
		if (line.startsWith("#ifdef"))
		{
			line_is_content = false;
			
			String define_name = line.replaceFirst("\\#ifdef[\\s]", "");
			
			Ifdef_inside_counter++;
			
			if (!Ifdef_is_skipping && !IsConstantDefined(ifdef_defines, define_name))
			{
				Ifdef_is_skipping_inside = Ifdef_inside_counter;
				Ifdef_is_skipping = true;
				
				//System.out.println("Skipping data because '"+define_name+"' is not defined");
			}
		}
		else if (line.startsWith("#ifndef"))
		{
			line_is_content = false;
			
			String define_name = line.replaceFirst("\\#ifndef[\\s]", "");
			
			Ifdef_inside_counter++;
			
			if (!Ifdef_is_skipping && IsConstantDefined(ifdef_defines, define_name))
			{
				Ifdef_is_skipping_inside = Ifdef_inside_counter;
				Ifdef_is_skipping = true;
				
				//System.out.println("Skipping data because '"+define_name+"' is not defined");
			}
		}
		else if (line.startsWith("#else"))
		{
			line_is_content = false;
			
			if (Ifdef_inside_counter <= Ifdef_is_skipping_inside)
			{
				Ifdef_is_skipping = !Ifdef_is_skipping;
				//System.out.println("Invert skipping! (#else)");
			}
		}
		else if (line.startsWith("#endif"))
		{
			line_is_content = false;
			
			if (Ifdef_inside_counter <= Ifdef_is_skipping_inside)
			{
				Ifdef_is_skipping = !Ifdef_is_skipping;
				//System.out.println("Invert skipping! (#endif)");
			}
			
			Ifdef_inside_counter--;
		}
		
		return (line_is_content && !Ifdef_is_skipping);
	}

	

// ------------------------------------------------------------------------------------------------
	/// @brief Parses all strings in <strings> and performs operations given by <format>
	/// @param format can be FORMAT_NO_NULL | FORMAT_NO_COMMENTS | FORMAT_NO_EXCEL_CHARS | FORMAT_TRIM
	// ------------------------------------------------------------------------------------------------
	static void FormatSheetStrings (String[][] strings, int format)
	{
			for(int line = 0; line < strings.length; line++)
				for(int col = 0; col < strings[line].length; col++)
				{
					if ((format & FORMAT_NO_NULL) != 0 && strings[line][col] == null)
						strings[line][col] = "";
					
					if ((format & FORMAT_NO_COMMENTS) != 0)
						strings[line][col] = RemoveComment(strings[line][col]);
					
					if ((format & FORMAT_NO_EXCEL_CHARS) != 0)
						strings[line][col] = correctExcelString(strings[line][col]);
					
					if ((format & FORMAT_TRIM) != 0)
						strings[line][col] = strings[line][col].trim();
						
					if ((format & FORMAT_DELETE_QUOTES) != 0)
						strings[line][col] = DeleteStartEndQuotes( strings[line][col] );
						
				}
	}	
	
	
// ------------------------------------------------------------------------------------------------
	/// @brief Removes " characters from the start and end of a string but only if there are such characters on both the start and end of a string
	/// @param s is the string to trim the "s from
	// ------------------------------------------------------------------------------------------------
	static String DeleteStartEndQuotes (String s)
	{
		if(s!=null && s.startsWith("\"") && s.endsWith("\"") && s.length()>2)
			s = s.substring(1, s.length() - 1);

		return s;
	}

	
// ------------------------------------------------------------------------------------------------
	/// @brief Removes a comment starting with "//" and ending at the end of string
	/// @param s is the string to trim the "s from
	// ------------------------------------------------------------------------------------------------
	static String RemoveComment (String s)
	{
		int comment_pos = s.indexOf("//");
		
		if (comment_pos == -1) return s;
		
		return s.substring(0, comment_pos);
	}


// ------------------------------------------------------------------------------------------------
	/// @brief Replaces some common "Excel" characters like "…" with "normal" characters ("..." in this very case)
	/// @param s is the string to parse
	// ------------------------------------------------------------------------------------------------
	static String correctExcelString (String s)
	{
		if (s == null) return null;

		s = s.replace('“','"');
		s = s.replace('”','"');

		s = s.replace('’','\'');
		s = s.replaceAll("…","...");
		
		s = s.replaceAll("\\\\n","\n");

		return s;
	}
	
	
	// ------------------------------------------------------------------------------------------------
	/// @brief Returns a string, which is n time concatenation of the source string
	// ------------------------------------------------------------------------------------------------
	
	static String AddNString(String source, int n)
	{
		String res = "";
		for(int i = 0; i < n; i++)
		{
			res += source;
		}
		return res;
	}
	
}
