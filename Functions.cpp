#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <bits/stdc++.h>

using namespace std;

int string_to_decimal(string str)
{
    int value;
    stringstream(str) >> value;
    return value;
}

string get_the_string(char c)
{
  string s(1, c);
  return s;
}

string dec_to_hex(int x, int fill = 5)
{
  stringstream s;
  s << setfill('0') << setw(fill) << hex << x;
  string temp = s.str();
  temp = temp.substr(temp.length() - fill, fill);
  transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
  return temp;
}

string string_expand(string data, int length, char fillChar, bool back = false)
{
  if (back)
  {
    if (length <= data.length())
    {
      return data.substr(0, length);
    }
    else
    {
      for (int i = length - data.length(); i > 0; i--)
      {
        data += fillChar;
      }
    }
  }
  else
  {
    if (length <= data.length())
    {
      return data.substr(data.length() - length, length);
    }
    else
    {
      for (int i = length - data.length(); i > 0; i--)
      {
        data = fillChar + data;
      }
    }
  }
  return data;
}
bool word_det;
int hex_to_dec(string x)
{
  return stoul(x, nullptr, 16);
}

string string_to_hex(const string &input)
{
  static const char *const lut = "0123456789ABCDEF";
  size_t len = input.length();

  string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i)
  {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}

bool check_white_spaces(char x)
{
  if (x == ' ' || x == '\t')
  {
    return true;
  }
  return false;
}

bool check_comment_line(string line)
{
  if (line[0] == '.')
  {
    return true;
  }
  return false;
}

bool if_all_num(string x)
{
  bool all_num = true;
  int i = 0;
  while (all_num && (i < x.length()))
  {
    all_num &= isdigit(x[i++]);
  }
  return all_num;
}
string det = "M^000000^05+VALB\nM^000007^05+VALC\nM^000011^06-VALB";
void reading_non_white_spaces(string line, int &index, bool &status, string &data, bool readTillEnd = false)
{
  data = "";
  status = true;
  if (readTillEnd)
  {
    data = line.substr(index, line.length() - index);
    if (data == "")
    {
      status = false;
    }
    return;
  }
  while (index < line.length() && !check_white_spaces(line[index]))
  {
    data += line[index];
    index++;
  }

  if (data == "")
  {
    status = false;
  }

  while (index < line.length() && check_white_spaces(line[index]))
  {
    index++;
  }
}

void read_byte(string line, int &index, bool &status, string &data)
{
  data = "";
  status = true;
  if (line[index] == 'C')
  {
    data += line[index++];
    char identifier = line[index++];
    data += identifier;
    while (index < line.length() && line[index] != identifier)
    {
      data += line[index];
      index++;
    }
    data += identifier;
    index++;
  }
  else
  {
    while (index < line.length() && !check_white_spaces(line[index]))
    {
      data += line[index];
      index++;
    }
  }

  if (data == "")
  {
    status = false;
  }

  while (index < line.length() && check_white_spaces(line[index]))
  {
    index++;
  }
}

void write_into_file(ofstream &file, string data, bool newline = true)
{
  if (newline)
  {
    file << data << endl;
  }
  else
  {
    file << data;
  }
}

string getting_real_opcode(string opcode)
{
  if (opcode[0] == '+' || opcode[0] == '@')
  {
    return opcode.substr(1, opcode.length() - 1);
  }
  return opcode;
}

char getting_flag_format(string data)
{
  if (data[0] == '#' || data[0] == '+' || data[0] == '@' || data[0] == '=')
  {
    return data[0];
  }
  return ' ';
}

class EvaluateString
{
public:
  int getResult();
  EvaluateString(string data);

private:
  string storedData;
  int index;
  char peek();
  char get();
  int term();
  int factor();
  int number();
};

EvaluateString::EvaluateString(string data)
{
  storedData = data;
  index = 0;
}

int EvaluateString::getResult()
{
  int result = term();
  while (peek() == '+' || peek() == '-')
  {
    if (get() == '+')
    {
      result += term();
    }
    else
    {
      result -= term();
    }
  }
  return result;
}

int EvaluateString::term()
{
  int result = factor();
  while (peek() == '*' || peek() == '/')
  {
    if (get() == '*')
    {
      result *= factor();
    }
    else
    {
      result /= factor();
    }
  }
  return result;
}

int EvaluateString::factor()
{
  if (peek() >= '0' && peek() <= '9')
  {
    return number();
  }
  else if (peek() == '(')
  {
    get();
    int result = getResult();
    get();
    return result;
  }
  else if (peek() == '-')
  {
    get();
    return -factor();
  }
  return 0;
}

int EvaluateString::number()
{
  int result = get() - '0';
  while (peek() >= '0' && peek() <= '9')
  {
    result = 10 * result + get() - '0';
  }
  return result;
}

char EvaluateString::get()
{
  return storedData[index++];
}

char EvaluateString::peek()
{
  return storedData[index];
}
