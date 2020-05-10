import os
import hashlib
from shutil import copyfile

MD5FullFile= []

file_list_clean = os.listdir('H:\\DoAnTotNghiep\\DAMH\\data_crawl\\sontdc_data_js\\clean_js\\res')
file_list_virus = os.listdir("H:\DoAnTotNghiep\DAMH\data_crawl\mausach_heufs")
DirectoryClean = 'H:\\DoAnTotNghiep\\DAMH\\data_crawl\\sontdc_data_js\\clean_js\\res'
DirectoryVirus = "H:\\DoAnTotNghiep\\DAMH\\data_crawl\\mausach_heufs"
os.mkdir("H:\\DoAnTotNghiep\\DAMH\\data_crawl\\mausach_heufs\\res")
bRet = False
i = 0
for b in range(len(file_list_clean)):
    with open(DirectoryClean+'\\'+file_list_clean[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5_returned = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5_returned == md5:
            bRet = True
            break
    if bRet == True:
        bRet = False
    else:   
        MD5FullFile.append(md5_returned)
        # copyfile(DirectoryClean+'\\'+file_list_clean[i], DirectoryClean + '\\res\\' + file_list_clean[i])
    i+=1   
print(i)
i = 0
for b in range(len(file_list_virus)):
    with open(DirectoryVirus+'\\'+file_list_virus[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5_returned = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5_returned == md5:
            bRet = True
            break
    if bRet == True:
        bRet = False
    else:   
        # MD5FullFile.append(md5_returned)
        copyfile(DirectoryVirus+'\\'+file_list_virus[i], DirectoryVirus + '\\res\\' + file_list_virus[i])
    i+=1   
print(i)
print(len(MD5FullFile))