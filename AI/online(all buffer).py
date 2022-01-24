import tensorflow as tf
import random
from pandas import DataFrame
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\AI\\model\\physical\\"

duration_model = tf.keras.models.load_model('(all buffer)duration_model.h5')
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\trace(for testing)\\"
data=np.loadtxt(addr+'trace(for testing)\\info(trace_name).txt',delimiter=' ')
tmp=[]
for i in range(len(data)):#initialization
    tmp.append(data[i][31])
while(len(tmp)!=32*len(data)):
    for i in range(len(data)):#一個迴圈是將每批req,打亂一次
        np.random.shuffle(data[i])#打亂順序，讓每個req都能被預測，否則LSTM架構會忽略參考的req            
        while(str(data[i][31]) in tmp):
            np.random.shuffle(data[i])
        tmp.append(data[i][31])#紀錄要被預測的req