#!/usr/bin/python

import os
import re
import numpy as np
import PIL
from PIL import Image
import sys

#print 'Number of arguments:', len(sys.argv), 'arguments.'
#print 'Argument List:', str(sys.argv)

filename = sys.argv[1]
print "Welcome to the matrix, processing: ",filename

def rgb2gray(rgb):
  r, g, b = rgb[0], rgb[1], rgb[2]
  #gray = 0.2989 * r + 0.5870 * g + 0.1140 * b
  gray = np.mean(rgb,-1)
  return gray

def convertToH(filename):
  img = PIL.Image.open(filename)
  rgb_im = img.convert('RGB')
  print "img size:", rgb_im.size
  w,h = rgb_im.size
  print "band size:", w*8
  pixelist = list(rgb_im.getdata())
  print "data length:", len(pixelist)
  
  pixels = rgb_im.load()
  

  # put pixels in big array, each byte is a 8vertical bar
  out = []
  for row in range(0, h/8 + 1):
    for col in range(0,w):
      for sl in range(0,8):
        #k = row*8*w + sl*w + col
        if(row*8+sl<h):
          v = pixels[col,row*8+sl]
          #print "pixel (row/col/subline) ", row, col, sl, int(rgb2gray(v))
          out.append( int(rgb2gray(v)) )
        else:
          out.append( 255 ) # white
  #print "pixel:",p,rgb2gray(p)

  # write out
  fileOut = open("thermalSerialProcessing/data/imagedata.txt",'w')
  #fileOut.write("#include <avr/pgmspace.h>\n")
  #fileOut.write("int W = "+str(w)+";\n")
  #fileOut.write("int H = "+str(h)+";\n")
  #fileOut.write("PROGMEM prog_uint16_t IMGD[] = {\n   ")
  fileOut.write(str(w)+" "+str(h)+"\n");
  for index,o in enumerate(out):
    #str(hex(o))
    fileOut.write(str(255-o)+"\n")
  fileOut.write("\n};")
  fileOut.close()
  print "done: wrote imagedata.h"


convertToH(filename)

