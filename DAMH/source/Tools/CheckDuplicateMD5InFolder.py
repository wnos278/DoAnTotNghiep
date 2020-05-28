# Tool check trùng md5 trong cùng một thư mục
import os
import hashlib
from shutil import copyfile
import sys 

path = sys.argv[1]
MD5FullFile= []

file_list = os.listdir(path)
DirectoryClean = path

# tao thu muc loc ra cac file khac nhau ve md5
os.mkdir(DirectoryClean + "\\res")
bRet = False
i = 0
arr_checktrungfile= []
for b in range(len(file_list)):
    with open(DirectoryClean+'\\'+file_list[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5_returned = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5_returned == md5:
            TenFileBiTrung = file_list[i]
            for j in range(len(arr_checktrungfile)):
                if arr_checktrungfile[j][0] == md5_returned:
                    arr_checktrungfile[j].append(TenFileBiTrung)
                    break 
            bRet = True
            break
    if bRet == True:
        bRet = False
    else:   
        # Luu mang chua thong tin cac file bi trung
        mang = arr_checktrungfile.append([md5_returned, file_list[i]])
        # 
        MD5FullFile.append(md5_returned)
        # copy nhung file khac nhau md5 sang day - comment neu chi dem
        copyfile(DirectoryClean+'\\'+file_list[i], DirectoryClean + '\\res\\' + file_list[i])
    i+=1   

# print(len(MD5FullFile))
print(arr_checktrungfile[2])
print(len(arr_checktrungfile[2]))