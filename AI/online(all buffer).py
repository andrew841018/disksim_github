import tensorflow as tf
import random
from pandas import DataFrame
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\AI\\model\\physical\\"

duration_model = tf.keras.models.load_model('(all buffer)duration_model.h5')
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\trace(for testing)\\"
data=np.loadtxt(addr+'trace(for testing)\\info(trace_name).txt',delimiter=' ',usecols=range(7))
tmp=np.zeros(16,5)
start=0
finish=15
c=0
result=np.array([])
while(finish!=len(data)):
    for i in range(start,finish,1):
        tmp[c]=test[i]
        c+=1
    c=0
    tmp=tmp.reshape(1,16,5)
    result=np.append(result,model.predict(tmp))
    print(model.predict(tmp))
    tmp=tmp.reshape(16,5)
    start+=1
    finish+=1