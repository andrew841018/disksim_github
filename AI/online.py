import tensorflow as tf
import random
from pandas import DataFrame
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\AI\\model\\"
duration_model = tf.keras.models.load_model(addr+'duration_model.h5')
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\trace(used to build RNN)\\"
data=np.loadtxt(addr+'info(run1_Postmark_2475).txt',delimiter=' ',usecols=range(7))
'''
tmp=np.array([])
for i in range(16):
    tmp=np.append(tmp,0)
'''
tmp=np.zeros((16,7))
start=0
finish=15
c=0
output=""
result=np.array([])
while(finish!=len(data)):
    for i in range(start,finish,1):
        tmp[c]=data[i]
        c+=1
    c=0
    tmp=tmp.reshape(1,16,7)
    if duration_model.predict(tmp)[0][0]==np.max(duration_model.predict(tmp)):
        output=0
    if duration_model.predict(tmp)[0][1]==np.max(duration_model.predict(tmp)):
        output=1
    if duration_model.predict(tmp)[0][2]==np.max(duration_model.predict(tmp)):
        output=2   
    with open('online(run1_Postmark_2475).txt','a') as f:
        f.write(str(data[finish][5])+" "+str(output)+"\n")
    #print(output)
    tmp=tmp.reshape(16,7)
    start+=1
    finish+=1



