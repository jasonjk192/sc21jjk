import os
import csv
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image

def crop_image(imagePath):
	im = Image.open(imagePath)
	width, height = im.size
	left = 300
	top = 65
	right = 600
	bottom = 335
	im1 = im.crop((left, top, right, bottom))
	return im1

technique = ['CMAA (Integrated GPU)', 'CSAA (Dedicated GPU)', 'Default', 'FXAAv3', 'MLAA', 'MSAA', 'SMAA', 'SSAA']
current_directory = os.path.dirname(__file__)
dir = current_directory+'/../Assets/Screenshots/cat'

for t in technique:
	imagePath = dir+'/screenshot ('+t+').png'
	print(imagePath)
	im1 = crop_image(imagePath)
	im1.save(dir+'/Cropped/cropped '+t+'.png')