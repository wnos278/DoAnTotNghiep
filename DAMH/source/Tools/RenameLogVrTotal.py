# Trường hợp sử dụng nhầm Tool "RenameToSHA256" để đổi file Log.log trả về thành SHA của nó
# thì sử dụng Tool này để đổi lại tên
# Yêu cầu: mẫu log cần đổi lại tên

import os
import hashlib 
from shutil import copyfile
import sys 

path = sys.argv[1]
# os.mkdir("H:\\DoAnTotNghiep\\DAMH\\source\\Tools\\mau_sach_log_doi_ten")
CurrentFolder = ""
so_mau_log_da_check = 0

def CheckVirus(filename):
    global so_mau_log_da_check
    global path
    global CurrentFolder
    so_mau_log_da_check += 1
    try :
        with open(filename, 'r') as f:
            Lines = f.readlines()
            for line in Lines:
                if "permalink: " in line:
                    f.close()
                    os.rename(filename, CurrentFolder + "\\Log.log")
                    return True
    except Exception as e:
        # Neu xay ra exception trong qua trinh check thi khong lay mau do nua
        print(e)
        return False

count_virus = 0
count_clean = 0
def RenameLogVrTotal(path):
    global count_virus
    global count_clean
    global CurrentFolder
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        if os.path.isdir(Directory + "\\" + filename) == True:
            CurrentFolder = Directory + "\\" + filename
            RenameLogVrTotal(Directory + "\\" + filename) # De qui neu la thu muc
        else:
            if CheckVirus(Directory + "\\" + filename) == False:# neu ket qua tra ra la virus hoac file exception thi dem
                count_virus += 1
            else:
                count_clean += 1
                
RenameLogVrTotal(path)
# Ghi ra ket qua
print("So mau khong phai la log: ", count_virus)
print("So mau Log da doi ten thanh cong: ", count_clean)
print("Tong so mau: ", so_mau_log_da_check)