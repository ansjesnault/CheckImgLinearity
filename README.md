# CheckImgLinearity
Utility allowing to load a raw image of a color swatch (color chart) with a given image handler SDK (could be a plugin) and will check the linearity of the image, it will :
 * Open, read and display the image with the given image SDK (by default Qt or OpenImageIO)
 * In addition of the image SDK choice, given an ini file (see in rsc for samples), it will load an associated image mask (WARNING: with same resolution) and compute pixel average by channel for each patch in order to display a final graph
 
If any transformation has been applied (automatic filter/gamma correction...), then the outputted graph (qcustomPlot on right side of the image) will be not linear.\n
So this application is allow to validate which image SDK is able to process raw images and convert it keeping linearity.\n
A simple interface should by used/implemented in ordre to add any other image SDK (like ImageMagick...).\n
This project should build under linux and mac (not yet tested).\n

# Brainstorming
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/ChkImgLin_bootstrap_design.JPG)
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/ChkImgLin_bool_result.JPG)

# TIPS/TRICKS:
Convert a raw image into another format by changing the colorspace and keeping the linearity :\n
dcraw -4 -o0 -T /path/to/cr2 /path/to/output.tiff

see also Microsoft Camera Codec Pack :\n
 * Win7: http://www.microsoft.com/en-us/download/details.aspx?id=26829 
 * Win8: http://www.microsoft.com/fr-fr/download/confirmation.aspx?id=34871
 * Win8.1:http://www.microsoft.com/fr-fr/download/details.aspx?id=40310
 * see also : http://cr2viewer.com/download.aspx