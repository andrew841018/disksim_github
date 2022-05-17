from pandas import DataFrame
import numpy as np
import pandas as pd
from keras.utils import np_utils
from tensorflow.keras.optimizers import Adam
from sklearn import preprocessing
from keras.utils.np_utils import * 
from pandas import DataFrame
import random
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing  import StandardScaler
import math
import keras
from xgboost import XGBClassifier
from sklearn.metrics import confusion_matrix,precision_score,accuracy_score,recall_score,f1_score,roc_auc_score,precision_recall_fscore_support,roc_curve,classification_report
import os,sys
import random
import matplotlib.pyplot as plt
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, LSTM#, CuDNNLSTM
from sklearn.model_selection import train_test_split
import tensorflow as tf
from collections import OrderedDict
#testing data的格式要和training data一樣，每一行也都要同樣意義
addr='C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\'
duration=np.loadtxt(addr+'duration_value\\duration(iozone).txt',delimiter=' ')#cached request index,benefit,size,duration
addr1=addr+'AI input feature\\'
req=np.loadtxt(addr1+"info(iozone2).txt",delimiter=' ',usecols=range(7))
req_for_predict=req
req_for_predict=np.delete(req_for_predict,2,axis=1)#delete physical_block_number
duration_label=np.zeros(shape=(200000,1)) 
count=0
x=[]##cached request
special_use=[]
for i in range(len(duration)):
    if duration[i][1]<2560 and duration[i][0] not in special_use:#duration<write buffer size=40 block
        duration_label[int(duration[i][0])]=0#class 0=soon label
    if 2560<duration[i][1] and duration[i][1]<5*2560 and duration[i][0] not in special_use:
        duration_label[int(duration[i][0])]=1#class 1=mean label
        special_use.append(duration[i][0])
    if 5*2560<duration[i][1] and duration[i][0] not in special_use: 
        duration_label[int(duration[i][0])]=2#class 2=late label
        special_use.append(duration[i][0])
        
duration_label=np.array(duration_label)
c=0
y=[]
x_train=[]
y_train=[]
x_test=[]
y_test=[]
for i in req_for_predict:
    x.append(i)
for i in req:
    y.append(duration_label[int(i[2])])
x=np.array(x)
y=np.array(y)
x_train=np.array(x_train)
y_train=np.array(y_train)
x_test=np.array(x_test)
y_test=np.array(y_test)
x_train,x_test=train_test_split(x,random_state=777,train_size=0.8)
#y_train=np_utils.to_categorical(y_train,3)
y_train,y_test=train_test_split(y,random_state=777,train_size=0.8)

'''*** 使用XGBoost來預測***
model=XGBClassifier(n_estimators=100, learning_rate= 0.3)
model.fit(x_train,y_train)
model.score(x_train,y_train)
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\trace(for testing)\\"
data=np.loadtxt(addr1+"info(run1_Postmark_2475).txt",delimiter=' ',usecols=range(7))
data=np.delete(data,2,axis=1)
predict=model.predict(data)
req_count=0
for i in predict:
    with open('online(duration).txt','a') as f:
        f.write(str(req[req_count][2])+" "+str(i)+"\n")
    req_count+=1
'''


for i in range(12):
    x_train = np.delete(x_train,0, axis = 0)
    y_train = np.delete(y_train,0, axis = 0)
for i in range(8):
    x_test=np.delete(x_test,0,axis=0)
    y_test=np.delete(y_test,0, axis = 0)
index=0
for i in range(len(x_train)):
    if (c+1) % 16!=0:
        y_train=np.delete(y_train,index,axis=0)
    else:
        index+=1#確定第31,63,95...比答案不會被刪除
    c+=1
c=0
index=0
for i in range(len(x_test)):
    if (c+1) % 16!=0:
        y_test=np.delete(y_test,index,axis=0)
    else:
        index+=1#確定第31,63,95...比答案不會被刪除
    c+=1
x_train=x_train.reshape(4829,16,6)
x_test=x_test.reshape(1207,16,6)
y_test=np_utils.to_categorical(y_test,3)
y_train=np_utils.to_categorical(y_train,3)
zero=0
one=0
two=0
for i in y_test:
    if int(i[0])==1:
        zero+=1
    if int(i[1])==1:
        one+=1
    if int(i[2])==1:
        two+=1
zero_=0
one_=0
two_=0
for i in y_train:
    if int(i[0])==1:
        zero_+=1
    if int(i[1])==1:
        one_+=1
    if int(i[2])==1:
        two_+=1


########################### build model 
metric=[
        keras.metrics.BinaryAccuracy(name='accuracy'),
        keras.metrics.Precision(name='precision'),
        keras.metrics.Recall(name='recall'),
        keras.metrics.AUC(name='auc')
        ]
model=Sequential()
##128=LSTM output size
model.add(LSTM(128,input_shape=(16,6),activation='sigmoid',return_sequences=True))
model.add(Dropout(0.2))
model.add(LSTM(128,activation='sigmoid',return_sequences=True))
model.add(Dropout(0.2))

#return_sequences=True.....將所有time step output 輸出
#false.....只輸出最後一個time step output
model.add(LSTM(128,activation='sigmoid'))
model.add(Dropout(0.2))
model.add(Dense(3,activation='softmax'))#classify into 1 class

#print(model.summary())
#opt=tf.keras.optimizers.Adam(lr=1e-3,decay=1e-5)
model.compile(optimizer='rmsprop',loss='categorical_crossentropy',metrics=metric)
'''
training data-->training, validation-->calculate accuracy
input_shape format=(batch size,timestep,input dimension)
PS:model.fit當中validation_data等同於evaluate功能，兩者選其一
'''
weight={0:1.4956629491945477,1:4.733333333333333,2: 8.324137931034484}
history=model.fit(x_train,y_train,epochs=600,validation_data=(x_test,y_test),class_weight=weight)
#注意，下面這個檔案會存在spyder當下所在，而非程式位置，可用cd更改位置
'''
plt.figure(dpi=250)#dpi越高，像素越高
plt.subplot(2,2,1)#兩行兩列的方形中，第一張圖
plt.title('PR curve')
plt.plot(history.history['recall'],history.history['precision'])#第一個參數表示x軸，第二個參數表示y軸
plt.xlabel('recall')
plt.ylabel('precision')
plt.legend()#表示那些線條代表那些數值
plt.subplot(2,2,2)#兩行兩列的方形中，第二張圖
'''

plt.figure(dpi=250)
plt.title('model accuracy')
plt.plot(history.history['accuracy'])
plt.xlabel('Epoch')
plt.ylabel('accuracy')
plt.show() 
model.save('duration_model.h5') 