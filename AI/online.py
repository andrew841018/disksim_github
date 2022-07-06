import tensorflow as tf
import random
from pandas import DataFrame
import numpy as np
import pandas as pd
import time
import math
import matplotlib.pyplot as plt
from tensorflow.compat.v1.keras.backend import get_session
start=time.perf_counter()
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\AI\\model\\"
duration_model = tf.keras.models.load_model(addr+'duration_model(run1_Postmark_2475).h5')
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\AI input feature\\generate by current code\\AI\\"
data=np.loadtxt(addr+'info(run1_Postmark_2475).txt',delimiter=' ',usecols=range(7))
data_for_predict=data
data_for_predict=np.delete(data_for_predict,2,axis=1)#delete physical_block_number
shap.initjs()
sum1=0
sum2=0
sum3=0
sum4=0
sum5=0
sum6=0
total=len(shap_values)*len(shap_values[0])*len(shap_values[0][0])
for i in range(len(shap_values)):
    for j in range(len(shap_values[0])):
        for k in range(len(shap_values[0][0])):
            sum1+=shap_values[i][j][k][0]
            sum2+=shap_values[i][j][k][1]
            sum3+=shap_values[i][j][k][2]
            sum4+=shap_values[i][j][k][3]
            sum5+=shap_values[i][j][k][4]
            sum6+=shap_values[i][j][k][5]
avg1=sum1/total
avg2=sum2/total
avg3=sum3/total
avg4=sum4/total
avg5=sum5/total
avg6=sum6/total
''' ***duration part****'''
tmp=np.zeros((16,6))
start=0
finish=15
c=0
output=""
result=np.array([])
content=[]#store write block....data[2]=physical block
while(finish!=len(data)):
    if data[finish][2] in content:
        start+=1
        finish+=1
        continue;
    for i in range(start,finish,1):
        tmp[c]=data_for_predict[i]
        c+=1
    c=0
    tmp=tmp.reshape(1,16,6)
    if duration_model.predict(tmp)[0][0]==np.max(duration_model.predict(tmp)):
        output=0
    if duration_model.predict(tmp)[0][1]==np.max(duration_model.predict(tmp)):
        output=1
    if duration_model.predict(tmp)[0][2]==np.max(duration_model.predict(tmp)):
        output=2   
    #with open('duration(run1_Postmark_2475).txt','a') as f:
        #f.write(str(data[finish][2])+" "+str(output)+"\n")
    content.append(data[finish][2])
    #print(output)
    tmp=tmp.reshape(16,6)
    start+=1
    finish+=1
end=time.perf_counter()
avg=(end-start)/len(data)
print("avg request response time:",abs(avg))
'''*** buffer part***
tmp=np.zeros((16,6))
start=0
finish=15
c=0
output=""
result=np.array([])
while(finish!=len(data)):
    for i in range(start,finish,1):
        tmp[c]=data_for_predict[i]
        c+=1
    c=0
    tmp=tmp.reshape(1,16,6)
    if duration_model.predict(tmp)[0][0]==np.max(duration_model.predict(tmp)):
        output=0
    if duration_model.predict(tmp)[0][1]==np.max(duration_model.predict(tmp)):
        output=1
    with open('online(run1_Postmark_2475)_buffer_with_ignore.txt','a') as f:
        f.write(str(data[finish][2])+" "+str(output)+"\n")
    #print(output)
    tmp=tmp.reshape(16,6)
    start+=1
    finish+=1'''


