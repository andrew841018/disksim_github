import os,sys
from pandas import DataFrame
import numpy as np
import pandas as pd
from keras.utils import np_utils
from tensorflow.keras.optimizers import Adam
from sklearn import preprocessing
from keras.utils.np_utils import * 
import keras_metrics as km
import pandas as pd
import random
import keras
import math
import os,sys
from sklearn.metrics import confusion_matrix,precision_score,accuracy_score,recall_score,f1_score,roc_auc_score,precision_recall_fscore_support,roc_curve,classification_report
from sklearn.metrics import plot_roc_curve,plot_precision_recall_curve,roc_curve
import matplotlib.pyplot as plt
import random
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, LSTM#, CuDNNLSTM
from sklearn.model_selection import train_test_split
import tensorflow as tf
from collections import OrderedDict
addr='C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\all buffer\\'
max1=0
tmp=[]
ignore=[]

for i in open(addr+'(physical)duration.txt','r'):
    block_i=i.split()[0]    
    b=0
    for j in open(addr+'(physical)duration.txt','r'):              
        block_j=j.split()[0]
        if block_i in ignore:
            b=1
            break
        if block_i==block_j:#碰到相同block number,including itself
            tmp.append(j.split()[1])  
    if b==0:                        
        with open(addr+'duration.txt','a') as f:
            f.write(block_i+" "+max(tmp)+"\n")
        tmp.clear() 
        ignore.append(block_i)