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
addr='C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\'
trace=np.loadtxt(addr+'trace(run1_Postmark_2475).txt',delimiter=' ')
sector_block_number=np.loadtxt(addr+'sector number and physical block number.txt',delimiter=' ')
duration=np.loadtxt(addr+'duration.txt',delimiter=' ')

trace=pd.DataFrame(trace,columns=['time','disk number','sector num','total sector','type','phy block num','dur'])
trace=trace.drop(columns=['disk number'])
sector_block_number=pd.DataFrame(sector_block_number,columns=['sector num','phy block num'])
duration=pd.DataFrame(duration,columns=['phy block num','dur'])
for i in trace['sector num']:
    for j in sector_block_number['sector num']:
        if i==j:
            for k in range(len(trace.loc[trace['sector num']==i]['sector num'])):
                trace_row=trace.loc[trace['sector num']==i]['sector num'].index[k]#相等的sector number所在位置
                for l in range(len(sector_block_number.loc[sector_block_number['sector num']==i]['sector num'])):
                sector_block_num_row=sector_block_number.loc[sector_block_number['sector num']==i]['sector num'].index[l]]
                trace['phy block num'][row]=sector_block_number['phy block num'][]
            

"""
                    s=f_time
                    s+=" "+f_sector_num
                    s+=" "+f_total_sector
                    s+=" "+f_req_type
                    s+=" "
                    s+=f1_phy_block_num
                    s+=" "+k.split()[1]
                    with open('run1_Postmark_2475(with block number and duration).txt','a') as f1:
                        f1.write(s+"\n")
"""            
