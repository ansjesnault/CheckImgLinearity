# CheckImgLinearity
Utility allowing to load a raw image of a color swatch (color chart) with a given image handler SDK (could be a plugin) and will check the linearity of the image, it will :
 * Open, read and display the image with the given image SDK (by default Qt or OpenImageIO)
 * In addition of the image SDK choice, given an ini file (see in rsc for samples), it will load an associated image mask (WARNING: with same resolution) and compute pixel average by channel for each patch in order to display a final graph
 
If any transformation has been applied (automatic filter/gamma correction...), then the outputted graph (qcustomPlot on right side of the image) will be not linear.  
So this application is allow to validate which image SDK is able to process raw images and convert it keeping linearity.  
A simple interface should by used/implemented in ordre to add any other image SDK (like ImageMagick...).  

This project use C++11 (build under MSVC11 and after) and should build under linux and mac (not yet tested).  

# Brainstorming
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/ChkImgLin_bootstrap_design.JPG)
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/ChkImgLin_bool_result.JPG)

# Results
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/CheckImgLinearityJPG.png)
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/doc/CheckImgLinearityCR2.png)

# TIPS/TRICKS:
Convert a raw image into another format by changing the colorspace and keeping the linearity :  
dcraw -4 -o0 -T /path/to/cr2 /path/to/output.tiff

see also Microsoft Camera Codec Pack :
 * Win7: http://www.microsoft.com/en-us/download/details.aspx?id=26829 
 * Win8: http://www.microsoft.com/fr-fr/download/confirmation.aspx?id=34871
 * Win8.1:http://www.microsoft.com/fr-fr/download/details.aspx?id=40310
 * see also : http://cr2viewer.com/download.aspx
 
![ScreenShot](https://raw.github.com/ejerome/CheckImgLinearity/master/LICENSE.png)
 
Last doc update :

2015-04-29_0740 PM
