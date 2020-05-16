# mục đích: kiểm tra log trả ra các mẫu trong tập sạch để xem có mẫu nào là virus không
# Yeu cau: Log tra ra tuong ung voi moi file la "Log.log"
# copy mau sach ra file de train, copy mau virus va log ra file de kt
# tham so dau vao: path - duong dan thu muc chua mau tra ve

import os
import hashlib 
from shutil import copyfile
import sys

so_mau_log_da_check = 0
# parent_path = "H:\\DoAnTotNghiep\\DAMH\\source\\Tools\\tes_readlog"
# path = "H:\\DoAnTotNghiep\\DAMH\\source\\Tools\\tes_readlog"
parent_path = sys.argv[1]
path = parent_path

CurrentFolder = ""
name_file = ""
def CheckVirus(filename, name):
    global so_mau_log_da_check
    global path
    global CurrentFolder
    global name_file
    global count_clean
    global count_exception

    so_mau_log_da_check += 1
    try :
        with open(filename, 'r') as f:
            Lines = f.readlines()
            # Get name file, to copy
            for line in Lines:
                # doc tung dong mot
                if "permalink: " in line:
                    arr = line.split('/')
                    name_file = arr[4]
                    break
            # Code sau day copy mau sach chu khong copy virus
            for line in Lines:
                if 'Detection: 0|' in line: # kiem tra neu mau log tra ve la mau sach - copy mau va return True
                    if os.path.isdir(path + '\\mau_sach\\') == False: # neu thu muc chua ton tai thi tao
                        os.mkdir(path + "\\mau_sach\\")
                    # Copy mau sach vua detect bang log tra ve
                    copyfile(CurrentFolder+"\\"+name_file, path + '\\mau_sach\\' + name_file)
                    count_clean += 1
                    return True
    except Exception as e:
        # Neu xay ra exception trong qua trinh check thi khong lay mau do nua
        count_exception += 1
        print(e)
        return False
    return False

count_virus = 0
count_clean = 0
count_exception = 0
def CheckLogVirusTotalSample(path):
    global count_virus
    global count_clean
    global count_exception
    global CurrentFolder
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        if os.path.isdir(Directory + "\\" + filename) == True:
            CurrentFolder = Directory + "\\" + filename
            CheckLogVirusTotalSample(Directory + "\\" + filename) # De qui neu la thu muc
        else:
            if "Log" in filename:
                if CheckVirus(Directory + "\\" + filename, filename) == False:# neu ket qua tra ra la virus hoac file exception thi dem
                    if os.path.isdir(parent_path + '\\mau_virus\\') == False: # neu thu muc chua ton tai thi tao
                        os.mkdir(parent_path + "\\mau_virus\\")
                    # Copy mau chua quet hoac virus vao thu muc mau_virus
                    try:
                        copyfile(CurrentFolder+"\\"+name_file, parent_path + '\\mau_virus\\' + name_file)
                        copyfile(CurrentFolder+"\\Log.log", parent_path + '\\mau_virus\\' + name_file+".log")
                        count_virus += 1
                    except Exception as e:
                        print(e)
                        count_exception += 1
                        continue
            

CheckLogVirusTotalSample(path)
# Ghi ra ket qua
print("so mau chua log virus hoac log chua quet: ", count_virus)
print("so mau log da quet la mau sach: ", count_clean)
print("Tong so mau log da check: ", so_mau_log_da_check)
print("So truong hop bi Exception: ", count_exception)