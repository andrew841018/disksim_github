branch:
rewrite-disksim_2021-base-on-simulation-info:
1. 將原本的disksim改寫成RC-RNN那篇論文的架構
2. 在這個branch裡面的disksim_2021會收集每個block的對應benefit value--->【sector num-physical block num-benefit-sector count.txt】
3. 此外有幾個特殊的txt檔以下分別介紹各自用處:
    (1) wb.txt: use for record current reuqest count
    (2)  skip.txt: 因為在改寫過程中發現有時候抓不到某些應該存在的benefit value，因此這個txt檔用來偵測那些block的benefit不存在於【sector num-physical block num-benefit-sector count.txt】裡面
    (3) hit_count.txt: 用來檢查當下的hit ratio狀況
    (4)info(trace name).txt (for example: info(run1_Postmark_2475).txt):用於抓取machine learning所需要的特徵，之後這個txt會送入AI做訓練(記住，每個trace都有一個info檔)
build-model-after-simulation:
1.這個主要是用來蒐集資料，目前我用於做simulation


