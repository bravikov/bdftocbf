/* file codepage: UTF-8
 * 
 * Файл: bdftocbf.cpp
 * Дата создания: 31 января 2012
 * Описание: Программа обрабатывает файлы шрифтов BDF и
             преобразует в компилируемый растровый шрифт (CBF), который
             удобен для вывода на простые маленькие индикаторы.
             Программа принимает данные через стандартный поток.
 * Автор: Бравиков Дмитрий (bravikov@gmail.com)
 * Сборка: make
 * Использование: cat font-example-16.bdf | ./bdftocbf.bin > cbf-example-16.h
 *
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdint.h>
#include "bitoperation.h"

using namespace std;

// Глиф CBF
class CBF_glyph
{
public:
  CBF_glyph();
  ~CBF_glyph() {};
  
  int encoding;
  string name;
  int width;
  vector<uint8_t> bitmap;
};

CBF_glyph::CBF_glyph()
{
  width    = 0;
  encoding = 0;
}

// Шрифт CBF
class cbf_font
{
public:
  cbf_font();
  ~cbf_font() {};
  
  int height;
  vector<CBF_glyph> glyphs;
};

cbf_font::cbf_font()
{
  height   = 0;
}

// Глиф BDF
class bdf_glyph
{
public:
  bdf_glyph();
  ~bdf_glyph() {};
  
  int encoding; // ENCODING integer
  string name;  // STARTCHAR string
  
  // BBX BBw BBh BBxoff0x BByoff0y
  int width;  // BBw
  int height; // BBh
  int xoff;   // BBxoff0x
  int yoff;   // BByoff0y
  
  vector< vector<uint8_t> * > rows; // BITMAP <hex data>
};

bdf_glyph::bdf_glyph()
{
  width    = 0;
  height   = 0;
  xoff     = 0;
  yoff     = 0;
  encoding = 0;
}

// Шрифт BDF
class bdf_font
{
public:
  bdf_font();
  ~bdf_font() {};
  
  int width;
  int height;
  int xoff;
  int yoff;
  vector<bdf_glyph> glyphs;
};

bdf_font::bdf_font()
{
  height = 0;
  width  = 0;
  xoff   = 0;
  yoff   = 0;
}


uint8_t char_to_hex(char c) // c - ASCII-символ
{
  if      (c >= '0' && c <= '9') return (c - 0x30);
  else if (c >= 'A' && c <= 'F') return (c - 0x97);
  
  return 0xFF; // Вернуть код ошибки
}

int main()
{
  vector<bdf_glyph * > glyphs;
  bdf_font font;
  
  
  // Обработка BDF
  
  int glyphs_count = 0;
  
  int nglyphs = -1;

  while(1)
  {
    string keyword;
    
    cin >> keyword;
    
    if (keyword == "ENDFONT") break;

    if (keyword == "FONTBOUNDINGBOX")
    {
      cin >> font.width;
      cin >> font.height;
      cin >> font.xoff;
      cin >> font.yoff;
    }
    
    if (keyword == "CHARS")
    {
      if (nglyphs == -1) cin >> nglyphs;
      else
      {
        cerr << "Повторение nglyphs" << endl;
        return 1;
      }
    }
      
    if (keyword == "STARTCHAR")
    {
      glyphs_count++;
      if (glyphs_count > nglyphs)
      {
        cerr << "Ошибка: (glyphs_count > nglyphs)" << endl;
        return 1;
      }
      glyphs.push_back(new bdf_glyph());
      cin >> glyphs.back()->name;     
    }
    
    if (keyword == "ENCODING")
    {
      cin >> glyphs.back()->encoding;
    }
    
    if (keyword == "BBX")
    {
      cin >> glyphs.back()->width;
      cin >> glyphs.back()->height;
      cin >> glyphs.back()->xoff;
      cin >> glyphs.back()->yoff;
    }
    
    if (keyword == "BITMAP")
    {     
      int byte_count = 1 + (-1 + glyphs.back()->width) / 8;
      
      for (int row = 0; row < glyphs.back()->height; row++)
      {
        glyphs.back()->rows.push_back(new vector<uint8_t>);
        for (int b = 0; b < byte_count; b++)
        {
          uint8_t byte, part1, part2;
          char sym;
          cin >> sym;
          part1 = char_to_hex(sym);
          cin >> sym;
          part2 = char_to_hex(sym);
          if (part1 == 0xFF || part2 == 0xFF)
          {
            cerr << "Ошибка: BITMAP не hex" << endl;
            return 1;
          }
          byte = (part1 << 4) | part2;
          glyphs.back()->rows.back()->push_back(byte);
        }
      }
    }
  }
  
  
  // Преобразование BDF -> CBF
  
  cbf_font new_font;
  
  // Высота шрифта CBF, количество пикселей кратно 8 
  new_font.height = 8 * ((font.height + 7) / 8);
  
  // Координата Y базовой линии шрифта в формате CBF
  int CBF_font_baseline = new_font.height + font.yoff - 1;
  
  for (unsigned int g = 0; g < glyphs.size(); g++)
  {  
    CBF_glyph new_glyph;
    
    new_glyph.width    = glyphs.at(g)->width;
    new_glyph.encoding = glyphs.at(g)->encoding;
    new_glyph.name     = glyphs.at(g)->name;
    
    new_glyph.bitmap.resize(new_font.height / 8 * new_glyph.width);
    
    for (unsigned int i = 0; i < new_glyph.bitmap.size(); i++)
      new_glyph.bitmap.at(i) = 0;
    
    int CBF_glyph_H_above_baseline = glyphs.at(g)->height + glyphs.at(g)->yoff;
    int CBF_glyph_initline = 1 + CBF_font_baseline - CBF_glyph_H_above_baseline;
    
    for (unsigned int r = 0; r < glyphs.at(g)->rows.size(); r++)
    {
      int bdf_row_size = glyphs.at(g)->rows.at(r)->size();
      
      for (int bdf_row_byte = 0; bdf_row_byte < bdf_row_size; bdf_row_byte++)
      {
        uint8_t bdf_byte_value = glyphs.at(g)->rows.at(r)->at(bdf_row_byte);
        
        for (int bdf_row_bit = 0; bdf_row_bit < 8; bdf_row_bit++)
        {
          int cbf_pixel_y = r + CBF_glyph_initline;
          int cbf_pixel_x = (7 - bdf_row_bit) + 8 * bdf_row_byte;
          
          if ( cbf_pixel_x < new_glyph.width )
          {
            int cbf_offset_byte = (cbf_pixel_y / 8) * new_glyph.width;
            int cbf_byte = cbf_pixel_x + cbf_offset_byte;
            int cbf_bit = cbf_pixel_y % 8;
            
            if ( getBit(bdf_byte_value, bdf_row_bit) )
              setBit(new_glyph.bitmap.at(cbf_byte), cbf_bit);
            else
              clrBit(new_glyph.bitmap.at(cbf_byte), cbf_bit);
          }
        }
      }
    }
    
    new_font.glyphs.push_back(new_glyph);
  }
  
  
  // Вывод CBF
  
  cbf_font cbf_font;
  
  cbf_font.height = new_font.height;
  
  unsigned int new_glyphs_count = new_font.glyphs.size();
  
  stringstream ss;
  ss << "FontH" << new_font.height;
  string font_name;
  ss >> font_name;
  
  cout << "#include <stdint.h>" << endl;
  cout << endl;
  cout << "const int " << font_name << "_height = " << new_font.height << ";\n";
  cout << endl;
  
  
  // Сформировать массив позиций глифов. Индекс i соответсвует коду символа
  uint16_t glyphs_position[256];
  for (int i = 0; i < 256; i++) glyphs_position[i] = 0xFFFF;
  uint16_t position = 0;
  for(unsigned int g = 0; g < new_glyphs_count; g++)
  {
    glyphs_position[new_font.glyphs.at(g).encoding] = position;
    position += 1 + new_font.glyphs.at(g).width * (new_font.height / 8);
  }
  
  
  // Вывести массив позиций глифов
  cout << "const uint16_t " << font_name << "_glyphs_position[256] = {";
  for (int i = 0; i < 256; i++)
  {
    cout << glyphs_position[i];
    if ( i < 256 - 1 ) cout << ", ";
  }
  cout << "}"<< endl;
  
  
  // Вывести массив битовой карты
  cout << endl;
  cout << "const uint8_t " << font_name << "_bitmap[] = {" << endl;
  cout << endl;

  for(unsigned int g = 0; g < new_glyphs_count; g++)
  {
    unsigned int bitmap_size = new_font.glyphs.at(g).bitmap.size();
    cout << new_font.glyphs.at(g).width << ", ";
    for (unsigned int b = 0; b < bitmap_size; b++)
    {
      cout << "0x";
      cout << uppercase << setfill('0') << setw(2) << hex;
      cout << int(new_font.glyphs.at(g).bitmap.at(b));
      
      if ( g < new_glyphs_count - 1 || b < bitmap_size - 1 ) cout << ", ";
    }
    
    cout << endl;
      // Распаковка CBF
      /*for (int y = 0; y < new_font.height; y++)
      {
        cout << endl;
        for (int x = 0; x < new_font.glyphs.at(g).width; x++)
        {
          int byte_offset = (y / 8) * new_font.glyphs.at(g).width;
          int byte = x + byte_offset;
          int bit = y % 8;
          if ( getBit(new_font.glyphs.at(g).bitmap.at(byte), bit) )
            cout << "#";
          else
            cout << ".";
        }
      }*/
  }
  
  cout << endl << "}" << endl;
  
  
  cout << endl;
  return 0;
}

