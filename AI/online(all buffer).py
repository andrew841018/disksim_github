import tensorflow as tf
import random
from pandas import DataFrame
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
duration_model = tf.keras.models.load_model('duration_model.h5')
addr="C:\\Users\\user\\Dropbox\\shared with ubuntu\\disksim_github\\collected data(from disksim)\\trace(for testing)\\"
data=np.loadtxt(addr+'trace(for testing)\\train.txt',delimiter=' ')
index=[]
###先以list形式存入，再轉np array比較快，理由不明
for i in range(len(data)):
    index.append(i)
index=np.array(index)
index=index.reshape(len(data),1)
req=np.c_[index,data]#add index to data
req=np.delete(req,-4,axis=1)#remove 第四行，從最後一行開始算起,axis=1代表刪除行
req=DataFrame(req,columns=['index','time','sector_num','total sector','type'])#transfer data into dataframe
req_np=req.to_numpy()
soon=[]
mean=[]
late=[]
for i in range(25):
    req_np=np.delete(req_np,6961240-i,axis=0)#axis=0代表刪除列，0代表第一列
tmp=cache_model.predict(req_np.reshape(217538,32,5))
t=0
count=1
cache_size=128#kb
#在data page進入cache後，又存取了eviction_time個request
#此時會進行demoting的動作
eviction_time=128*5
req_index=[[-1 for i in range(3)]for k in range(len(tmp))]
cache_req_info=pd.DataFrame(req_index,columns=['index','number of access request','label'])
cache_req_info['index']=cache_req_info['index'].astype(int)
cache_req_info['number of access request']=cache_req_info['number of access request'].astype(int)
cache_req_info['label']=cache_req_info['label'].astype(int)
cache_count=0
i_ndex=0
count=1
y=[]
x=[]
miss=1
hit_count=0
delete=[]
for i in tmp:
    if i[0]>=0.7 and cache_size>0:#cached
        t=count*32-1#index of current request
        cache_req_info['index'][cache_count]=t
        label_rate=duration_model.predict(req_np[t].reshape(1,1,5))
        Max=np.max(label_rate)
        label=np.where(label_rate[0]==Max)[0][0]
        if label==0:
            soon.append(t)
            cache_req_info['label'][cache_count]=0
            cache_size-=4
        if label==1:
            mean.append(t)
            cache_req_info['label'][cache_count]=1
            cache_size-=4
        if label==2:
            late.append(t)
            cache_req_info['label'][cache_count]=2
            cache_size-=4
        cache_count+=1
        count+=1
    miss_or_hit=(cache_req_info['index']==i_ndex)##acceess req i_ndex,check miss or hit on cache
    for m in range(len(miss_or_hit)):###access req num
        if miss_or_hit[m]==True: ###cache have req i_ndex
            hit_count+=1
            miss=0
        if cache_req_info['index'][m]==-1:
            break
    if miss==1:
        a=req['sector_num']==req['sector_num'][i_ndex]
        b=req['total sector']==req['total sector'][i_ndex]
        c=req['type']==req['type'][i_ndex]
        d=a & b & c
        for m in range(len(d)):
            if d[m]==True and m!=i_ndex and m in cache_req_info['index']:##找出與i_ndex同樣的req，且確實存在於cache中
                hit_count+=1 
                #print(d[m],d)
                #name = input("Please enter your name: ")
                
                break
    miss=1
    if cache_req_info['index'][i_ndex]!=-1:
        req_num=cache_req_info.loc[i_ndex,'index']
        if cache_req_info['label'][i_ndex]==0:
            element=soon[soon.index(req_num)]
            soon.remove(element)
            soon.append(element)
        if cache_req_info['label'][i_ndex]==1:
            element=mean[mean.index(req_num)]
            mean.remove(element)
            mean.append(element)
        if cache_req_info['label'][i_ndex]==2:
            element=late[late.index(req_num)]
            late.remove(element)
            late.append(element)
    i_ndex+=1
    j=0
    while(cache_req_info['index'][j]!=-1):##request j in the cache ?
        b=0
        #print("enter cache?")
        #print(cache_req_info.head())
        cache_req_info['number of access request'][j]+=1
        if cache_req_info['number of access request'][j]==eviction_time:
            ##demoting
            if cache_req_info['label'][j]==0:
                req_num=cache_req_info.loc[j,'index']#抓出label=0的request number
                for k in range(int(len(soon)*0.2)+1):#avoid MRU 20% req from demoting
                    if req_num==soon[len(soon)-1-k]:
                        b=1
                if b==0:        
                    soon.remove(req_num)
                    delete.append(req_num)
                    cache_size+=4                    
                ###evict soon request
            if cache_req_info['label'][j]==1:
                req_num=cache_req_info.loc[j,'index']
                for k in range(int(len(mean)*0.2)+1):#avoid MRU 20% req from demoting
                    if req_num==mean[len(mean)-1-k]:
                        b=1
                if b==0:                      
                    mean.remove(req_num)
                    soon.insert(0,req_num)
                    cache_req_info['number of access request'][j]=1
                    cache_req_info['label'][j]=0
                ###demote mean to soon
            if cache_req_info['label'][j]==2:
                req_num=cache_req_info.loc[j,'index']
                for k in range(int(len(late)*0.2)+1):#avoid MRU 20% req from demoting
                    if req_num==late[len(late)-1-k]:
                        b=1                  
                if b==0:        
                    late.remove(req_num)
                    mean.insert(0,req_num)
                    cache_req_info['number of access request'][j]=1
                    cache_req_info['label'][j]=1
                ###demote late to mean       
        j+=1
    x.append(i_ndex)
    y.append(round(hit_count/i_ndex,2))
x=np.array(x)##access request history
y=np.array(y)##當下request狀況所算出的hit ratio    
ans=round(hit_count/len(cache_req_info),2)
ans=str(ans*100)
print(ans+"%")

plt.figure(dpi=250)
plt.plot(x,y)
plt.title('hit ratio')
plt.xlabel('request count')
plt.legend(['hit count'], loc='lower left')
plt.show() 

    
