ToBDF.bat:
  Convert .ttf/otf to .bdf

ToUF.bat
  convert .bdf to .uf

ToPng.bat
  Create .png image of .bdf


To generate fonts intended only for time and readout displays:
Use -e to configure encoding table to include or exclude individual glyths or complete encoding range.

We want only: Space *-+:,. 0-9 Amp
bdf2ufont.exe -e -1_65535,+32,+42_58,-47,+65,+77,+80,+97,+109,+112 file.bdf 