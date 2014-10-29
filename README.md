ReAdW
=====

## Description
Convert ThermoFinningan RAW mass spectrometry files to the mzXML format[^mzXML].

## Dependencies
ReAdW requires either Xcalibur or MSFileReader to be installed or minimally just the three DLLs: XRawfile2.dll, fileio.dll, fregistry.dll.
Here's the link to download Thermo's freely available [MSFileReader](http://sjsupport.thermofinnigan.com/public/detail.asp?id=703).

## Compilation
ReAdW can be compiled from source using Visual Studio 2010.

## Latest changes
**2014.1.1, 08/08/2014**
- Add new instrument types, parse synchronous precursor selection (sps) text in filter line, report ion injection times as "injectionTime" attribute in the "scan" element.

## References
[^mzXML]: "Pedrioli, P. G. A. et al. A common open representation of mass spectrometry data and its application to proteomics research. Nat Biotech 22, 1459–1466 (2004)."
