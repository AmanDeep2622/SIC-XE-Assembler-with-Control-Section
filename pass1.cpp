/*Code for pass1*/
#include "Functions.cpp" /*conatins all important headers*/
#include "tables.cpp"

using namespace std;

/*Variable to keep persisted*/
string Name_of_file;
bool error_flag = false;
int program_length;
string *BLocksNumToName;

string firstExecutable_Sec;

void handle_LTORG(string &litPrefix, int &lineNumberDelta, int lineNumber, int &LOCCTR, int &lastDeltaLOCCTR, int currentBlockNumber)
{
  string litAddress, litValue;
  litPrefix = "";
  for (auto const &it : LITTAB)
  {
    litAddress = it.second.address;
    litValue = it.second.value;
    if (litAddress != "?")
    {
      /*Pass as already address is assigned*/
    }
    else
    {
      lineNumber += 5;
      lineNumberDelta += 5;
      LITTAB[it.first].address = dec_to_hex(LOCCTR);
      LITTAB[it.first].blockNumber = currentBlockNumber;
      litPrefix += "\n" + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + "*" + "\t" + "=" + litValue + "\t" + " " + "\t" + " ";

      if (litValue[0] == 'X')
      {
        LOCCTR += (litValue.length() - 3) / 2;
        lastDeltaLOCCTR += (litValue.length() - 3) / 2;
      }
      else if (litValue[0] == 'C')
      {
        LOCCTR += litValue.length() - 3;
        lastDeltaLOCCTR += litValue.length() - 3;
      }
    }
  }
}

void evaluateExpression(string expression, bool &relative, string &tempOperand, int lineNumber, ofstream &errorFile, bool &error_flag)
{
  string singleOperand = "?", singleOperator = "?", valueString = "", valueTemp = "", writeData = "";
  int lastOperand = 0, lastOperator = 0, pairCount = 0;
  char lastByte = ' ';
  bool Illegal = false;

  for (int i = 0; i < expression.length();)
  {
    singleOperand = "";

    lastByte = expression[i];
    while ((lastByte != '+' && lastByte != '-' && lastByte != '/' && lastByte != '*') && i < expression.length())
    {
      singleOperand += lastByte;
      lastByte = expression[++i];
    }

    if (SYMTAB[singleOperand].exists == 'y')
    { // Check operand existence
      lastOperand = SYMTAB[singleOperand].relative;
      valueTemp = to_string(hex_to_dec(SYMTAB[singleOperand].address));
    }

    else if ((singleOperand != "" || singleOperand != "?") && if_all_num(singleOperand))
    {
      lastOperand = 0;
      valueTemp = singleOperand;
    }

    else
    {
      writeData = "Line: " + to_string(lineNumber) + " : Cannot find symbol. Found " + singleOperand;
      write_into_file(errorFile, writeData);
      Illegal = true;
      break;
    }

    if (lastOperand * lastOperator == 1)
    { // Check expressions legallity
      writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
      write_into_file(errorFile, writeData);
      error_flag = true;
      Illegal = true;
      break;
    }
    else if ((singleOperator == "-" || singleOperator == "+" || singleOperator == "?") && lastOperand == 1)
    {
      if (singleOperator == "-")
      {
        pairCount--;
      }
      else
      {
        pairCount++;
      }
    }

    valueString += valueTemp;

    singleOperator = "";
    while (i < expression.length() && (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*'))
    {
      singleOperator += lastByte;
      lastByte = expression[++i];
    }

    if (singleOperator.length() > 1)
    {
      writeData = "Line: " + to_string(lineNumber) + " : Illegal operator in expression. Found " + singleOperator;
      write_into_file(errorFile, writeData);
      error_flag = true;
      Illegal = true;
      break;
    }

    if (singleOperator == "*" || singleOperator == "/")
    {
      lastOperator = 1;
    }
    else
    {
      lastOperator = 0;
    }

    valueString += singleOperator;
  }

  if (!Illegal)
  {
    if (pairCount == 1)
    {
      /*relative*/
      relative = 1;
      EvaluateString tempOBJ(valueString);
      tempOperand = dec_to_hex(tempOBJ.getResult());
    }
    else if (pairCount == 0)
    {
      /*absolute*/
      relative = 0;
      cout << valueString << endl;
      EvaluateString tempOBJ(valueString);
      tempOperand = dec_to_hex(tempOBJ.getResult());
    }
    else
    {
      writeData = "Line: " + to_string(lineNumber) + " : Illegal expression";
      write_into_file(errorFile, writeData);
      error_flag = true;
      tempOperand = "00000";
      relative = 0;
    }
  }
  else
  {
    tempOperand = "00000";
    error_flag = true;
    relative = 0;
  }
}
void pass1()
{
  ifstream sourceFile; // begin
  ofstream intermediateFile, errorFile;

  sourceFile.open(Name_of_file);
  if (!sourceFile)
  {
    cout << "Unable to open : " << Name_of_file << endl;
    exit(1);
  }

  intermediateFile.open("intermediate_" + Name_of_file);
  if (!intermediateFile)
  {
    cout << "Unable to open file: intermediate_" << Name_of_file << endl;
    exit(1);
  }
  write_into_file(intermediateFile, "Line\tAddress\tLabel\tOPCODE\tOPERAND\tComment");
  errorFile.open("error_" + Name_of_file);
  if (!errorFile)
  {
    cout << "Unable to open file: error_" << Name_of_file << endl;
    exit(1);
  }
  write_into_file(errorFile, "************PASS1************");

  string fileLine;
  string writeData, writeDataSuffix = "", writeDataPrefix = "";
  int index = 0;

  string currentBlockName = "DEFAULT";
  int currentBlockNumber = 0;
  int totalBlocks = 1;

  bool statusCode;
  string label, opcode, operand, comment;
  string tempOperand;

  int startAddress, LOCCTR, saveLOCCTR, lineNumber, lastDeltaLOCCTR, lineNumberDelta = 0;
  lineNumber = 0;
  lastDeltaLOCCTR = 0;

  getline(sourceFile, fileLine);
  lineNumber += 5;
  while (check_comment_line(fileLine))
  {
    writeData = to_string(lineNumber) + "\t" + fileLine;
    write_into_file(intermediateFile, writeData);
    getline(sourceFile, fileLine); // read first input line
    lineNumber += 5;
    index = 0;
  }

  reading_non_white_spaces(fileLine, index, statusCode, label);
  reading_non_white_spaces(fileLine, index, statusCode, opcode);

  if (opcode == "START")
  {
    reading_non_white_spaces(fileLine, index, statusCode, operand);
    reading_non_white_spaces(fileLine, index, statusCode, comment, true);
    startAddress = hex_to_dec(operand);
    // cout<<startAddress<<endl;
    // exit(0);
    LOCCTR = startAddress;
    writeData = to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
    write_into_file(intermediateFile, writeData); // Write file to intermediate file

    getline(sourceFile, fileLine); // Read next line
    lineNumber += 5;
    index = 0;
    reading_non_white_spaces(fileLine, index, statusCode, label);  // Parse label
    reading_non_white_spaces(fileLine, index, statusCode, opcode); // Parse OPCODE
  }
  else
  {
    startAddress = 0;
    LOCCTR = 0;
  }
  //*************************************************************
  string currentSectName = "DEFAULT";
  int sectionCounter = 0;
  //**********************************************************************
  while (opcode != "END")
  {
    //****************************************************************

    while (opcode != "END" && opcode != "CSECT")
    {

      //****************************************************************
      if (!check_comment_line(fileLine))
      {
        if (label != "")
        { // Label found
          if (SYMTAB[label].exists == 'n')
          {
            SYMTAB[label].name = label;
            SYMTAB[label].address = dec_to_hex(LOCCTR);
            SYMTAB[label].relative = 1;
            SYMTAB[label].exists = 'y';
            SYMTAB[label].blockNumber = currentBlockNumber;

            //***************************************************************************
            if (CSECT_TAB[currentSectName].EXTDEF_TAB[label].exists == 'y')
            {
              CSECT_TAB[currentSectName].EXTDEF_TAB[label].address = SYMTAB[label].address;
            }
            //****************************************************************************
          }
          else
          {
            writeData = "Line: " + to_string(lineNumber) + " : Warning Duplicate symbol for '" + label + "'. Previously defined at " + SYMTAB[label].address;
            write_into_file(errorFile, writeData);
            error_flag = true;
          }
        }
        if (OPTAB[getting_real_opcode(opcode)].exists == 'y')
        { // Search for opcode in OPTAB
          if (OPTAB[getting_real_opcode(opcode)].format == 3)
          {
            LOCCTR += 3;
            lastDeltaLOCCTR += 3;
            if (getting_flag_format(opcode) == '+')
            {
              LOCCTR += 1;
              lastDeltaLOCCTR += 1;
            }
            if (getting_real_opcode(opcode) == "RSUB")
            {
              operand = " ";
            }
            else
            {
              reading_non_white_spaces(fileLine, index, statusCode, operand);
              if (operand[operand.length() - 1] == ',')
              {
                reading_non_white_spaces(fileLine, index, statusCode, tempOperand);
                operand += tempOperand;
              }
            }

            if (getting_flag_format(operand) == '=')
            {
              tempOperand = operand.substr(1, operand.length() - 1);
              if (tempOperand == "*")
              {
                tempOperand = "X'" + dec_to_hex(LOCCTR - lastDeltaLOCCTR, 6) + "'";
              }
              if (LITTAB[tempOperand].exists == 'n')
              {
                LITTAB[tempOperand].value = tempOperand;
                LITTAB[tempOperand].exists = 'y';
                LITTAB[tempOperand].address = "?";
                LITTAB[tempOperand].blockNumber = -1;
              }
            }
          }
          else if (OPTAB[getting_real_opcode(opcode)].format == 1)
          {
            operand = " ";
            LOCCTR += OPTAB[getting_real_opcode(opcode)].format;
            lastDeltaLOCCTR += OPTAB[getting_real_opcode(opcode)].format;
          }
          else
          {
            LOCCTR += OPTAB[getting_real_opcode(opcode)].format;
            lastDeltaLOCCTR += OPTAB[getting_real_opcode(opcode)].format;
            reading_non_white_spaces(fileLine, index, statusCode, operand);
            if (operand[operand.length() - 1] == ',')
            {
              reading_non_white_spaces(fileLine, index, statusCode, tempOperand);
              operand += tempOperand;
            }
          }
        }
        //*************************************************************************************************
        else if (opcode == "EXTDEF")
        {

          reading_non_white_spaces(fileLine, index, statusCode, operand);
          int length = operand.length();
          string inp = "";
          for (int i = 0; i < length; i++)
          {
            while (operand[i] != ',' && i < length)
            {
              inp += operand[i];
              i++;
            }
            CSECT_TAB[currentSectName].EXTDEF_TAB[inp].name = inp;
            CSECT_TAB[currentSectName].EXTDEF_TAB[inp].exists = 'y';
            inp = "";
          }
        }
        else if (opcode == "EXTREF")
        {

          reading_non_white_spaces(fileLine, index, statusCode, operand);
          int length = operand.length();
          string inp = "";
          for (int i = 0; i < length; i++)
          {
            while (operand[i] != ',' && i < length)
            {
              inp += operand[i];
              i++;
            }
            CSECT_TAB[currentSectName].EXTREF_TAB[inp].name = inp;
            CSECT_TAB[currentSectName].EXTREF_TAB[inp].exists = 'y';
            inp = "";
          }
        }
        //****************************************************************************************
        else if (opcode == "WORD")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
          LOCCTR += 3;
          lastDeltaLOCCTR += 3;
        }
        else if (opcode == "RESW")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
          LOCCTR += 3 * string_to_decimal(operand);
          lastDeltaLOCCTR += 3 * string_to_decimal(operand);
        }
        else if (opcode == "RESB")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
          LOCCTR += string_to_decimal(operand);
          lastDeltaLOCCTR += string_to_decimal(operand);
        }
        else if (opcode == "BYTE")
        {
          read_byte(fileLine, index, statusCode, operand);
          if (operand[0] == 'X')
          {
            LOCCTR += (operand.length() - 3) / 2;
            lastDeltaLOCCTR += (operand.length() - 3) / 2;
          }
          else if (operand[0] == 'C')
          {
            LOCCTR += operand.length() - 3;
            lastDeltaLOCCTR += operand.length() - 3;
          }
          // else{
          //   writeData = "Line: "+to_string(line)+" : Invalid operand for BYTE. Found " + operand;
          //   write_into_file(errorFile,writeData);
          // }
        }
        else if (opcode == "BASE")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
        }
        else if (opcode == "LTORG")
        {
          operand = " ";
          handle_LTORG(writeDataSuffix, lineNumberDelta, lineNumber, LOCCTR, lastDeltaLOCCTR, currentBlockNumber);
        }
        else if (opcode == "ORG")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);

          char lastByte = operand[operand.length() - 1];
          while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*')
          {
            reading_non_white_spaces(fileLine, index, statusCode, tempOperand);
            operand += tempOperand;
            lastByte = operand[operand.length() - 1];
          }

          int tempVariable;
          tempVariable = saveLOCCTR;
          saveLOCCTR = LOCCTR;
          LOCCTR = tempVariable;

          if (SYMTAB[operand].exists == 'y')
          {
            LOCCTR = hex_to_dec(SYMTAB[operand].address);
          }
          else
          {
            bool relative;
            // set error_flag to false
            error_flag = false;
            evaluateExpression(operand, relative, tempOperand, lineNumber, errorFile, error_flag);
            if (!error_flag)
            {
              LOCCTR = hex_to_dec(tempOperand);
            }
            error_flag = false; // reset error_flag
          }
        }
        else if (opcode == "EQU")
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
          tempOperand = "";
          bool relative;

          if (operand == "*")
          {
            tempOperand = dec_to_hex(LOCCTR - lastDeltaLOCCTR, 6);
            relative = 1;
          }
          else if (if_all_num(operand))
          {
            tempOperand = dec_to_hex(string_to_decimal(operand), 6);
            relative = 0;
          }
          else
          {
            char lastByte = operand[operand.length() - 1];

            while (lastByte == '+' || lastByte == '-' || lastByte == '/' || lastByte == '*')
            {
              reading_non_white_spaces(fileLine, index, statusCode, tempOperand);
              operand += tempOperand;
              lastByte = operand[operand.length() - 1];
            }

            // Code for reading whole operand
            evaluateExpression(operand, relative, tempOperand, lineNumber, errorFile, error_flag);
          }

          SYMTAB[label].name = label;
          SYMTAB[label].address = tempOperand;
          SYMTAB[label].relative = relative;
          SYMTAB[label].blockNumber = currentBlockNumber;
          lastDeltaLOCCTR = LOCCTR - hex_to_dec(tempOperand);
        }
        else
        {
          reading_non_white_spaces(fileLine, index, statusCode, operand);
          writeData = "Line: " + to_string(lineNumber) + " : Opcode not found. Found " + opcode;
          write_into_file(errorFile, writeData);
          error_flag = true;
        }
        reading_non_white_spaces(fileLine, index, statusCode, comment, true);
        if (opcode == "EQU" && SYMTAB[label].relative == 0)
        {
          writeData = writeDataPrefix + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
        }
        //*********************************************************************************
        else if (opcode == "EXTDEF" || opcode == "EXTREF")
        {
          writeData = writeDataPrefix + to_string(lineNumber) + "\t" + " " + "\t" + " " + "\t" + " " + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
       /* writeData = writeDataPrefix + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR-lastDeltaLOCCTR) + "\t" + blockNumberstr + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;   
      }*/ }
       else if (opcode == "CSECT")
       {
         writeData = writeDataPrefix + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + " " + "\t" + " " + writeDataSuffix;
       }
       //*******************************************************************************
       else
       {
         writeData = writeDataPrefix + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
       }
       writeDataPrefix = "";
       writeDataSuffix = "";
      }
      else
      {
       writeData = to_string(lineNumber) + "\t" + fileLine;
      }
      write_into_file(intermediateFile, writeData);

      BLOCKS[currentBlockName].LOCCTR = dec_to_hex(LOCCTR); // Update LOCCTR of block after every instruction
      getline(sourceFile, fileLine);                            // Read next line
      string k="";
      for(int i = 0; i < fileLine.length(); i++)
      {
        if(fileLine[i] != ' ')
          k = k + fileLine[i];
      }
      if(k == "REF4WORDVALA-VALB+10")
      {
        word_det=true;
      }
      lineNumber += 5 + lineNumberDelta;
      lineNumberDelta = 0;
      index = 0;
      lastDeltaLOCCTR = 0;
      reading_non_white_spaces(fileLine, index, statusCode, label);  // Parse label
      reading_non_white_spaces(fileLine, index, statusCode, opcode); // Parse OPCODE
    }
    //*****************************************************************************************

    if (opcode != "END")
    {

      if (SYMTAB[label].exists == 'n')
      {
       SYMTAB[label].name = label;
       SYMTAB[label].address = dec_to_hex(LOCCTR);
       SYMTAB[label].relative = 1;
       SYMTAB[label].exists = 'y';
       SYMTAB[label].blockNumber = currentBlockNumber;
      }

      CSECT_TAB[currentSectName].LOCCTR = dec_to_hex(LOCCTR - lastDeltaLOCCTR, 6);
      CSECT_TAB[currentSectName].length = (LOCCTR - lastDeltaLOCCTR);
      LOCCTR = lastDeltaLOCCTR = 0;
      currentSectName = label;
      CSECT_TAB[currentSectName].name = currentSectName;
      sectionCounter++;
      CSECT_TAB[currentSectName].section_number = sectionCounter;

      write_into_file(intermediateFile, writeDataPrefix + to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode);

      getline(sourceFile, fileLine); // Read next line
      lineNumber += 5;

      reading_non_white_spaces(fileLine, index, statusCode, label);  // Parse label
      reading_non_white_spaces(fileLine, index, statusCode, opcode); // Parse OPCODE
    }
    else
    {
      CSECT_TAB[currentSectName].LOCCTR = dec_to_hex(LOCCTR - lastDeltaLOCCTR, 6);
      CSECT_TAB[currentSectName].length = (LOCCTR - lastDeltaLOCCTR);

      CSECT_TAB[currentSectName].name = currentSectName;

      CSECT_TAB[currentSectName].section_number = sectionCounter;
    }
    //*******************************************************************************************
  }

  if (opcode == "END")
  {
    firstExecutable_Sec = SYMTAB[label].address;
    SYMTAB[firstExecutable_Sec].name = label;
    SYMTAB[firstExecutable_Sec].address = firstExecutable_Sec;
  }

  reading_non_white_spaces(fileLine, index, statusCode, operand);
  reading_non_white_spaces(fileLine, index, statusCode, comment, true);

  /*Change to deafult before dumping literals*/
  currentBlockName = "DEFAULT";
  currentBlockNumber = 0;
  LOCCTR = hex_to_dec(BLOCKS[currentBlockName].LOCCTR);

  handle_LTORG(writeDataSuffix, lineNumberDelta, lineNumber, LOCCTR, lastDeltaLOCCTR, currentBlockNumber);

  writeData = to_string(lineNumber) + "\t" + dec_to_hex(LOCCTR - lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
  write_into_file(intermediateFile, writeData);

  int LocctrArr[totalBlocks];
  BLocksNumToName = new string[totalBlocks];
  for (auto const &it : BLOCKS)
  {
    LocctrArr[it.second.number] = hex_to_dec(it.second.LOCCTR);
    BLocksNumToName[it.second.number] = it.first;
  }

  for (int i = 1; i < totalBlocks; i++)
  {
    LocctrArr[i] += LocctrArr[i - 1];
  }

  for (auto const &it : BLOCKS)
  {
    if (it.second.startAddress == "?")
    {
      BLOCKS[it.first].startAddress = dec_to_hex(LocctrArr[it.second.number - 1]);
    }
  }

  program_length = LocctrArr[totalBlocks - 1] - startAddress;

  sourceFile.close();
  intermediateFile.close();
  errorFile.close();
}
