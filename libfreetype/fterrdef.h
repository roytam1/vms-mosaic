/***************************************************************************/
/*                                                                         */
/*  fterrdef.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
/*                                                                         */
/*  Copyright 2002, 2004, 2006, 2007 by                                    */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                LIST OF ERROR CODES/MESSAGES             *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/


  /* You need to define both FT_ERRORDEF_ and FT_NOERRORDEF_ before */
  /* including this file.                                           */


  /* generic errors */

#if !defined(VMS) || defined(__STDC__)
  FT_NOERRORDEF_( Ok,                                        0x00, \
                  "no error" )

  FT_ERRORDEF_( Cannot_Open_Resource,                        0x01, \
                "cannot open resource" )
  FT_ERRORDEF_( Unknown_File_Format,                         0x02, \
                "unknown file format" )
  FT_ERRORDEF_( Invalid_File_Format,                         0x03, \
                "broken file" )
  FT_ERRORDEF_( Invalid_Version,                             0x04, \
                "invalid FreeType version" )
  FT_ERRORDEF_( Lower_Module_Version,                        0x05, \
                "module version is too low" )
  FT_ERRORDEF_( Invalid_Argument,                            0x06, \
                "invalid argument" )
  FT_ERRORDEF_( Unimplemented_Feature,                       0x07, \
                "unimplemented feature" )
  FT_ERRORDEF_( Invalid_Table,                               0x08, \
                "broken table" )
  FT_ERRORDEF_( Invalid_Offset,                              0x09, \
                "broken offset within table" )
  FT_ERRORDEF_( Array_Too_Large,                             0x0A, \
                "array allocation size too large" )

  /* glyph/character errors */

  FT_ERRORDEF_( Invalid_Glyph_Index,                         0x10, \
                "invalid glyph index" )
  FT_ERRORDEF_( Invalid_Character_Code,                      0x11, \
                "invalid character code" )
  FT_ERRORDEF_( Invalid_Glyph_Format,                        0x12, \
                "unsupported glyph image format" )
  FT_ERRORDEF_( Cannot_Render_Glyph,                         0x13, \
                "cannot render this glyph format" )
  FT_ERRORDEF_( Invalid_Outline,                             0x14, \
                "invalid outline" )
  FT_ERRORDEF_( Invalid_Composite,                           0x15, \
                "invalid composite glyph" )
  FT_ERRORDEF_( Too_Many_Hints,                              0x16, \
                "too many hints" )
  FT_ERRORDEF_( Invalid_Pixel_Size,                          0x17, \
                "invalid pixel size" )

  /* handle errors */

  FT_ERRORDEF_( Invalid_Handle,                              0x20, \
                "invalid object handle" )
  FT_ERRORDEF_( Invalid_Library_Handle,                      0x21, \
                "invalid library handle" )
  FT_ERRORDEF_( Invalid_Driver_Handle,                       0x22, \
                "invalid module handle" )
  FT_ERRORDEF_( Invalid_Face_Handle,                         0x23, \
                "invalid face handle" )
  FT_ERRORDEF_( Invalid_Size_Handle,                         0x24, \
                "invalid size handle" )
  FT_ERRORDEF_( Invalid_Slot_Handle,                         0x25, \
                "invalid glyph slot handle" )
  FT_ERRORDEF_( Invalid_CharMap_Handle,                      0x26, \
                "invalid charmap handle" )
  FT_ERRORDEF_( Invalid_Cache_Handle,                        0x27, \
                "invalid cache manager handle" )
  FT_ERRORDEF_( Invalid_Stream_Handle,                       0x28, \
                "invalid stream handle" )

  /* driver errors */

  FT_ERRORDEF_( Too_Many_Drivers,                            0x30, \
                "too many modules" )
  FT_ERRORDEF_( Too_Many_Extensions,                         0x31, \
                "too many extensions" )

  /* memory errors */

  FT_ERRORDEF_( Out_Of_Memory,                               0x40, \
                "out of memory" )
  FT_ERRORDEF_( Unlisted_Object,                             0x41, \
                "unlisted object" )

  /* stream errors */

  FT_ERRORDEF_( Cannot_Open_Stream,                          0x51, \
                "cannot open stream" )
  FT_ERRORDEF_( Invalid_Stream_Seek,                         0x52, \
                "invalid stream seek" )
  FT_ERRORDEF_( Invalid_Stream_Skip,                         0x53, \
                "invalid stream skip" )
  FT_ERRORDEF_( Invalid_Stream_Read,                         0x54, \
                "invalid stream read" )
  FT_ERRORDEF_( Invalid_Stream_Operation,                    0x55, \
                "invalid stream operation" )
  FT_ERRORDEF_( Invalid_Frame_Operation,                     0x56, \
                "invalid frame operation" )
  FT_ERRORDEF_( Nested_Frame_Access,                         0x57, \
                "nested frame access" )
  FT_ERRORDEF_( Invalid_Frame_Read,                          0x58, \
                "invalid frame read" )

  /* raster errors */

  FT_ERRORDEF_( Raster_Uninitialized,                        0x60, \
                "raster uninitialized" )
  FT_ERRORDEF_( Raster_Corrupted,                            0x61, \
                "raster corrupted" )
  FT_ERRORDEF_( Raster_Overflow,                             0x62, \
                "raster overflow" )
  FT_ERRORDEF_( Raster_Negative_Height,                      0x63, \
                "negative height while rastering" )

  /* cache errors */

  FT_ERRORDEF_( Too_Many_Caches,                             0x70, \
                "too many registered caches" )

  /* TrueType and SFNT errors */

  FT_ERRORDEF_( Invalid_Opcode,                              0x80, \
                "invalid opcode" )
  FT_ERRORDEF_( Too_Few_Arguments,                           0x81, \
                "too few arguments" )
  FT_ERRORDEF_( Stack_Overflow,                              0x82, \
                "stack overflow" )
  FT_ERRORDEF_( Code_Overflow,                               0x83, \
                "code overflow" )
  FT_ERRORDEF_( Bad_Argument,                                0x84, \
                "bad argument" )
  FT_ERRORDEF_( Divide_By_Zero,                              0x85, \
                "division by zero" )
  FT_ERRORDEF_( Invalid_Reference,                           0x86, \
                "invalid reference" )
  FT_ERRORDEF_( Debug_OpCode,                                0x87, \
                "found debug opcode" )
  FT_ERRORDEF_( ENDF_In_Exec_Stream,                         0x88, \
                "found ENDF opcode in execution stream" )
  FT_ERRORDEF_( Nested_DEFS,                                 0x89, \
                "nested DEFS" )
  FT_ERRORDEF_( Invalid_CodeRange,                           0x8A, \
                "invalid code range" )
  FT_ERRORDEF_( Execution_Too_Long,                          0x8B, \
                "execution context too long" )
  FT_ERRORDEF_( Too_Many_Function_Defs,                      0x8C, \
                "too many function definitions" )
  FT_ERRORDEF_( Too_Many_Instruction_Defs,                   0x8D, \
                "too many instruction definitions" )
  FT_ERRORDEF_( Table_Missing,                               0x8E, \
                "SFNT font table missing" )
  FT_ERRORDEF_( Horiz_Header_Missing,                        0x8F, \
                "horizontal header (hhea) table missing" )
  FT_ERRORDEF_( Locations_Missing,                           0x90, \
                "locations (loca) table missing" )
  FT_ERRORDEF_( Name_Table_Missing,                          0x91, \
                "name table missing" )
  FT_ERRORDEF_( CMap_Table_Missing,                          0x92, \
                "character map (cmap) table missing" )
  FT_ERRORDEF_( Hmtx_Table_Missing,                          0x93, \
                "horizontal metrics (hmtx) table missing" )
  FT_ERRORDEF_( Post_Table_Missing,                          0x94, \
                "PostScript (post) table missing" )
  FT_ERRORDEF_( Invalid_Horiz_Metrics,                       0x95, \
                "invalid horizontal metrics" )
  FT_ERRORDEF_( Invalid_CharMap_Format,                      0x96, \
                "invalid character map (cmap) format" )
  FT_ERRORDEF_( Invalid_PPem,                                0x97, \
                "invalid ppem value" )
  FT_ERRORDEF_( Invalid_Vert_Metrics,                        0x98, \
                "invalid vertical metrics" )
  FT_ERRORDEF_( Could_Not_Find_Context,                      0x99, \
                "could not find context" )
  FT_ERRORDEF_( Invalid_Post_Table_Format,                   0x9A, \
                "invalid PostScript (post) table format" )
  FT_ERRORDEF_( Invalid_Post_Table,                          0x9B, \
                "invalid PostScript (post) table" )

  /* CFF, CID, and Type 1 errors */

  FT_ERRORDEF_( Syntax_Error,                                0xA0, \
                "opcode syntax error" )
  FT_ERRORDEF_( Stack_Underflow,                             0xA1, \
                "argument stack underflow" )
  FT_ERRORDEF_( Ignore,                                      0xA2, \
                "ignore" )

  /* BDF errors */

  FT_ERRORDEF_( Missing_Startfont_Field,                     0xB0, \
                "`STARTFONT' field missing" )
  FT_ERRORDEF_( Missing_Font_Field,                          0xB1, \
                "`FONT' field missing" )
  FT_ERRORDEF_( Missing_Size_Field,                          0xB2, \
                "`SIZE' field missing" )
  FT_ERRORDEF_( Missing_Chars_Field,                         0xB3, \
                "`CHARS' field missing" )
  FT_ERRORDEF_( Missing_Startchar_Field,                     0xB4, \
                "`STARTCHAR' field missing" )
  FT_ERRORDEF_( Missing_Encoding_Field,                      0xB5, \
                "`ENCODING' field missing" )
  FT_ERRORDEF_( Missing_Bbx_Field,                           0xB6, \
                "`BBX' field missing" )
  FT_ERRORDEF_( Bbx_Too_Big,                                 0xB7, \
                "`BBX' too big" )
  FT_ERRORDEF_( Corrupted_Font_Header,                       0xB8, \
                "Font header corrupted or missing fields" )
  FT_ERRORDEF_( Corrupted_Font_Glyphs,                       0xB9, \
                "Font glyphs corrupted or missing fields" )
#else
#ifndef DID_FTERRDEF
#define DID_FTERRDEF
enum {
       FT_Err_Ok = 0x00,
       FT_Err_Cannot_Open_Resource = 0x01 + 0,
       FT_Err_Unknown_File_Format = 0x02 + 0,
       FT_Err_Invalid_File_Format = 0x03 + 0,
       FT_Err_Invalid_Version = 0x04 + 0,
       FT_Err_Lower_Module_Version = 0x05 + 0,
       FT_Err_Invalid_Argument = 0x06 + 0,
       FT_Err_Unimplemented_Feature = 0x07 + 0,
       FT_Err_Invalid_Table = 0x08 + 0,
       FT_Err_Invalid_Offset = 0x09 + 0,
       FT_Err_Array_Too_Large = 0x0A + 0,
       FT_Err_Invalid_Glyph_Index = 0x10 + 0,
       FT_Err_Invalid_Character_Code = 0x11 + 0,
       FT_Err_Invalid_Glyph_Format = 0x12 + 0,
       FT_Err_Cannot_Render_Glyph = 0x13 + 0,
       FT_Err_Invalid_Outline = 0x14 + 0,
       FT_Err_Invalid_Composite = 0x15 + 0,
       FT_Err_Too_Many_Hints = 0x16 + 0,
       FT_Err_Invalid_Pixel_Size = 0x17 + 0,
       FT_Err_Invalid_Handle = 0x20 + 0,
       FT_Err_Invalid_Library_Handle = 0x21 + 0,
       FT_Err_Invalid_Driver_Handle = 0x22 + 0,
       FT_Err_Invalid_Face_Handle = 0x23 + 0,
       FT_Err_Invalid_Size_Handle = 0x24 + 0,
       FT_Err_Invalid_Slot_Handle = 0x25 + 0,
       FT_Err_Invalid_CharMap_Handle = 0x26 + 0,
       FT_Err_Invalid_Cache_Handle = 0x27 + 0,
       FT_Err_Invalid_Stream_Handle = 0x28 + 0,
       FT_Err_Too_Many_Drivers = 0x30 + 0,
       FT_Err_Too_Many_Extensions = 0x31 + 0,
       FT_Err_Out_Of_Memory = 0x40 + 0,
       FT_Err_Unlisted_Object = 0x41 + 0,
       FT_Err_Cannot_Open_Stream = 0x51 + 0,
       FT_Err_Invalid_Stream_Seek = 0x52 + 0,
       FT_Err_Invalid_Stream_Skip = 0x53 + 0,
       FT_Err_Invalid_Stream_Read = 0x54 + 0,
       FT_Err_Invalid_Stream_Operation = 0x55 + 0,
       FT_Err_Invalid_Frame_Operation = 0x56 + 0,
       FT_Err_Nested_Frame_Access = 0x57 + 0,
       FT_Err_Invalid_Frame_Read = 0x58 + 0,
       FT_Err_Raster_Uninitialized = 0x60 + 0,
       FT_Err_Raster_Corrupted = 0x61 + 0,
       FT_Err_Raster_Overflow = 0x62 + 0,
       FT_Err_Raster_Negative_Height = 0x63 + 0,
       FT_Err_Too_Many_Caches = 0x70 + 0,
       FT_Err_Invalid_Opcode = 0x80 + 0,
       FT_Err_Too_Few_Arguments = 0x81 + 0,
       FT_Err_Stack_Overflow = 0x82 + 0,
       FT_Err_Code_Overflow = 0x83 + 0,
       FT_Err_Bad_Argument = 0x84 + 0,
       FT_Err_Divide_By_Zero = 0x85 + 0,
       FT_Err_Invalid_Reference = 0x86 + 0,
       FT_Err_Debug_OpCode = 0x87 + 0,
       FT_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       FT_Err_Nested_DEFS = 0x89 + 0,
       FT_Err_Invalid_CodeRange = 0x8A + 0,
       FT_Err_Execution_Too_Long = 0x8B + 0,
       FT_Err_Too_Many_Function_Defs = 0x8C + 0,
       FT_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       FT_Err_Table_Missing = 0x8E + 0,
       FT_Err_Horiz_Header_Missing = 0x8F + 0,
       FT_Err_Locations_Missing = 0x90 + 0,
       FT_Err_Name_Table_Missing = 0x91 + 0,
       FT_Err_CMap_Table_Missing = 0x92 + 0,
       FT_Err_Hmtx_Table_Missing = 0x93 + 0,
       FT_Err_Post_Table_Missing = 0x94 + 0,
       FT_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       FT_Err_Invalid_CharMap_Format = 0x96 + 0,
       FT_Err_Invalid_PPem = 0x97 + 0,
       FT_Err_Invalid_Vert_Metrics = 0x98 + 0,
       FT_Err_Could_Not_Find_Context = 0x99 + 0,
       FT_Err_Invalid_Post_Table_Format = 0x9A + 0,
       FT_Err_Invalid_Post_Table = 0x9B + 0,
       FT_Err_Syntax_Error = 0xA0 + 0,
       FT_Err_Stack_Underflow = 0xA1 + 0,
       FT_Err_Ignore = 0xA2 + 0,
       FT_Err_Missing_Startfont_Field = 0xB0 + 0,
       FT_Err_Missing_Font_Field = 0xB1 + 0,
       FT_Err_Missing_Size_Field = 0xB2 + 0,
       FT_Err_Missing_Chars_Field = 0xB3 + 0,
       FT_Err_Missing_Startchar_Field = 0xB4 + 0,
       FT_Err_Missing_Encoding_Field = 0xB5 + 0,
       FT_Err_Missing_Bbx_Field = 0xB6 + 0,
       FT_Err_Bbx_Too_Big = 0xB7 + 0,
       FT_Err_Corrupted_Font_Header = 0xB8 + 0,
       FT_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       FT_Err_Max };
enum {
        Gzip_Err_Ok = 0x00,
	Gzip_Err_Invalid_File_Format = 0x03 + 0,
	Gzip_Err_Unimplemented_Feature = 0x07 + 0,
	Gzip_Err_Invalid_Stream_Operation = 0x55 + 0,
        Gzip_Err_Max };
enum {
        LZW_Err_Ok = 0x00,
	LZW_Err_Invalid_File_Format = 0x03 + 0,
	LZW_Err_Unimplemented_Feature = 0x07 + 0,
	LZW_Err_Invalid_Stream_Operation = 0x55 + 0,
        LZW_Err_Max };
enum {
        AF_Err_Ok = 0x00,
	AF_Err_Invalid_File_Format = 0x03 + 0,
        AF_Err_Invalid_Argument = 0x06 + 0,
	AF_Err_Unimplemented_Feature = 0x07 + 0,
        AF_Err_Invalid_Composite = 0x15 + 0,
        AF_Err_Out_Of_Memory = 0x40 + 0,
	AF_Err_Invalid_Stream_Operation = 0x55 + 0,
        AF_Err_Max };
enum {
       BDF_Err_Ok = 0x00,
       BDF_Err_Cannot_Open_Resource = 0x01 + 0,
       BDF_Err_Unknown_File_Format = 0x02 + 0,
       BDF_Err_Invalid_File_Format = 0x03 + 0,
       BDF_Err_Invalid_Version = 0x04 + 0,
       BDF_Err_Lower_Module_Version = 0x05 + 0,
       BDF_Err_Invalid_Argument = 0x06 + 0,
       BDF_Err_Unimplemented_Feature = 0x07 + 0,
       BDF_Err_Invalid_Table = 0x08 + 0,
       BDF_Err_Invalid_Offset = 0x09 + 0,
       BDF_Err_Array_Too_Large = 0x0A + 0,
       BDF_Err_Invalid_Glyph_Index = 0x10 + 0,
       BDF_Err_Invalid_Character_Code = 0x11 + 0,
       BDF_Err_Invalid_Glyph_Format = 0x12 + 0,
       BDF_Err_Cannot_Render_Glyph = 0x13 + 0,
       BDF_Err_Invalid_Outline = 0x14 + 0,
       BDF_Err_Invalid_Composite = 0x15 + 0,
       BDF_Err_Too_Many_Hints = 0x16 + 0,
       BDF_Err_Invalid_Pixel_Size = 0x17 + 0,
       BDF_Err_Invalid_Handle = 0x20 + 0,
       BDF_Err_Invalid_Library_Handle = 0x21 + 0,
       BDF_Err_Invalid_Driver_Handle = 0x22 + 0,
       BDF_Err_Invalid_Face_Handle = 0x23 + 0,
       BDF_Err_Invalid_Size_Handle = 0x24 + 0,
       BDF_Err_Invalid_Slot_Handle = 0x25 + 0,
       BDF_Err_Invalid_CharMap_Handle = 0x26 + 0,
       BDF_Err_Invalid_Cache_Handle = 0x27 + 0,
       BDF_Err_Invalid_Stream_Handle = 0x28 + 0,
       BDF_Err_Too_Many_Drivers = 0x30 + 0,
       BDF_Err_Too_Many_Extensions = 0x31 + 0,
       BDF_Err_Out_Of_Memory = 0x40 + 0,
       BDF_Err_Unlisted_Object = 0x41 + 0,
       BDF_Err_Cannot_Open_Stream = 0x51 + 0,
       BDF_Err_Invalid_Stream_Seek = 0x52 + 0,
       BDF_Err_Invalid_Stream_Skip = 0x53 + 0,
       BDF_Err_Invalid_Stream_Read = 0x54 + 0,
       BDF_Err_Invalid_Stream_Operation = 0x55 + 0,
       BDF_Err_Invalid_Frame_Operation = 0x56 + 0,
       BDF_Err_Nested_Frame_Access = 0x57 + 0,
       BDF_Err_Invalid_Frame_Read = 0x58 + 0,
       BDF_Err_Raster_Uninitialized = 0x60 + 0,
       BDF_Err_Raster_Corrupted = 0x61 + 0,
       BDF_Err_Raster_Overflow = 0x62 + 0,
       BDF_Err_Raster_Negative_Height = 0x63 + 0,
       BDF_Err_Too_Many_Caches = 0x70 + 0,
       BDF_Err_Invalid_Opcode = 0x80 + 0,
       BDF_Err_Too_Few_Arguments = 0x81 + 0,
       BDF_Err_Stack_Overflow = 0x82 + 0,
       BDF_Err_Code_Overflow = 0x83 + 0,
       BDF_Err_Bad_Argument = 0x84 + 0,
       BDF_Err_Divide_By_Zero = 0x85 + 0,
       BDF_Err_Invalid_Reference = 0x86 + 0,
       BDF_Err_Debug_OpCode = 0x87 + 0,
       BDF_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       BDF_Err_Nested_DEFS = 0x89 + 0,
       BDF_Err_Invalid_CodeRange = 0x8A + 0,
       BDF_Err_Execution_Too_Long = 0x8B + 0,
       BDF_Err_Too_Many_Function_Defs = 0x8C + 0,
       BDF_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       BDF_Err_Table_Missing = 0x8E + 0,
       BDF_Err_Horiz_Header_Missing = 0x8F + 0,
       BDF_Err_Locations_Missing = 0x90 + 0,
       BDF_Err_Name_Table_Missing = 0x91 + 0,
       BDF_Err_CMap_Table_Missing = 0x92 + 0,
       BDF_Err_Hmtx_Table_Missing = 0x93 + 0,
       BDF_Err_Post_Table_Missing = 0x94 + 0,
       BDF_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       BDF_Err_Invalid_CharMap_Format = 0x96 + 0,
       BDF_Err_Invalid_PPem = 0x97 + 0,
       BDF_Err_Invalid_Vert_Metrics = 0x98 + 0,
       BDF_Err_Could_Not_Find_Context = 0x99 + 0,
       BDF_Err_Invalid_Post_Table_Format = 0x9A + 0,
       BDF_Err_Invalid_Post_Table = 0x9B + 0,
       BDF_Err_Syntax_Error = 0xA0 + 0,
       BDF_Err_Stack_Underflow = 0xA1 + 0,
       BDF_Err_Ignore = 0xA2 + 0,
       BDF_Err_Missing_Startfont_Field = 0xB0 + 0,
       BDF_Err_Missing_Font_Field = 0xB1 + 0,
       BDF_Err_Missing_Size_Field = 0xB2 + 0,
       BDF_Err_Missing_Chars_Field = 0xB3 + 0,
       BDF_Err_Missing_Startchar_Field = 0xB4 + 0,
       BDF_Err_Missing_Encoding_Field = 0xB5 + 0,
       BDF_Err_Missing_Bbx_Field = 0xB6 + 0,
       BDF_Err_Bbx_Too_Big = 0xB7 + 0,
       BDF_Err_Corrupted_Font_Header = 0xB8 + 0,
       BDF_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       BDF_Err_Max };
enum {
       CFF_Err_Ok = 0x00,
       CFF_Err_Cannot_Open_Resource = 0x01 + 0,
       CFF_Err_Unknown_File_Format = 0x02 + 0,
       CFF_Err_Invalid_File_Format = 0x03 + 0,
       CFF_Err_Invalid_Version = 0x04 + 0,
       CFF_Err_Lower_Module_Version = 0x05 + 0,
       CFF_Err_Invalid_Argument = 0x06 + 0,
       CFF_Err_Unimplemented_Feature = 0x07 + 0,
       CFF_Err_Invalid_Table = 0x08 + 0,
       CFF_Err_Invalid_Offset = 0x09 + 0,
       CFF_Err_Array_Too_Large = 0x0A + 0,
       CFF_Err_Invalid_Glyph_Index = 0x10 + 0,
       CFF_Err_Invalid_Character_Code = 0x11 + 0,
       CFF_Err_Invalid_Glyph_Format = 0x12 + 0,
       CFF_Err_Cannot_Render_Glyph = 0x13 + 0,
       CFF_Err_Invalid_Outline = 0x14 + 0,
       CFF_Err_Invalid_Composite = 0x15 + 0,
       CFF_Err_Too_Many_Hints = 0x16 + 0,
       CFF_Err_Invalid_Pixel_Size = 0x17 + 0,
       CFF_Err_Invalid_Handle = 0x20 + 0,
       CFF_Err_Invalid_Library_Handle = 0x21 + 0,
       CFF_Err_Invalid_Driver_Handle = 0x22 + 0,
       CFF_Err_Invalid_Face_Handle = 0x23 + 0,
       CFF_Err_Invalid_Size_Handle = 0x24 + 0,
       CFF_Err_Invalid_Slot_Handle = 0x25 + 0,
       CFF_Err_Invalid_CharMap_Handle = 0x26 + 0,
       CFF_Err_Invalid_Cache_Handle = 0x27 + 0,
       CFF_Err_Invalid_Stream_Handle = 0x28 + 0,
       CFF_Err_Too_Many_Drivers = 0x30 + 0,
       CFF_Err_Too_Many_Extensions = 0x31 + 0,
       CFF_Err_Out_Of_Memory = 0x40 + 0,
       CFF_Err_Unlisted_Object = 0x41 + 0,
       CFF_Err_Cannot_Open_Stream = 0x51 + 0,
       CFF_Err_Invalid_Stream_Seek = 0x52 + 0,
       CFF_Err_Invalid_Stream_Skip = 0x53 + 0,
       CFF_Err_Invalid_Stream_Read = 0x54 + 0,
       CFF_Err_Invalid_Stream_Operation = 0x55 + 0,
       CFF_Err_Invalid_Frame_Operation = 0x56 + 0,
       CFF_Err_Nested_Frame_Access = 0x57 + 0,
       CFF_Err_Invalid_Frame_Read = 0x58 + 0,
       CFF_Err_Raster_Uninitialized = 0x60 + 0,
       CFF_Err_Raster_Corrupted = 0x61 + 0,
       CFF_Err_Raster_Overflow = 0x62 + 0,
       CFF_Err_Raster_Negative_Height = 0x63 + 0,
       CFF_Err_Too_Many_Caches = 0x70 + 0,
       CFF_Err_Invalid_Opcode = 0x80 + 0,
       CFF_Err_Too_Few_Arguments = 0x81 + 0,
       CFF_Err_Stack_Overflow = 0x82 + 0,
       CFF_Err_Code_Overflow = 0x83 + 0,
       CFF_Err_Bad_Argument = 0x84 + 0,
       CFF_Err_Divide_By_Zero = 0x85 + 0,
       CFF_Err_Invalid_Reference = 0x86 + 0,
       CFF_Err_Debug_OpCode = 0x87 + 0,
       CFF_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       CFF_Err_Nested_DEFS = 0x89 + 0,
       CFF_Err_Invalid_CodeRange = 0x8A + 0,
       CFF_Err_Execution_Too_Long = 0x8B + 0,
       CFF_Err_Too_Many_Function_Defs = 0x8C + 0,
       CFF_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       CFF_Err_Table_Missing = 0x8E + 0,
       CFF_Err_Horiz_Header_Missing = 0x8F + 0,
       CFF_Err_Locations_Missing = 0x90 + 0,
       CFF_Err_Name_Table_Missing = 0x91 + 0,
       CFF_Err_CMap_Table_Missing = 0x92 + 0,
       CFF_Err_Hmtx_Table_Missing = 0x93 + 0,
       CFF_Err_Post_Table_Missing = 0x94 + 0,
       CFF_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       CFF_Err_Invalid_CharMap_Format = 0x96 + 0,
       CFF_Err_Invalid_PPem = 0x97 + 0,
       CFF_Err_Invalid_Vert_Metrics = 0x98 + 0,
       CFF_Err_Could_Not_Find_Context = 0x99 + 0,
       CFF_Err_Invalid_Post_Table_Format = 0x9A + 0,
       CFF_Err_Invalid_Post_Table = 0x9B + 0,
       CFF_Err_Syntax_Error = 0xA0 + 0,
       CFF_Err_Stack_Underflow = 0xA1 + 0,
       CFF_Err_Ignore = 0xA2 + 0,
       CFF_Err_Missing_Startfont_Field = 0xB0 + 0,
       CFF_Err_Missing_Font_Field = 0xB1 + 0,
       CFF_Err_Missing_Size_Field = 0xB2 + 0,
       CFF_Err_Missing_Chars_Field = 0xB3 + 0,
       CFF_Err_Missing_Startchar_Field = 0xB4 + 0,
       CFF_Err_Missing_Encoding_Field = 0xB5 + 0,
       CFF_Err_Missing_Bbx_Field = 0xB6 + 0,
       CFF_Err_Bbx_Too_Big = 0xB7 + 0,
       CFF_Err_Corrupted_Font_Header = 0xB8 + 0,
       CFF_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       CFF_Err_Max };
enum {
       CID_Err_Ok = 0x00,
       CID_Err_Cannot_Open_Resource = 0x01 + 0,
       CID_Err_Unknown_File_Format = 0x02 + 0,
       CID_Err_Invalid_File_Format = 0x03 + 0,
       CID_Err_Invalid_Version = 0x04 + 0,
       CID_Err_Lower_Module_Version = 0x05 + 0,
       CID_Err_Invalid_Argument = 0x06 + 0,
       CID_Err_Unimplemented_Feature = 0x07 + 0,
       CID_Err_Invalid_Table = 0x08 + 0,
       CID_Err_Invalid_Offset = 0x09 + 0,
       CID_Err_Array_Too_Large = 0x0A + 0,
       CID_Err_Invalid_Glyph_Index = 0x10 + 0,
       CID_Err_Invalid_Character_Code = 0x11 + 0,
       CID_Err_Invalid_Glyph_Format = 0x12 + 0,
       CID_Err_Cannot_Render_Glyph = 0x13 + 0,
       CID_Err_Invalid_Outline = 0x14 + 0,
       CID_Err_Invalid_Composite = 0x15 + 0,
       CID_Err_Too_Many_Hints = 0x16 + 0,
       CID_Err_Invalid_Pixel_Size = 0x17 + 0,
       CID_Err_Invalid_Handle = 0x20 + 0,
       CID_Err_Invalid_Library_Handle = 0x21 + 0,
       CID_Err_Invalid_Driver_Handle = 0x22 + 0,
       CID_Err_Invalid_Face_Handle = 0x23 + 0,
       CID_Err_Invalid_Size_Handle = 0x24 + 0,
       CID_Err_Invalid_Slot_Handle = 0x25 + 0,
       CID_Err_Invalid_CharMap_Handle = 0x26 + 0,
       CID_Err_Invalid_Cache_Handle = 0x27 + 0,
       CID_Err_Invalid_Stream_Handle = 0x28 + 0,
       CID_Err_Too_Many_Drivers = 0x30 + 0,
       CID_Err_Too_Many_Extensions = 0x31 + 0,
       CID_Err_Out_Of_Memory = 0x40 + 0,
       CID_Err_Unlisted_Object = 0x41 + 0,
       CID_Err_Cannot_Open_Stream = 0x51 + 0,
       CID_Err_Invalid_Stream_Seek = 0x52 + 0,
       CID_Err_Invalid_Stream_Skip = 0x53 + 0,
       CID_Err_Invalid_Stream_Read = 0x54 + 0,
       CID_Err_Invalid_Stream_Operation = 0x55 + 0,
       CID_Err_Invalid_Frame_Operation = 0x56 + 0,
       CID_Err_Nested_Frame_Access = 0x57 + 0,
       CID_Err_Invalid_Frame_Read = 0x58 + 0,
       CID_Err_Raster_Uninitialized = 0x60 + 0,
       CID_Err_Raster_Corrupted = 0x61 + 0,
       CID_Err_Raster_Overflow = 0x62 + 0,
       CID_Err_Raster_Negative_Height = 0x63 + 0,
       CID_Err_Too_Many_Caches = 0x70 + 0,
       CID_Err_Invalid_Opcode = 0x80 + 0,
       CID_Err_Too_Few_Arguments = 0x81 + 0,
       CID_Err_Stack_Overflow = 0x82 + 0,
       CID_Err_Code_Overflow = 0x83 + 0,
       CID_Err_Bad_Argument = 0x84 + 0,
       CID_Err_Divide_By_Zero = 0x85 + 0,
       CID_Err_Invalid_Reference = 0x86 + 0,
       CID_Err_Debug_OpCode = 0x87 + 0,
       CID_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       CID_Err_Nested_DEFS = 0x89 + 0,
       CID_Err_Invalid_CodeRange = 0x8A + 0,
       CID_Err_Execution_Too_Long = 0x8B + 0,
       CID_Err_Too_Many_Function_Defs = 0x8C + 0,
       CID_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       CID_Err_Table_Missing = 0x8E + 0,
       CID_Err_Horiz_Header_Missing = 0x8F + 0,
       CID_Err_Locations_Missing = 0x90 + 0,
       CID_Err_Name_Table_Missing = 0x91 + 0,
       CID_Err_CMap_Table_Missing = 0x92 + 0,
       CID_Err_Hmtx_Table_Missing = 0x93 + 0,
       CID_Err_Post_Table_Missing = 0x94 + 0,
       CID_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       CID_Err_Invalid_CharMap_Format = 0x96 + 0,
       CID_Err_Invalid_PPem = 0x97 + 0,
       CID_Err_Invalid_Vert_Metrics = 0x98 + 0,
       CID_Err_Could_Not_Find_Context = 0x99 + 0,
       CID_Err_Invalid_Post_Table_Format = 0x9A + 0,
       CID_Err_Invalid_Post_Table = 0x9B + 0,
       CID_Err_Syntax_Error = 0xA0 + 0,
       CID_Err_Stack_Underflow = 0xA1 + 0,
       CID_Err_Ignore = 0xA2 + 0,
       CID_Err_Missing_Startfont_Field = 0xB0 + 0,
       CID_Err_Missing_Font_Field = 0xB1 + 0,
       CID_Err_Missing_Size_Field = 0xB2 + 0,
       CID_Err_Missing_Chars_Field = 0xB3 + 0,
       CID_Err_Missing_Startchar_Field = 0xB4 + 0,
       CID_Err_Missing_Encoding_Field = 0xB5 + 0,
       CID_Err_Missing_Bbx_Field = 0xB6 + 0,
       CID_Err_Bbx_Too_Big = 0xB7 + 0,
       CID_Err_Corrupted_Font_Header = 0xB8 + 0,
       CID_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       CID_Err_Max };

enum {
        FNT_Err_Ok = 0x00,
        FNT_Err_Unknown_File_Format = 0x02 + 0,
	FNT_Err_Invalid_File_Format = 0x03 + 0,
        FNT_Err_Invalid_Argument = 0x06 + 0,
	FNT_Err_Unimplemented_Feature = 0x07 + 0,
        FNT_Err_Invalid_Composite = 0x15 + 0,
        FNT_Err_Invalid_Pixel_Size = 0x17 + 0,
        FNT_Err_Out_Of_Memory = 0x40 + 0,
	FNT_Err_Invalid_Stream_Operation = 0x55 + 0,
        FNT_Err_Bad_Argument = 0x84 + 0,
        FNT_Err_Max };
enum {
       FTC_Err_Ok = 0x00,
       FTC_Err_Cannot_Open_Resource = 0x01 + 0,
       FTC_Err_Unknown_File_Format = 0x02 + 0,
       FTC_Err_Invalid_File_Format = 0x03 + 0,
       FTC_Err_Invalid_Version = 0x04 + 0,
       FTC_Err_Lower_Module_Version = 0x05 + 0,
       FTC_Err_Invalid_Argument = 0x06 + 0,
       FTC_Err_Unimplemented_Feature = 0x07 + 0,
       FTC_Err_Invalid_Table = 0x08 + 0,
       FTC_Err_Invalid_Offset = 0x09 + 0,
       FTC_Err_Array_Too_Large = 0x0A + 0,
       FTC_Err_Invalid_Glyph_Index = 0x10 + 0,
       FTC_Err_Invalid_Character_Code = 0x11 + 0,
       FTC_Err_Invalid_Glyph_Format = 0x12 + 0,
       FTC_Err_Cannot_Render_Glyph = 0x13 + 0,
       FTC_Err_Invalid_Outline = 0x14 + 0,
       FTC_Err_Invalid_Composite = 0x15 + 0,
       FTC_Err_Too_Many_Hints = 0x16 + 0,
       FTC_Err_Invalid_Pixel_Size = 0x17 + 0,
       FTC_Err_Invalid_Handle = 0x20 + 0,
       FTC_Err_Invalid_Library_Handle = 0x21 + 0,
       FTC_Err_Invalid_Driver_Handle = 0x22 + 0,
       FTC_Err_Invalid_Face_Handle = 0x23 + 0,
       FTC_Err_Invalid_Size_Handle = 0x24 + 0,
       FTC_Err_Invalid_Slot_Handle = 0x25 + 0,
       FTC_Err_Invalid_CharMap_Handle = 0x26 + 0,
       FTC_Err_Invalid_Cache_Handle = 0x27 + 0,
       FTC_Err_Invalid_Stream_Handle = 0x28 + 0,
       FTC_Err_Too_Many_Drivers = 0x30 + 0,
       FTC_Err_Too_Many_Extensions = 0x31 + 0,
       FTC_Err_Out_Of_Memory = 0x40 + 0,
       FTC_Err_Unlisted_Object = 0x41 + 0,
       FTC_Err_Cannot_Open_Stream = 0x51 + 0,
       FTC_Err_Invalid_Stream_Seek = 0x52 + 0,
       FTC_Err_Invalid_Stream_Skip = 0x53 + 0,
       FTC_Err_Invalid_Stream_Read = 0x54 + 0,
       FTC_Err_Invalid_Stream_Operation = 0x55 + 0,
       FTC_Err_Invalid_Frame_Operation = 0x56 + 0,
       FTC_Err_Nested_Frame_Access = 0x57 + 0,
       FTC_Err_Invalid_Frame_Read = 0x58 + 0,
       FTC_Err_Raster_Uninitialized = 0x60 + 0,
       FTC_Err_Raster_Corrupted = 0x61 + 0,
       FTC_Err_Raster_Overflow = 0x62 + 0,
       FTC_Err_Raster_Negative_Height = 0x63 + 0,
       FTC_Err_Too_Many_Caches = 0x70 + 0,
       FTC_Err_Invalid_Opcode = 0x80 + 0,
       FTC_Err_Too_Few_Arguments = 0x81 + 0,
       FTC_Err_Stack_Overflow = 0x82 + 0,
       FTC_Err_Code_Overflow = 0x83 + 0,
       FTC_Err_Bad_Argument = 0x84 + 0,
       FTC_Err_Divide_By_Zero = 0x85 + 0,
       FTC_Err_Invalid_Reference = 0x86 + 0,
       FTC_Err_Debug_OpCode = 0x87 + 0,
       FTC_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       FTC_Err_Nested_DEFS = 0x89 + 0,
       FTC_Err_Invalid_CodeRange = 0x8A + 0,
       FTC_Err_Execution_Too_Long = 0x8B + 0,
       FTC_Err_Too_Many_Function_Defs = 0x8C + 0,
       FTC_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       FTC_Err_Table_Missing = 0x8E + 0,
       FTC_Err_Horiz_Header_Missing = 0x8F + 0,
       FTC_Err_Locations_Missing = 0x90 + 0,
       FTC_Err_Name_Table_Missing = 0x91 + 0,
       FTC_Err_CMap_Table_Missing = 0x92 + 0,
       FTC_Err_Hmtx_Table_Missing = 0x93 + 0,
       FTC_Err_Post_Table_Missing = 0x94 + 0,
       FTC_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       FTC_Err_Invalid_CharMap_Format = 0x96 + 0,
       FTC_Err_Invalid_PPem = 0x97 + 0,
       FTC_Err_Invalid_Vert_Metrics = 0x98 + 0,
       FTC_Err_Could_Not_Find_Context = 0x99 + 0,
       FTC_Err_Invalid_Post_Table_Format = 0x9A + 0,
       FTC_Err_Invalid_Post_Table = 0x9B + 0,
       FTC_Err_Syntax_Error = 0xA0 + 0,
       FTC_Err_Stack_Underflow = 0xA1 + 0,
       FTC_Err_Ignore = 0xA2 + 0,
       FTC_Err_Missing_Startfont_Field = 0xB0 + 0,
       FTC_Err_Missing_Font_Field = 0xB1 + 0,
       FTC_Err_Missing_Size_Field = 0xB2 + 0,
       FTC_Err_Missing_Chars_Field = 0xB3 + 0,
       FTC_Err_Missing_Startchar_Field = 0xB4 + 0,
       FTC_Err_Missing_Encoding_Field = 0xB5 + 0,
       FTC_Err_Missing_Bbx_Field = 0xB6 + 0,
       FTC_Err_Bbx_Too_Big = 0xB7 + 0,
       FTC_Err_Corrupted_Font_Header = 0xB8 + 0,
       FTC_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       FTC_Err_Max };
enum {
       Smooth_Err_Ok = 0x00,
       Smooth_Err_Cannot_Open_Resource = 0x01 + 0,
       Smooth_Err_Unknown_File_Format = 0x02 + 0,
       Smooth_Err_Invalid_File_Format = 0x03 + 0,
       Smooth_Err_Invalid_Version = 0x04 + 0,
       Smooth_Err_Lower_Module_Version = 0x05 + 0,
       Smooth_Err_Invalid_Argument = 0x06 + 0,
       Smooth_Err_Unimplemented_Feature = 0x07 + 0,
       Smooth_Err_Invalid_Table = 0x08 + 0,
       Smooth_Err_Invalid_Offset = 0x09 + 0,
       Smooth_Err_Array_Too_Large = 0x0A + 0,
       Smooth_Err_Invalid_Glyph_Index = 0x10 + 0,
       Smooth_Err_Invalid_Character_Code = 0x11 + 0,
       Smooth_Err_Invalid_Glyph_Format = 0x12 + 0,
       Smooth_Err_Cannot_Render_Glyph = 0x13 + 0,
       Smooth_Err_Invalid_Outline = 0x14 + 0,
       Smooth_Err_Invalid_Composite = 0x15 + 0,
       Smooth_Err_Too_Many_Hints = 0x16 + 0,
       Smooth_Err_Invalid_Pixel_Size = 0x17 + 0,
       Smooth_Err_Invalid_Handle = 0x20 + 0,
       Smooth_Err_Invalid_Library_Handle = 0x21 + 0,
       Smooth_Err_Invalid_Driver_Handle = 0x22 + 0,
       Smooth_Err_Invalid_Face_Handle = 0x23 + 0,
       Smooth_Err_Invalid_Size_Handle = 0x24 + 0,
       Smooth_Err_Invalid_Slot_Handle = 0x25 + 0,
       Smooth_Err_Invalid_CharMap_Handle = 0x26 + 0,
       Smooth_Err_Invalid_Cache_Handle = 0x27 + 0,
       Smooth_Err_Invalid_Stream_Handle = 0x28 + 0,
       Smooth_Err_Too_Many_Drivers = 0x30 + 0,
       Smooth_Err_Too_Many_Extensions = 0x31 + 0,
       Smooth_Err_Out_Of_Memory = 0x40 + 0,
       Smooth_Err_Unlisted_Object = 0x41 + 0,
       Smooth_Err_Cannot_Open_Stream = 0x51 + 0,
       Smooth_Err_Invalid_Stream_Seek = 0x52 + 0,
       Smooth_Err_Invalid_Stream_Skip = 0x53 + 0,
       Smooth_Err_Invalid_Stream_Read = 0x54 + 0,
       Smooth_Err_Invalid_Stream_Operation = 0x55 + 0,
       Smooth_Err_Invalid_Frame_Operation = 0x56 + 0,
       Smooth_Err_Nested_Frame_Access = 0x57 + 0,
       Smooth_Err_Invalid_Frame_Read = 0x58 + 0,
       Smooth_Err_Raster_Uninitialized = 0x60 + 0,
       Smooth_Err_Raster_Corrupted = 0x61 + 0,
       Smooth_Err_Raster_Overflow = 0x62 + 0,
       Smooth_Err_Raster_Negative_Height = 0x63 + 0,
       Smooth_Err_Too_Many_Caches = 0x70 + 0,
       Smooth_Err_Invalid_Opcode = 0x80 + 0,
       Smooth_Err_Too_Few_Arguments = 0x81 + 0,
       Smooth_Err_Stack_Overflow = 0x82 + 0,
       Smooth_Err_Code_Overflow = 0x83 + 0,
       Smooth_Err_Bad_Argument = 0x84 + 0,
       Smooth_Err_Divide_By_Zero = 0x85 + 0,
       Smooth_Err_Invalid_Reference = 0x86 + 0,
       Smooth_Err_Debug_OpCode = 0x87 + 0,
       Smooth_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       Smooth_Err_Nested_DEFS = 0x89 + 0,
       Smooth_Err_Invalid_CodeRange = 0x8A + 0,
       Smooth_Err_Execution_Too_Long = 0x8B + 0,
       Smooth_Err_Too_Many_Function_Defs = 0x8C + 0,
       Smooth_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       Smooth_Err_Table_Missing = 0x8E + 0,
       Smooth_Err_Horiz_Header_Missing = 0x8F + 0,
       Smooth_Err_Locations_Missing = 0x90 + 0,
       Smooth_Err_Name_Table_Missing = 0x91 + 0,
       Smooth_Err_CMap_Table_Missing = 0x92 + 0,
       Smooth_Err_Hmtx_Table_Missing = 0x93 + 0,
       Smooth_Err_Post_Table_Missing = 0x94 + 0,
       Smooth_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       Smooth_Err_Invalid_CharMap_Format = 0x96 + 0,
       Smooth_Err_Invalid_PPem = 0x97 + 0,
       Smooth_Err_Invalid_Vert_Metrics = 0x98 + 0,
       Smooth_Err_Could_Not_Find_Context = 0x99 + 0,
       Smooth_Err_Invalid_Post_Table_Format = 0x9A + 0,
       Smooth_Err_Invalid_Post_Table = 0x9B + 0,
       Smooth_Err_Syntax_Error = 0xA0 + 0,
       Smooth_Err_Stack_Underflow = 0xA1 + 0,
       Smooth_Err_Ignore = 0xA2 + 0,
       Smooth_Err_Missing_Startfont_Field = 0xB0 + 0,
       Smooth_Err_Missing_Font_Field = 0xB1 + 0,
       Smooth_Err_Missing_Size_Field = 0xB2 + 0,
       Smooth_Err_Missing_Chars_Field = 0xB3 + 0,
       Smooth_Err_Missing_Startchar_Field = 0xB4 + 0,
       Smooth_Err_Missing_Encoding_Field = 0xB5 + 0,
       Smooth_Err_Missing_Bbx_Field = 0xB6 + 0,
       Smooth_Err_Bbx_Too_Big = 0xB7 + 0,
       Smooth_Err_Corrupted_Font_Header = 0xB8 + 0,
       Smooth_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       Smooth_Err_Max };

enum {
        GXV_Err_Ok = 0x00,
	GXV_Err_Invalid_File_Format = 0x03 + 0,
        GXV_Err_Invalid_Argument = 0x06 + 0,
	GXV_Err_Unimplemented_Feature = 0x07 + 0,
        GXV_Err_Invalid_Composite = 0x15 + 0,
        GXV_Err_Invalid_Pixel_Size = 0x17 + 0,
        GXV_Err_Out_Of_Memory = 0x40 + 0,
	GXV_Err_Invalid_Stream_Operation = 0x55 + 0,
        GXV_Err_Bad_Argument = 0x84 + 0,
        GXV_Err_Table_Missing = 0x8E + 0,
	GXV_Err_Max };

enum {
        OTV_Err_Ok = 0x00,
	OTV_Err_Invalid_File_Format = 0x03 + 0,
        OTV_Err_Invalid_Argument = 0x06 + 0,
	OTV_Err_Unimplemented_Feature = 0x07 + 0,
        OTV_Err_Invalid_Table = 0x08 + 0,
        OTV_Err_Invalid_Offset = 0x09 + 0,
        OTV_Err_Invalid_Composite = 0x15 + 0,
        OTV_Err_Invalid_Pixel_Size = 0x17 + 0,
        OTV_Err_Out_Of_Memory = 0x40 + 0,
	OTV_Err_Invalid_Stream_Operation = 0x55 + 0,
        OTV_Err_Bad_Argument = 0x84 + 0,
        OTV_Err_Table_Missing = 0x8E + 0,
        OTV_Err_Max };

enum {
       PCF_Err_Ok = 0x00,
       PCF_Err_Cannot_Open_Resource = 0x01 + 0,
       PCF_Err_Unknown_File_Format = 0x02 + 0,
       PCF_Err_Invalid_File_Format = 0x03 + 0,
       PCF_Err_Invalid_Version = 0x04 + 0,
       PCF_Err_Lower_Module_Version = 0x05 + 0,
       PCF_Err_Invalid_Argument = 0x06 + 0,
       PCF_Err_Unimplemented_Feature = 0x07 + 0,
       PCF_Err_Invalid_Table = 0x08 + 0,
       PCF_Err_Invalid_Offset = 0x09 + 0,
       PCF_Err_Array_Too_Large = 0x0A + 0,
       PCF_Err_Invalid_Glyph_Index = 0x10 + 0,
       PCF_Err_Invalid_Character_Code = 0x11 + 0,
       PCF_Err_Invalid_Glyph_Format = 0x12 + 0,
       PCF_Err_Cannot_Render_Glyph = 0x13 + 0,
       PCF_Err_Invalid_Outline = 0x14 + 0,
       PCF_Err_Invalid_Composite = 0x15 + 0,
       PCF_Err_Too_Many_Hints = 0x16 + 0,
       PCF_Err_Invalid_Pixel_Size = 0x17 + 0,
       PCF_Err_Invalid_Handle = 0x20 + 0,
       PCF_Err_Invalid_Library_Handle = 0x21 + 0,
       PCF_Err_Invalid_Driver_Handle = 0x22 + 0,
       PCF_Err_Invalid_Face_Handle = 0x23 + 0,
       PCF_Err_Invalid_Size_Handle = 0x24 + 0,
       PCF_Err_Invalid_Slot_Handle = 0x25 + 0,
       PCF_Err_Invalid_CharMap_Handle = 0x26 + 0,
       PCF_Err_Invalid_Cache_Handle = 0x27 + 0,
       PCF_Err_Invalid_Stream_Handle = 0x28 + 0,
       PCF_Err_Too_Many_Drivers = 0x30 + 0,
       PCF_Err_Too_Many_Extensions = 0x31 + 0,
       PCF_Err_Out_Of_Memory = 0x40 + 0,
       PCF_Err_Unlisted_Object = 0x41 + 0,
       PCF_Err_Cannot_Open_Stream = 0x51 + 0,
       PCF_Err_Invalid_Stream_Seek = 0x52 + 0,
       PCF_Err_Invalid_Stream_Skip = 0x53 + 0,
       PCF_Err_Invalid_Stream_Read = 0x54 + 0,
       PCF_Err_Invalid_Stream_Operation = 0x55 + 0,
       PCF_Err_Invalid_Frame_Operation = 0x56 + 0,
       PCF_Err_Nested_Frame_Access = 0x57 + 0,
       PCF_Err_Invalid_Frame_Read = 0x58 + 0,
       PCF_Err_Raster_Uninitialized = 0x60 + 0,
       PCF_Err_Raster_Corrupted = 0x61 + 0,
       PCF_Err_Raster_Overflow = 0x62 + 0,
       PCF_Err_Raster_Negative_Height = 0x63 + 0,
       PCF_Err_Too_Many_Caches = 0x70 + 0,
       PCF_Err_Invalid_Opcode = 0x80 + 0,
       PCF_Err_Too_Few_Arguments = 0x81 + 0,
       PCF_Err_Stack_Overflow = 0x82 + 0,
       PCF_Err_Code_Overflow = 0x83 + 0,
       PCF_Err_Bad_Argument = 0x84 + 0,
       PCF_Err_Divide_By_Zero = 0x85 + 0,
       PCF_Err_Invalid_Reference = 0x86 + 0,
       PCF_Err_Debug_OpCode = 0x87 + 0,
       PCF_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       PCF_Err_Nested_DEFS = 0x89 + 0,
       PCF_Err_Invalid_CodeRange = 0x8A + 0,
       PCF_Err_Execution_Too_Long = 0x8B + 0,
       PCF_Err_Too_Many_Function_Defs = 0x8C + 0,
       PCF_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       PCF_Err_Table_Missing = 0x8E + 0,
       PCF_Err_Horiz_Header_Missing = 0x8F + 0,
       PCF_Err_Locations_Missing = 0x90 + 0,
       PCF_Err_Name_Table_Missing = 0x91 + 0,
       PCF_Err_CMap_Table_Missing = 0x92 + 0,
       PCF_Err_Hmtx_Table_Missing = 0x93 + 0,
       PCF_Err_Post_Table_Missing = 0x94 + 0,
       PCF_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       PCF_Err_Invalid_CharMap_Format = 0x96 + 0,
       PCF_Err_Invalid_PPem = 0x97 + 0,
       PCF_Err_Invalid_Vert_Metrics = 0x98 + 0,
       PCF_Err_Could_Not_Find_Context = 0x99 + 0,
       PCF_Err_Invalid_Post_Table_Format = 0x9A + 0,
       PCF_Err_Invalid_Post_Table = 0x9B + 0,
       PCF_Err_Syntax_Error = 0xA0 + 0,
       PCF_Err_Stack_Underflow = 0xA1 + 0,
       PCF_Err_Ignore = 0xA2 + 0,
       PCF_Err_Missing_Startfont_Field = 0xB0 + 0,
       PCF_Err_Missing_Font_Field = 0xB1 + 0,
       PCF_Err_Missing_Size_Field = 0xB2 + 0,
       PCF_Err_Missing_Chars_Field = 0xB3 + 0,
       PCF_Err_Missing_Startchar_Field = 0xB4 + 0,
       PCF_Err_Missing_Encoding_Field = 0xB5 + 0,
       PCF_Err_Missing_Bbx_Field = 0xB6 + 0,
       PCF_Err_Bbx_Too_Big = 0xB7 + 0,
       PCF_Err_Corrupted_Font_Header = 0xB8 + 0,
       PCF_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       PCF_Err_Max };

enum {
        PFR_Err_Ok = 0x00,
        PFR_Err_Unknown_File_Format = 0x02 + 0,
	PFR_Err_Invalid_File_Format = 0x03 + 0,
        PFR_Err_Invalid_Argument = 0x06 + 0,
	PFR_Err_Unimplemented_Feature = 0x07 + 0,
        PFR_Err_Invalid_Table = 0x08 + 0,
        PFR_Err_Invalid_Composite = 0x15 + 0,
        PFR_Err_Invalid_Pixel_Size = 0x17 + 0,
        PFR_Err_Out_Of_Memory = 0x40 + 0,
	PFR_Err_Invalid_Stream_Operation = 0x55 + 0,
        PFR_Err_Bad_Argument = 0x84 + 0,
        PFR_Err_Table_Missing = 0x8E + 0,
        PFR_Err_Max };

enum {
       PSaux_Err_Ok = 0x00,
       PSaux_Err_Cannot_Open_Resource = 0x01 + 0,
       PSaux_Err_Unknown_File_Format = 0x02 + 0,
       PSaux_Err_Invalid_File_Format = 0x03 + 0,
       PSaux_Err_Invalid_Version = 0x04 + 0,
       PSaux_Err_Lower_Module_Version = 0x05 + 0,
       PSaux_Err_Invalid_Argument = 0x06 + 0,
       PSaux_Err_Unimplemented_Feature = 0x07 + 0,
       PSaux_Err_Invalid_Table = 0x08 + 0,
       PSaux_Err_Invalid_Offset = 0x09 + 0,
       PSaux_Err_Array_Too_Large = 0x0A + 0,
       PSaux_Err_Invalid_Glyph_Index = 0x10 + 0,
       PSaux_Err_Invalid_Character_Code = 0x11 + 0,
       PSaux_Err_Invalid_Glyph_Format = 0x12 + 0,
       PSaux_Err_Cannot_Render_Glyph = 0x13 + 0,
       PSaux_Err_Invalid_Outline = 0x14 + 0,
       PSaux_Err_Invalid_Composite = 0x15 + 0,
       PSaux_Err_Too_Many_Hints = 0x16 + 0,
       PSaux_Err_Invalid_Pixel_Size = 0x17 + 0,
       PSaux_Err_Invalid_Handle = 0x20 + 0,
       PSaux_Err_Invalid_Library_Handle = 0x21 + 0,
       PSaux_Err_Invalid_Driver_Handle = 0x22 + 0,
       PSaux_Err_Invalid_Face_Handle = 0x23 + 0,
       PSaux_Err_Invalid_Size_Handle = 0x24 + 0,
       PSaux_Err_Invalid_Slot_Handle = 0x25 + 0,
       PSaux_Err_Invalid_CharMap_Handle = 0x26 + 0,
       PSaux_Err_Invalid_Cache_Handle = 0x27 + 0,
       PSaux_Err_Invalid_Stream_Handle = 0x28 + 0,
       PSaux_Err_Too_Many_Drivers = 0x30 + 0,
       PSaux_Err_Too_Many_Extensions = 0x31 + 0,
       PSaux_Err_Out_Of_Memory = 0x40 + 0,
       PSaux_Err_Unlisted_Object = 0x41 + 0,
       PSaux_Err_Cannot_Open_Stream = 0x51 + 0,
       PSaux_Err_Invalid_Stream_Seek = 0x52 + 0,
       PSaux_Err_Invalid_Stream_Skip = 0x53 + 0,
       PSaux_Err_Invalid_Stream_Read = 0x54 + 0,
       PSaux_Err_Invalid_Stream_Operation = 0x55 + 0,
       PSaux_Err_Invalid_Frame_Operation = 0x56 + 0,
       PSaux_Err_Nested_Frame_Access = 0x57 + 0,
       PSaux_Err_Invalid_Frame_Read = 0x58 + 0,
       PSaux_Err_Raster_Uninitialized = 0x60 + 0,
       PSaux_Err_Raster_Corrupted = 0x61 + 0,
       PSaux_Err_Raster_Overflow = 0x62 + 0,
       PSaux_Err_Raster_Negative_Height = 0x63 + 0,
       PSaux_Err_Too_Many_Caches = 0x70 + 0,
       PSaux_Err_Invalid_Opcode = 0x80 + 0,
       PSaux_Err_Too_Few_Arguments = 0x81 + 0,
       PSaux_Err_Stack_Overflow = 0x82 + 0,
       PSaux_Err_Code_Overflow = 0x83 + 0,
       PSaux_Err_Bad_Argument = 0x84 + 0,
       PSaux_Err_Divide_By_Zero = 0x85 + 0,
       PSaux_Err_Invalid_Reference = 0x86 + 0,
       PSaux_Err_Debug_OpCode = 0x87 + 0,
       PSaux_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       PSaux_Err_Nested_DEFS = 0x89 + 0,
       PSaux_Err_Invalid_CodeRange = 0x8A + 0,
       PSaux_Err_Execution_Too_Long = 0x8B + 0,
       PSaux_Err_Too_Many_Function_Defs = 0x8C + 0,
       PSaux_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       PSaux_Err_Table_Missing = 0x8E + 0,
       PSaux_Err_Horiz_Header_Missing = 0x8F + 0,
       PSaux_Err_Locations_Missing = 0x90 + 0,
       PSaux_Err_Name_Table_Missing = 0x91 + 0,
       PSaux_Err_CMap_Table_Missing = 0x92 + 0,
       PSaux_Err_Hmtx_Table_Missing = 0x93 + 0,
       PSaux_Err_Post_Table_Missing = 0x94 + 0,
       PSaux_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       PSaux_Err_Invalid_CharMap_Format = 0x96 + 0,
       PSaux_Err_Invalid_PPem = 0x97 + 0,
       PSaux_Err_Invalid_Vert_Metrics = 0x98 + 0,
       PSaux_Err_Could_Not_Find_Context = 0x99 + 0,
       PSaux_Err_Invalid_Post_Table_Format = 0x9A + 0,
       PSaux_Err_Invalid_Post_Table = 0x9B + 0,
       PSaux_Err_Syntax_Error = 0xA0 + 0,
       PSaux_Err_Stack_Underflow = 0xA1 + 0,
       PSaux_Err_Ignore = 0xA2 + 0,
       PSaux_Err_Missing_Startfont_Field = 0xB0 + 0,
       PSaux_Err_Missing_Font_Field = 0xB1 + 0,
       PSaux_Err_Missing_Size_Field = 0xB2 + 0,
       PSaux_Err_Missing_Chars_Field = 0xB3 + 0,
       PSaux_Err_Missing_Startchar_Field = 0xB4 + 0,
       PSaux_Err_Missing_Encoding_Field = 0xB5 + 0,
       PSaux_Err_Missing_Bbx_Field = 0xB6 + 0,
       PSaux_Err_Bbx_Too_Big = 0xB7 + 0,
       PSaux_Err_Corrupted_Font_Header = 0xB8 + 0,
       PSaux_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       PSaux_Err_Max };

enum {
        PSH_Err_Ok = 0x00,
        PSH_Err_Unknown_File_Format = 0x02 + 0,
	PSH_Err_Invalid_File_Format = 0x03 + 0,
        PSH_Err_Invalid_Argument = 0x06 + 0,
	PSH_Err_Unimplemented_Feature = 0x07 + 0,
        PSH_Err_Invalid_Composite = 0x15 + 0,
        PSH_Err_Invalid_Pixel_Size = 0x17 + 0,
        PSH_Err_Out_Of_Memory = 0x40 + 0,
	PSH_Err_Invalid_Stream_Operation = 0x55 + 0,
        PSH_Err_Bad_Argument = 0x84 + 0,
        PSH_Err_Table_Missing = 0x8E + 0,
        PSH_Err_Max };

enum {
        PSnames_Err_Ok = 0x00,
        PSnames_Err_Unknown_File_Format = 0x02 + 0,
	PSnames_Err_Invalid_File_Format = 0x03 + 0,
        PSnames_Err_Invalid_Argument = 0x06 + 0,
	PSnames_Err_Unimplemented_Feature = 0x07 + 0,
        PSnames_Err_Invalid_Composite = 0x15 + 0,
        PSnames_Err_Invalid_Pixel_Size = 0x17 + 0,
        PSnames_Err_Out_Of_Memory = 0x40 + 0,
	PSnames_Err_Invalid_Stream_Operation = 0x55 + 0,
        PSnames_Err_Bad_Argument = 0x84 + 0,
        PSnames_Err_Table_Missing = 0x8E + 0,
        PSnames_Err_Max };

enum {
       Raster_Err_Ok = 0x00,
       Raster_Err_Cannot_Open_Resource = 0x01 + 0,
       Raster_Err_Unknown_File_Format = 0x02 + 0,
       Raster_Err_Invalid_File_Format = 0x03 + 0,
       Raster_Err_Invalid_Version = 0x04 + 0,
       Raster_Err_Lower_Module_Version = 0x05 + 0,
       Raster_Err_Invalid_Argument = 0x06 + 0,
       Raster_Err_Unimplemented_Feature = 0x07 + 0,
       Raster_Err_Invalid_Table = 0x08 + 0,
       Raster_Err_Invalid_Offset = 0x09 + 0,
       Raster_Err_Array_Too_Large = 0x0A + 0,
       Raster_Err_Invalid_Glyph_Index = 0x10 + 0,
       Raster_Err_Invalid_Character_Code = 0x11 + 0,
       Raster_Err_Invalid_Glyph_Format = 0x12 + 0,
       Raster_Err_Cannot_Render_Glyph = 0x13 + 0,
       Raster_Err_Invalid_Outline = 0x14 + 0,
       Raster_Err_Invalid_Composite = 0x15 + 0,
       Raster_Err_Too_Many_Hints = 0x16 + 0,
       Raster_Err_Invalid_Pixel_Size = 0x17 + 0,
       Raster_Err_Invalid_Handle = 0x20 + 0,
       Raster_Err_Invalid_Library_Handle = 0x21 + 0,
       Raster_Err_Invalid_Driver_Handle = 0x22 + 0,
       Raster_Err_Invalid_Face_Handle = 0x23 + 0,
       Raster_Err_Invalid_Size_Handle = 0x24 + 0,
       Raster_Err_Invalid_Slot_Handle = 0x25 + 0,
       Raster_Err_Invalid_CharMap_Handle = 0x26 + 0,
       Raster_Err_Invalid_Cache_Handle = 0x27 + 0,
       Raster_Err_Invalid_Stream_Handle = 0x28 + 0,
       Raster_Err_Too_Many_Drivers = 0x30 + 0,
       Raster_Err_Too_Many_Extensions = 0x31 + 0,
       Raster_Err_Out_Of_Memory = 0x40 + 0,
       Raster_Err_Unlisted_Object = 0x41 + 0,
       Raster_Err_Cannot_Open_Stream = 0x51 + 0,
       Raster_Err_Invalid_Stream_Seek = 0x52 + 0,
       Raster_Err_Invalid_Stream_Skip = 0x53 + 0,
       Raster_Err_Invalid_Stream_Read = 0x54 + 0,
       Raster_Err_Invalid_Stream_Operation = 0x55 + 0,
       Raster_Err_Invalid_Frame_Operation = 0x56 + 0,
       Raster_Err_Nested_Frame_Access = 0x57 + 0,
       Raster_Err_Invalid_Frame_Read = 0x58 + 0,
       Raster_Err_Raster_Uninitialized = 0x60 + 0,
       Raster_Err_Raster_Corrupted = 0x61 + 0,
       Raster_Err_Raster_Overflow = 0x62 + 0,
       Raster_Err_Raster_Negative_Height = 0x63 + 0,
       Raster_Err_Too_Many_Caches = 0x70 + 0,
       Raster_Err_Invalid_Opcode = 0x80 + 0,
       Raster_Err_Too_Few_Arguments = 0x81 + 0,
       Raster_Err_Stack_Overflow = 0x82 + 0,
       Raster_Err_Code_Overflow = 0x83 + 0,
       Raster_Err_Bad_Argument = 0x84 + 0,
       Raster_Err_Divide_By_Zero = 0x85 + 0,
       Raster_Err_Invalid_Reference = 0x86 + 0,
       Raster_Err_Debug_OpCode = 0x87 + 0,
       Raster_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       Raster_Err_Nested_DEFS = 0x89 + 0,
       Raster_Err_Invalid_CodeRange = 0x8A + 0,
       Raster_Err_Execution_Too_Long = 0x8B + 0,
       Raster_Err_Too_Many_Function_Defs = 0x8C + 0,
       Raster_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       Raster_Err_Table_Missing = 0x8E + 0,
       Raster_Err_Horiz_Header_Missing = 0x8F + 0,
       Raster_Err_Locations_Missing = 0x90 + 0,
       Raster_Err_Name_Table_Missing = 0x91 + 0,
       Raster_Err_CMap_Table_Missing = 0x92 + 0,
       Raster_Err_Hmtx_Table_Missing = 0x93 + 0,
       Raster_Err_Post_Table_Missing = 0x94 + 0,
       Raster_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       Raster_Err_Invalid_CharMap_Format = 0x96 + 0,
       Raster_Err_Invalid_PPem = 0x97 + 0,
       Raster_Err_Invalid_Vert_Metrics = 0x98 + 0,
       Raster_Err_Could_Not_Find_Context = 0x99 + 0,
       Raster_Err_Invalid_Post_Table_Format = 0x9A + 0,
       Raster_Err_Invalid_Post_Table = 0x9B + 0,
       Raster_Err_Syntax_Error = 0xA0 + 0,
       Raster_Err_Stack_Underflow = 0xA1 + 0,
       Raster_Err_Ignore = 0xA2 + 0,
       Raster_Err_Missing_Startfont_Field = 0xB0 + 0,
       Raster_Err_Missing_Font_Field = 0xB1 + 0,
       Raster_Err_Missing_Size_Field = 0xB2 + 0,
       Raster_Err_Missing_Chars_Field = 0xB3 + 0,
       Raster_Err_Missing_Startchar_Field = 0xB4 + 0,
       Raster_Err_Missing_Encoding_Field = 0xB5 + 0,
       Raster_Err_Missing_Bbx_Field = 0xB6 + 0,
       Raster_Err_Bbx_Too_Big = 0xB7 + 0,
       Raster_Err_Corrupted_Font_Header = 0xB8 + 0,
       Raster_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       Raster_Err_Max };

enum {
       SFNT_Err_Ok = 0x00,
       SFNT_Err_Cannot_Open_Resource = 0x01 + 0,
       SFNT_Err_Unknown_File_Format = 0x02 + 0,
       SFNT_Err_Invalid_File_Format = 0x03 + 0,
       SFNT_Err_Invalid_Version = 0x04 + 0,
       SFNT_Err_Lower_Module_Version = 0x05 + 0,
       SFNT_Err_Invalid_Argument = 0x06 + 0,
       SFNT_Err_Unimplemented_Feature = 0x07 + 0,
       SFNT_Err_Invalid_Table = 0x08 + 0,
       SFNT_Err_Invalid_Offset = 0x09 + 0,
       SFNT_Err_Array_Too_Large = 0x0A + 0,
       SFNT_Err_Invalid_Glyph_Index = 0x10 + 0,
       SFNT_Err_Invalid_Character_Code = 0x11 + 0,
       SFNT_Err_Invalid_Glyph_Format = 0x12 + 0,
       SFNT_Err_Cannot_Render_Glyph = 0x13 + 0,
       SFNT_Err_Invalid_Outline = 0x14 + 0,
       SFNT_Err_Invalid_Composite = 0x15 + 0,
       SFNT_Err_Too_Many_Hints = 0x16 + 0,
       SFNT_Err_Invalid_Pixel_Size = 0x17 + 0,
       SFNT_Err_Invalid_Handle = 0x20 + 0,
       SFNT_Err_Invalid_Library_Handle = 0x21 + 0,
       SFNT_Err_Invalid_Driver_Handle = 0x22 + 0,
       SFNT_Err_Invalid_Face_Handle = 0x23 + 0,
       SFNT_Err_Invalid_Size_Handle = 0x24 + 0,
       SFNT_Err_Invalid_Slot_Handle = 0x25 + 0,
       SFNT_Err_Invalid_CharMap_Handle = 0x26 + 0,
       SFNT_Err_Invalid_Cache_Handle = 0x27 + 0,
       SFNT_Err_Invalid_Stream_Handle = 0x28 + 0,
       SFNT_Err_Too_Many_Drivers = 0x30 + 0,
       SFNT_Err_Too_Many_Extensions = 0x31 + 0,
       SFNT_Err_Out_Of_Memory = 0x40 + 0,
       SFNT_Err_Unlisted_Object = 0x41 + 0,
       SFNT_Err_Cannot_Open_Stream = 0x51 + 0,
       SFNT_Err_Invalid_Stream_Seek = 0x52 + 0,
       SFNT_Err_Invalid_Stream_Skip = 0x53 + 0,
       SFNT_Err_Invalid_Stream_Read = 0x54 + 0,
       SFNT_Err_Invalid_Stream_Operation = 0x55 + 0,
       SFNT_Err_Invalid_Frame_Operation = 0x56 + 0,
       SFNT_Err_Nested_Frame_Access = 0x57 + 0,
       SFNT_Err_Invalid_Frame_Read = 0x58 + 0,
       SFNT_Err_Raster_Uninitialized = 0x60 + 0,
       SFNT_Err_Raster_Corrupted = 0x61 + 0,
       SFNT_Err_Raster_Overflow = 0x62 + 0,
       SFNT_Err_Raster_Negative_Height = 0x63 + 0,
       SFNT_Err_Too_Many_Caches = 0x70 + 0,
       SFNT_Err_Invalid_Opcode = 0x80 + 0,
       SFNT_Err_Too_Few_Arguments = 0x81 + 0,
       SFNT_Err_Stack_Overflow = 0x82 + 0,
       SFNT_Err_Code_Overflow = 0x83 + 0,
       SFNT_Err_Bad_Argument = 0x84 + 0,
       SFNT_Err_Divide_By_Zero = 0x85 + 0,
       SFNT_Err_Invalid_Reference = 0x86 + 0,
       SFNT_Err_Debug_OpCode = 0x87 + 0,
       SFNT_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       SFNT_Err_Nested_DEFS = 0x89 + 0,
       SFNT_Err_Invalid_CodeRange = 0x8A + 0,
       SFNT_Err_Execution_Too_Long = 0x8B + 0,
       SFNT_Err_Too_Many_Function_Defs = 0x8C + 0,
       SFNT_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       SFNT_Err_Table_Missing = 0x8E + 0,
       SFNT_Err_Horiz_Header_Missing = 0x8F + 0,
       SFNT_Err_Locations_Missing = 0x90 + 0,
       SFNT_Err_Name_Table_Missing = 0x91 + 0,
       SFNT_Err_CMap_Table_Missing = 0x92 + 0,
       SFNT_Err_Hmtx_Table_Missing = 0x93 + 0,
       SFNT_Err_Post_Table_Missing = 0x94 + 0,
       SFNT_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       SFNT_Err_Invalid_CharMap_Format = 0x96 + 0,
       SFNT_Err_Invalid_PPem = 0x97 + 0,
       SFNT_Err_Invalid_Vert_Metrics = 0x98 + 0,
       SFNT_Err_Could_Not_Find_Context = 0x99 + 0,
       SFNT_Err_Invalid_Post_Table_Format = 0x9A + 0,
       SFNT_Err_Invalid_Post_Table = 0x9B + 0,
       SFNT_Err_Syntax_Error = 0xA0 + 0,
       SFNT_Err_Stack_Underflow = 0xA1 + 0,
       SFNT_Err_Ignore = 0xA2 + 0,
       SFNT_Err_Missing_Startfont_Field = 0xB0 + 0,
       SFNT_Err_Missing_Font_Field = 0xB1 + 0,
       SFNT_Err_Missing_Size_Field = 0xB2 + 0,
       SFNT_Err_Missing_Chars_Field = 0xB3 + 0,
       SFNT_Err_Missing_Startchar_Field = 0xB4 + 0,
       SFNT_Err_Missing_Encoding_Field = 0xB5 + 0,
       SFNT_Err_Missing_Bbx_Field = 0xB6 + 0,
       SFNT_Err_Bbx_Too_Big = 0xB7 + 0,
       SFNT_Err_Corrupted_Font_Header = 0xB8 + 0,
       SFNT_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       SFNT_Err_Max };

enum {
        T1_Err_Ok = 0x00,
        T1_Err_Unknown_File_Format = 0x02 + 0,
	T1_Err_Invalid_File_Format = 0x03 + 0,
        T1_Err_Invalid_Argument = 0x06 + 0,
	T1_Err_Unimplemented_Feature = 0x07 + 0,
        T1_Err_Invalid_Composite = 0x15 + 0,
        T1_Err_Invalid_Pixel_Size = 0x17 + 0,
        T1_Err_Out_Of_Memory = 0x40 + 0,
	T1_Err_Invalid_Stream_Operation = 0x55 + 0,
        T1_Err_Bad_Argument = 0x84 + 0,
        T1_Err_Table_Missing = 0x8E + 0,
        T1_Err_Ignore = 0xA2 + 0,
        T1_Err_Max };

enum {
        T42_Err_Ok = 0x00,
        T42_Err_Unknown_File_Format = 0x02 + 0,
	T42_Err_Invalid_File_Format = 0x03 + 0,
        T42_Err_Invalid_Argument = 0x06 + 0,
	T42_Err_Unimplemented_Feature = 0x07 + 0,
        T42_Err_Invalid_Composite = 0x15 + 0,
        T42_Err_Invalid_Pixel_Size = 0x17 + 0,
        T42_Err_Out_Of_Memory = 0x40 + 0,
	T42_Err_Invalid_Stream_Operation = 0x55 + 0,
        T42_Err_Bad_Argument = 0x84 + 0,
        T42_Err_Table_Missing = 0x8E + 0,
        T42_Err_Max };

enum {
       TT_Err_Ok = 0x00,
       TT_Err_Cannot_Open_Resource = 0x01 + 0,
       TT_Err_Unknown_File_Format = 0x02 + 0,
       TT_Err_Invalid_File_Format = 0x03 + 0,
       TT_Err_Invalid_Version = 0x04 + 0,
       TT_Err_Lower_Module_Version = 0x05 + 0,
       TT_Err_Invalid_Argument = 0x06 + 0,
       TT_Err_Unimplemented_Feature = 0x07 + 0,
       TT_Err_Invalid_Table = 0x08 + 0,
       TT_Err_Invalid_Offset = 0x09 + 0,
       TT_Err_Array_Too_Large = 0x0A + 0,
       TT_Err_Invalid_Glyph_Index = 0x10 + 0,
       TT_Err_Invalid_Character_Code = 0x11 + 0,
       TT_Err_Invalid_Glyph_Format = 0x12 + 0,
       TT_Err_Cannot_Render_Glyph = 0x13 + 0,
       TT_Err_Invalid_Outline = 0x14 + 0,
       TT_Err_Invalid_Composite = 0x15 + 0,
       TT_Err_Too_Many_Hints = 0x16 + 0,
       TT_Err_Invalid_Pixel_Size = 0x17 + 0,
       TT_Err_Invalid_Handle = 0x20 + 0,
       TT_Err_Invalid_Library_Handle = 0x21 + 0,
       TT_Err_Invalid_Driver_Handle = 0x22 + 0,
       TT_Err_Invalid_Face_Handle = 0x23 + 0,
       TT_Err_Invalid_Size_Handle = 0x24 + 0,
       TT_Err_Invalid_Slot_Handle = 0x25 + 0,
       TT_Err_Invalid_CharMap_Handle = 0x26 + 0,
       TT_Err_Invalid_Cache_Handle = 0x27 + 0,
       TT_Err_Invalid_Stream_Handle = 0x28 + 0,
       TT_Err_Too_Many_Drivers = 0x30 + 0,
       TT_Err_Too_Many_Extensions = 0x31 + 0,
       TT_Err_Out_Of_Memory = 0x40 + 0,
       TT_Err_Unlisted_Object = 0x41 + 0,
       TT_Err_Cannot_Open_Stream = 0x51 + 0,
       TT_Err_Invalid_Stream_Seek = 0x52 + 0,
       TT_Err_Invalid_Stream_Skip = 0x53 + 0,
       TT_Err_Invalid_Stream_Read = 0x54 + 0,
       TT_Err_Invalid_Stream_Operation = 0x55 + 0,
       TT_Err_Invalid_Frame_Operation = 0x56 + 0,
       TT_Err_Nested_Frame_Access = 0x57 + 0,
       TT_Err_Invalid_Frame_Read = 0x58 + 0,
       TT_Err_Raster_Uninitialized = 0x60 + 0,
       TT_Err_Raster_Corrupted = 0x61 + 0,
       TT_Err_Raster_Overflow = 0x62 + 0,
       TT_Err_Raster_Negative_Height = 0x63 + 0,
       TT_Err_Too_Many_Caches = 0x70 + 0,
       TT_Err_Invalid_Opcode = 0x80 + 0,
       TT_Err_Too_Few_Arguments = 0x81 + 0,
       TT_Err_Stack_Overflow = 0x82 + 0,
       TT_Err_Code_Overflow = 0x83 + 0,
       TT_Err_Bad_Argument = 0x84 + 0,
       TT_Err_Divide_By_Zero = 0x85 + 0,
       TT_Err_Invalid_Reference = 0x86 + 0,
       TT_Err_Debug_OpCode = 0x87 + 0,
       TT_Err_ENDF_In_Exec_Stream = 0x88 + 0,
       TT_Err_Nested_DEFS = 0x89 + 0,
       TT_Err_Invalid_CodeRange = 0x8A + 0,
       TT_Err_Execution_Too_Long = 0x8B + 0,
       TT_Err_Too_Many_Function_Defs = 0x8C + 0,
       TT_Err_Too_Many_Instruction_Defs = 0x8D + 0,
       TT_Err_Table_Missing = 0x8E + 0,
       TT_Err_Horiz_Header_Missing = 0x8F + 0,
       TT_Err_Locations_Missing = 0x90 + 0,
       TT_Err_Name_Table_Missing = 0x91 + 0,
       TT_Err_CMap_Table_Missing = 0x92 + 0,
       TT_Err_Hmtx_Table_Missing = 0x93 + 0,
       TT_Err_Post_Table_Missing = 0x94 + 0,
       TT_Err_Invalid_Horiz_Metrics = 0x95 + 0,
       TT_Err_Invalid_CharMap_Format = 0x96 + 0,
       TT_Err_Invalid_PPem = 0x97 + 0,
       TT_Err_Invalid_Vert_Metrics = 0x98 + 0,
       TT_Err_Could_Not_Find_Context = 0x99 + 0,
       TT_Err_Invalid_Post_Table_Format = 0x9A + 0,
       TT_Err_Invalid_Post_Table = 0x9B + 0,
       TT_Err_Syntax_Error = 0xA0 + 0,
       TT_Err_Stack_Underflow = 0xA1 + 0,
       TT_Err_Ignore = 0xA2 + 0,
       TT_Err_Missing_Startfont_Field = 0xB0 + 0,
       TT_Err_Missing_Font_Field = 0xB1 + 0,
       TT_Err_Missing_Size_Field = 0xB2 + 0,
       TT_Err_Missing_Chars_Field = 0xB3 + 0,
       TT_Err_Missing_Startchar_Field = 0xB4 + 0,
       TT_Err_Missing_Encoding_Field = 0xB5 + 0,
       TT_Err_Missing_Bbx_Field = 0xB6 + 0,
       TT_Err_Bbx_Too_Big = 0xB7 + 0,
       TT_Err_Corrupted_Font_Header = 0xB8 + 0,
       TT_Err_Corrupted_Font_Glyphs = 0xB9 + 0,
       TT_Err_Max };
#endif
#endif

/* END */
