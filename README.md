````
_____________________________________________________________________________
   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄▄   ▄                                                 
  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ██▄  ▄  ██▀▄                                           
  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██▄▀ ██ ██ █                                           
_____________________________________________________________________________
                                                                             
CMSXbin - Convert binary to text file

Usage: CMSXbin <inputfile> [options]

Options:
  -o output     Filename of the output file (default: use input filename with .h/.asm extension)
  -t mane       Data table name (default: use input filename)
  -8b           8-bits data size (default)
  -16b          16-bits data size
  -32b          32-bits data size
  -dec          Decimal data format
  -hex          Hexadecimal data format (default)
  -bin          Binary data format
  -c            C language data (default)
  -asm          Assembler language data
  -n X          Number of data per line (default: 16)
                (0: means only one line)
  -skip F S     Skip binary data from 'F' address to a 'S' size
                Value can be hexadecimal (format: 0xFFFF) or decimal
                Can be invoked multiple times to skip several sections
  -ascii        Display ASCII code of each line (only for 8-bits data)
  -ad           Display decimal address offset of the first element of each line
  -ax           Display hexadecimal address offset of the first element of each line
  -h            Display the help
````
