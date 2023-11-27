ToBDF.bat:
  Convert .ttf/otf to .bdf

ToUF.bat
  convert .bdf to .uf

ToPng.bat
  Create .png image of .bdf



To generate fonts intended only for time and readout displays:

-e is used to configure encoding table to include/exclude individual glyths or range of glyths.
For digits only chars: Space *-+:,. 0-9 Amp
bdf2ufont.exe -e -1_65535,+32,+42_58,-47,+65,+77,+80,+97,+109,+112 file.bdf 