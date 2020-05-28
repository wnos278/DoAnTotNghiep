# Tool copy những file không có đuôi txt vào trong thư mục
# Áp dụng để copy file virus trong tập mẫu thu được từ virustotal

import os
import hashlib 
from shutil import copyfile
import sys

# parent_path = sys.argv[1]
parent_path = "H:\\crawl and extract\\sample\\fortinet"
path = parent_path

CurrentFolder = ""
name_file = ""
count_virus = 0
count_exception = 0
def GetVirusSample(path):
    global count_virus
    global count_exception
    global CurrentFolder
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        if os.path.isdir(Directory + "\\" + filename) == True:
            CurrentFolder = Directory + "\\" + filename
            GetVirusSample(Directory + "\\" + filename) # De qui neu la thu muc
        else:
            if ".txt" not in filename:
                #neu khong la file log thi copy sang tap mau
                if os.path.isdir(parent_path + '\\mau_virus\\') == False: # neu thu muc chua ton tai thi tao
                        os.mkdir(parent_path + "\\mau_virus\\")
                # Copy mau chua quet hoac virus vao thu muc mau_virus
                try:
                    copyfile(CurrentFolder+"\\"+filename, parent_path + '\\mau_virus\\' + filename)
                    count_virus += 1
                except Exception as e:
                    print(e)
                    count_exception += 1
                    continue
            

GetVirusSample(path)
# Ghi ra ket qua
print("So mau virus da copy: ", count_virus)
print("So truong hop bi Exception: ", count_exception)