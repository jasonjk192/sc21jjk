import sys
import os
import csv
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def scenePlot(path):
	sceneFile=open(path,'r')
	sceneDataFrame = pd.read_csv(sceneFile, header=0, names=['Sample Based Time'])
	sceneDataFrame = sceneDataFrame.div(1000.0)
	sceneMedian = np.empty(sceneDataFrame.size)
	sceneMedian.fill(sceneDataFrame.median()[0])
	sceneAvg = np.empty(sceneDataFrame.size)
	sceneAvg.fill(sceneDataFrame.mean()[0])
	sceneMedianDataFrame = pd.DataFrame(sceneMedian)
	sceneMedianDataFrame.columns = ['Sample Based Median: '+str(round(sceneDataFrame.median()[0],3))+' μs']
	sceneAvgDataFrame = pd.DataFrame(sceneAvg)
	sceneAvgDataFrame.columns = ['Sample Based Average: '+str(round(sceneDataFrame.mean()[0],3))+' μs']
	return sceneDataFrame, sceneMedianDataFrame, sceneAvgDataFrame
	
def shaderPlot(path):
	shaderFile=open(path,'r')
	shaderDataFrame = pd.read_csv(shaderFile, header=0, names=['Screen Space Based Time'])
	shaderDataFrame = shaderDataFrame.div(1000.0)
	shaderMedian = np.empty(shaderDataFrame.size)
	shaderMedian.fill(shaderDataFrame.median()[0])
	shaderAvg = np.empty(shaderDataFrame.size)
	shaderAvg.fill(shaderDataFrame.mean()[0])
	shaderMedianDataFrame = pd.DataFrame(shaderMedian)
	shaderMedianDataFrame.columns = ['Screen Space Based Median: '+str(round(shaderDataFrame.median()[0],3))+' μs']
	shaderAvgDataFrame = pd.DataFrame(shaderAvg)
	shaderAvgDataFrame.columns = ['Screen Space Based Average: '+str(round(shaderDataFrame.mean()[0],3))+' μs']
	return shaderDataFrame, shaderMedianDataFrame, shaderAvgDataFrame
	
def totalPlot(sceneDataFrame, shaderDataFrame):
	sceneArray = sceneDataFrame.to_numpy()
	shaderArray = shaderDataFrame.to_numpy()
	totalArray = []
	for idx in range(len(sceneArray)):
		total = sceneArray[idx] + shaderArray[idx]
		totalArray.append(total)
	totalDataFrame = pd.DataFrame(totalArray, columns = ['Total Time'])
	totalMedian = np.empty(totalDataFrame.size)
	totalMedian.fill(totalDataFrame.median()[0])
	totalAvg = np.empty(totalDataFrame.size)
	totalAvg.fill(totalDataFrame.mean()[0])
	totalMedianDataFrame = pd.DataFrame(totalMedian)
	totalMedianDataFrame.columns = ['Total Median: '+str(round(totalDataFrame.median()[0],3))+' μs']
	totalAvgDataFrame = pd.DataFrame(totalAvg)
	totalAvgDataFrame.columns = ['Total Average: '+str(round(totalDataFrame.mean()[0],3))+' μs']
	return totalDataFrame, totalMedianDataFrame, totalAvgDataFrame
	
def chartPlot(algorithmName):
	current_directory = os.path.dirname(__file__)
	scenePath = current_directory+'/../Assets/CSV/sceneTime ('+algorithmName+').csv'
	shaderPath = current_directory+'/../Assets/CSV/shaderTime ('+algorithmName+').csv'

	sceneDataFrame, sceneMedianDataFrame, sceneAvgDataFrame = scenePlot(scenePath)
	print("Processed: sceneTime")
	shaderDataFrame, shaderMedianDataFrame, shaderAvgDataFrame = shaderPlot(shaderPath)
	print("Processed: shaderTime")
	totalDataFrame, totalMedianDataFrame, totalAvgDataFrame = totalPlot(sceneDataFrame, shaderDataFrame)
	print("Processed: totalTime")

	ax = sceneDataFrame.plot(kind='line', xlabel='frame', ylabel='time (μs)')
	sceneMedianDataFrame.plot(ax=ax,kind='line')
	sceneAvgDataFrame.plot(ax=ax,kind='line')
	plt.savefig(current_directory+'/../Assets/CSV/Plots/'+algorithmName+' sceneTime.png')

	bx = shaderDataFrame.plot(kind='line', xlabel='frame', ylabel='time (μs)')
	shaderMedianDataFrame.plot(ax=bx,kind='line')
	shaderAvgDataFrame.plot(ax=bx,kind='line')
	plt.savefig(current_directory+'/../Assets/CSV/Plots/'+algorithmName+' shaderTime.png')
	
	cx = totalDataFrame.plot(kind='line', xlabel='frame', ylabel='time (μs)')
	totalMedianDataFrame.plot(ax=cx,kind='line')
	totalAvgDataFrame.plot(ax=cx,kind='line')
	plt.savefig(current_directory+'/../Assets/CSV/Plots/'+algorithmName+' totalTime.png')
	plt.close()

aListD = ['CSAA8xD', 'CSAA8xQD', 'CSAA16xD', 'CSAA16xQD', 'DefaultD', 'FXAAD', 'MLAAD', 'MSAA2xD', 'MSAA4xD', 'MSAA8xD', 'MSAA16xD', 'SMAAD', 'SSAA2xD', 'SSAA4xD']
aListI = ['CMAAI', 'DefaultI', 'FXAAI', 'MLAAI', 'MSAA2xI', 'MSAA4xI', 'MSAA8xI', 'MSAA16xI', 'SMAAI', 'SSAA2xI', 'SSAA4xI']

algorithmName = 'DefaultD'
if(len(sys.argv)>1):
	algorithmName = sys.argv[1]
	
if(algorithmName.lower() == 'loopd'):
	for aD in aListD:
		print("Processing: "+aD)
		chartPlot(aD)
elif(algorithmName.lower() == 'loopi'):
	for aI in aListI:
		print("Processing: "+aI)
		chartPlot(aI)
else:
	print("Processing: "+algorithmName)
	chartPlot(algorithmName)




