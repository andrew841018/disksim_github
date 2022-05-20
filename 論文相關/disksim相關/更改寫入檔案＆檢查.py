import os,sys
###原先寫入的順序是由下而上，這個程式會進行轉換，變成由上而下，並檢查
###確認沒問題（將原檔和新寫入的檔案，一行一行比較，當然一個是由上往下讀，
###另一個是由下往上讀）
addr="/Users/andrew/Desktop/"
a=[]
'''
f1=open(addr+"a.txt",'r')
length=len(f1.readlines())
with open(addr+"a.txt") as f:
    data = f.readlines()[0:length]
for i in range(length-1,-1,-1):
    s=str(data[i]).replace("\n","")
    a.append(s) 
    print(s)
    with open(addr+'order.txt','a') as f2:
       f2.write(s+'\n')
'''      
b=0
f1=open(addr+"a.txt",'r')
f1_content=[]
for i in f1.readlines():
    f1_content.append(i)
f2=open(addr+'order.txt','r')
f2_content=[]
for i in f2.readlines():
    f2_content.append(i)
if len(f1_content)==len(f2_content):
    for i in range(len(f1_content)):
        if f1_content[i]==f2_content[len(f1_content)-i-1]:
            b=1#correct
        else:
            b==0#error
    if b==1:    
        print("correct")
    

