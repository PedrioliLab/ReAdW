ReAdW
=====

## Description
Convert ThermoFinningan RAW mass spectrometry files to the mzXML format[1].

## Dependencies
ReAdW requires either Xcalibur or MSFileReader to be installed or minimally just the three DLLs: XRawfile2.dll, fileio.dll, fregistry.dll.  
Here's the link to download Thermo's freely available [MSFileReader](http://sjsupport.thermofinnigan.com/public/detail.asp?id=703).

## Binaries
Two binaries are available depending on which dependency is installed on your system:
- The MSFileReader compatible is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.201411.msfilereader.exe)
- The Xcalibur compatible one is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.201411.xcalibur.exe)

## Compilation
ReAdW can be compiled from source using Visual Studio 2010.

## Converting multiple files
Save the following code in a .bat file (e.g. convertall.bat) in a directory with a bunch of raw files you want to convert.

    @echo off
    for %%i in (*.RAW) do ReAdW.exe --mzXML --compress --centroid %%i

Double click on the .bat file to convert each of them using ReAdW.  
You will need to rename your ReAdW binary to ReAdW.exe and place it in the same directory or edit the previous code to reference your specific ReAdW binary.
Remove the "--compress" option if you don't want to zlib compress the peaklists.
Remove the "--centroid" option if you don't want to centroid peaks.

## Latest changes
**2014.1.1, 08/08/2014**
- Add new instrument types, parse synchronous precursor selection (sps) text in filter line, report ion injection times as "injectionTime" attribute in the "scan" element.

## References
[1]: "Pedrioli, P. G. A. et al. A common open representation of mass spectrometry data and its application to proteomics research. Nat Biotech 22, 1459–1466 (2004)."
