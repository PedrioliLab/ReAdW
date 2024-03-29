ReAdW
=====

## Description
Convert ThermoFinningan RAW mass spectrometry files to the mzXML format[1].  
Originally a part of the [Trans-Proteomic Pipeline](http://tools.proteomecenter.org/wiki/index.php?title=Software:TPP)

## Dependencies
ReAdW requires either Xcalibur or MSFileReader to be installed or minimally just the three DLLs: XRawfile2_x64.dll, Fileio_x64.dll, fregistry_x64.dll. 

MSFileReader can be [downloaded from Thermo's GitHub page](https://github.com/thermofisherlsms/MSFileReader); grab the file MSReader_x64.exe.  

Alternatively, MSFileReader can be downloaded by creating a [free account with ThermoFischer Scientific](https://thermo.flexnetoperations.com/control/thmo/login). Once registered you will find the software under "Other Releases > Release Archive".

Note: there is some unknown issue with MSFileReader 3.1 in terms of the DLL being registered/recognized.  The work-around is to first install MSFileReader_3.0_SP3.exe followed by installing MSReader_x64.exe without uninstalling MSFileReader_3.0_SP3.exe.

## Binaries
Two binaries are available depending on which dependency is installed on your system:
- The MSFileReader compatible is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.2023010.msfilereader.exe)
- The Xcalibur compatible one is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.2023010.xcalibur.exe)

Note: the binaries have been tested to work under wine on \*nix like systems.

## Docker
A Dockerized version of ReAdW is available [here](https://github.com/PedrioliLab/docker-readw)

## Compilation
ReAdW can be compiled from source using Visual Studio 2019.

## Usage
For a quick help call ReAdW.exe without any arguments.

```
Usage: ReAdW [options] <raw file path> [<output file>]

 Options

  --centroid, -c: Centroid all scans (MS1 and MS2)
      meaningful only if data was acquired in profile mode;
      default: off
  [Advanced option, default OFF] --precursorFromFilterLine: only
      try to get the precursor MZ value from the Thermo
      "filterline" text; only use this if you have a good reason!
      Otherwise, the program first will try to obtain a more accurate
       mass from the "Monoisotopic M/Z:" "trailer value"
  --nocompress, -n: Do not use zlib for compressing peaks
      default: on
  --verbose, -v:   verbose
  --gzip, -g:   gzip the output file (independent of peak compression)

  output file: (Optional) Filename for output file;
      if not supplied, the output file will be created
      in the same directory as the input file.


Example: convert input.raw file to output.mzXML, centroiding MS1 and MS2 scans

      ReAdW --centroid C:\test\input.raw c:\test\output.mzXML
```

## Converting multiple files
**On a Windows system**  
Save the following code in a .bat file (e.g. convertall.bat) in a directory with a bunch of raw files you want to convert.

    @echo off
    for %%i in (*.RAW) do ReAdW.exe --centroid %%i

Double click on the .bat file to convert each of them using ReAdW.  
You will need to rename your ReAdW binary to ReAdW.exe and place it in the same directory or edit the previous code to reference your specific ReAdW binary.
Add the "--nocompress" option if you don't want to zlib compress the peaklists.
Remove the "--centroid" option if you don't want to centroid peaks.

**On a \*nix system**  
To run ReAdW on a \*nix like system you will first need to install wine.  

Next follow these steps:
- Copy convertRawDir.sh from the scripts folder into your PATH
- Edit convertRawDir.sh to set the value of PATH_TO_READW to point to the folder in which you have isntalled ReAdW
- Make sure that your ReAdW executable is called ReAdW.exe

The script will convert all new RAW files in a folder and place the resulting mzXML files in a separate mzXML folder.  
## Latest changes
**2023.1.0, 2023/11/08**
- Add support for Orbitrap Astral data (accounting for "cv=" text in the filter line).

**2016.1.0, 2016/06/21**
- Remove enumeration of instrument types in code; just pass read instrument type to output
- If there is an error reading the injection time; do not write out that attribute

**2015.1.0, 2015/06/28**
- Add support for ETD+SA.

**2014.1.1, 2014/08/08**
- Add new instrument types, parse synchronous precursor selection (sps) text in filter line, report ion injection times as "injectionTime" attribute in the "scan" element.

## References
[1]: "Pedrioli, P. G. A. et al. A common open representation of mass spectrometry data and its application to proteomics research. Nat Biotech 22, 1459–1466 (2004)."
