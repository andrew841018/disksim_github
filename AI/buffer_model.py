from pandas import DataFrame
import numpy as np
import pandas as pd
from keras.utils import np_utils
from tensorflow.keras.optimizers import Adam
from sklearn import preprocessing
from keras.utils.np_utils import * 
from pandas import DataFrame
from xgboost import XGBClassifier
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
#for window
addr='C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\'
buffer=np.loadtxt(addr+'with ignore(RNN paper method)\\(physical)buffer_or_not.txt',delimiter=' ')#cached request index,benefit,size,duration

# for mac   
#addr='//Users/mhtseng/Desktop/git/disksim_github/collected data(from disksim)/'
#buffer=np.loadtxt(addr+'with ignore(RNN paper method)/(physical)buffer_or_not.txt',delimiter=' ')
#for window
addr1=addr+'trace(used to build RNN)\\physical\\'
# for mac 
#addr1=addr+'trace(used to build RNN)/physical/'
req=np.loadtxt(addr1+"info(run1_Postmark_2475).txt",delimiter=' ',usecols=range(7))
buffer_label=np.array([])
for i in range(200000):
    buffer_label=np.append(buffer_label,0)
for i in buffer:
    buffer_label[int(i[0])]=int(i[1])#used block number as index, store buffer_or_not into buffer_label
count=0
x=[]##cached request
c=0
y=[]
x_train=[]
y_train=[]
x_test=[]
y_test=[]
for i in req:
    x.append(i)
    y.append(buffer_label[int(i[5])])
x=np.array(x)
y=np.array(y)
x_train=np.array(x_train)
y_train=np.array(y_train)
x_test=np.array(x_test)
y_test=np.array(y_test)
x_train,x_test=train_test_split(x,random_state=777,train_size=0.8)
#y_train=np_utils.to_categorical(y_train,3)
y_train,y_test=train_test_split(y,random_state=777,train_size=0.8)
#########以下不一定正確，要等實際執行才知道要delete幾行
for i in range(6):
    x_train = np.delete(x_train,0, axis = 0)
    y_train = np.delete(y_train,0, axis = 0)
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
x_train=x_train.reshape(9277,16,7)
x_test=x_test.reshape(2319,16,7)
y_test=np_utils.to_categorical(y_test,2)
y_train=np_utils.to_categorical(y_train,2)



########################### build model 
model=XGBClassifier(n_estimators=100, learning_rate= 0.3)
model.fit(x_train,y_train)
model.score(x_train,y_train)
c=0
count=0
for i in range(len(y_test)):    
    if y_test[i]!=0:
        count+=1
        if model.predict(x_test)[i]==1:
            c+=1
        ans=round(c/count,2)
        ans=str(ans*100)
        print(ans+"%")
