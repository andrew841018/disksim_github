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

duration=np.loadtxt(addr+'All buffer\\(logical)duration.txt',delimiter=' ')#cached request index,benefit,size,duration
addr1=addr+'trace(used to build RNN)\\logical\\'
req=np.loadtxt(addr1+"info(run1_Postmark_2475).txt",delimiter=' ')
duration_label=[]
duration_table=np.array([])
for i in range(1000000):
    duration_table=np.append(duration_table,0)
for i in range(len(req)):
    #store the block number req[i][6] corresponding index
    duration_table[req[i][6]]=i
    
count=0
soon=[]
mean=[]
late=[]
x_train=[]##cached request
x_test=[]
zero=0
one=0
two=0
for i in range(len(duration)):
    if duration[i][1]<40:#duration<write buffer size=40 block
        duration_label.append(0)#class 0=soon label
        zero+=1
        soon.append(duration[i][0])
    if 40<duration[i][1] and duration[i][1]<5*40:
        duration_label.append(1)#class 1=mean label
        one+=1
        mean.append(duration[i][0])
    if 5*40<duration[i][1]: 
        duration_label.append(2)#class 2=late label
        two+=1
        late.append(duration[i][0])
duration_label=np.array(duration_label)

c=0
y_train=[]
for i in req:
    x_train.append(i)
x_train=np.array(x_train).reshape(160,1,5)
y_train=np.array(y_train)
y_train=np_utils.to_categorical(y_train,3)


y_test=[]
test_label=np.loadtxt(addr+'ground true label_test.txt',delimiter=' ')#cached request index,benefit,size,duration
test_req=np.loadtxt(addr+'input_new_test.txt',delimiter=' ')
duration_test=[]
cache_test=[]##testing data的cache index
zero_test=0
one_test=0
two_test=0
for i in range(len(test_label)):
    #data[i][3]=duration
    #data[i][2]=cache size
    if test_label[i][3]<128:#duration<cache_size
        duration_test.append(0)#class 0=soon label
        zero_test+=1
        cache_test.append(test_label[i][0])
    if 128<test_label[i][3] and test_label[i][3]<5*test_label[i][2]:
        duration_test.append(1)#class 1=mean label
        one_test+=1
        cache_test.append(test_label[i][0])
    if 5*128<test_label[i][3]: 
        duration_test.append(2)#class 2=late label
        two_test+=1
        cache_test.append(test_label[i][0])
#### assign request label to y      
count=0  
for i in test_req:
    if count in cache_test:
        x_test.append(i)
        y_test.append(duration_test[cache_test.index(count)])
    count+=1
duration_test=np.array(duration_test)
y_test=np.array(y_test)
x_test=np.array(x_test).reshape(210,1,5)
y_test=np_utils.to_categorical(y_test,3)


'''
########################## LR
weight={0:127,1:184318.142857142857142,2: 1.0717299578059072}
lr=LogisticRegression(class_weight=weight)
lr.fit(x_train,y_train)    
lr.predict(x_test)
'''
########################### build model 
metric=[
        keras.metrics.BinaryAccuracy(name='accuracy'),
        keras.metrics.Precision(name='precision'),
        keras.metrics.Recall(name='recall'),
        keras.metrics.AUC(name='auc')
        ]
model=Sequential()
##128=LSTM output size
model.add(LSTM(128,input_shape=(1,5),activation='relu',return_sequences=True))
model.add(Dropout(0.2))
model.add(LSTM(128,activation='relu',return_sequences=True))
model.add(Dropout(0.2))

#return_sequences=True.....將所有time step output 輸出
#false.....只輸出最後一個time step output
model.add(LSTM(128,activation='relu'))
model.add(Dropout(0.2))
model.add(Dense(3,activation='softmax'))#classify into 1 class

#print(model.summary())

#opt=tf.keras.optimizers.Adam(lr=1e-3,decay=1e-5)
model.compile(optimizer='rmsprop',loss='categorical_crossentropy',metrics=metric)
'''
training data-->training, validation-->calculate accuracy
input_shape format=(batch size,timestep,input dimension)
PS:model.fit當中validation_data等同於evaluate功能，兩者選其一
##############################問題可能出在y_train,y_test的值是0,1,2(似乎只能是100,010,001)......看怎麼解決
ValueError: Shapes (None, 3)--->current y_train.shape and (None, 1)-->default y_train.shape are incompatible
'''
#weight={0:127,1:184318.142857142857142,2: 1.0717299578059072}
history=model.fit(x_train,y_train,epochs=3000,validation_data=(x_test,y_test))
#注意，下面這個檔案會存在spyder當下所在，而非程式位置，可用cd更改位置
'''
plt.figure(dpi=250)#dpi越高，像素越高
ㄅㄨplt.subplot(2,2,1)#兩行兩列的方形中，第一張圖
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
plt.legend()

plt.show() 
model.save('duration_model.h5') 