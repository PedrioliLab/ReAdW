ReAdW
=====

## Description
Convert ThermoFinningan RAW mass spectrometry files to the mzXML format[1].  
Originally a part of the [Trans-Proteomic Pipeline](http://tools.proteomecenter.org/wiki/index.php?title=Software:TPP)

## Dependencies
ReAdW requires either Xcalibur or MSFileReader to be installed or minimally just the three DLLs: XRawfile2.dll, fileio.dll, fregistry.dll.  
Here's the link to download Thermo's freely available [MSFileReader](http://sjsupport.thermofinnigan.com/public/detail.asp?id=703).

## Binaries
Two binaries are available depending on which dependency is installed on your system:
- The MSFileReader compatible is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.201411.msfilereader.exe)
- The Xcalibur compatible one is [here](https://github.com/PedrioliLab/ReAdW/blob/master/bin/ReAdW.201411.xcalibur.exe)

## Compilation
ReAdW can be compiled from source using Visual Studio 2010.

## Usage
For a quick help call ReAdW.exe without any arguments.

```
ReAdW [options] <raw file path> [<output file>]

 Options
  --mzXML:         mzXML mode (default)

  --centroid, -c: Centroid all scans (MS1 and MS2)
      meaningful only if data was acquired in profile mode;
      default: off
  [Advanced option, default OFF] --precursorFromFilterLine: only
      try to get the precursor MZ value from the Thermo
      "filterline" text; only use this if you have a good reason!
      Otherwise, the program first will try to obtain a more accurate
       mass from the "Monoisotopic M/Z:" "trailer value"
  --compress, -z: Use zlib for compressing peaks
      default: off
  --verbose, -v:   verbose
  --gzip, -g:   gzip the output file (independent of peak compression)

  output file: (Optional) Filename for output file;
      if not supplied, the output file will be created
      in the same directory as the input file.


Example: convert input.raw file to output.mzXML, centroiding MS1 and MS2 scans

      ReAdW --mzXML -c C:\test\input.raw c:\test\output.mzXML
```

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
