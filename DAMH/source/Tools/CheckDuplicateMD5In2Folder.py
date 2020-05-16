# Tool check trùng md5 trong 2 tập mẫu virus và sạch, hoặc 2 tập mẫu sạch hoặc 2 tập mẫu virus
# Có thêm chức năng tách mẫu trùng ra và chỉ lấy một md5
# inputfile va outputfile la 2 folder dau vao, input dung lam thu muc goc va output la thu muc can so sanh
# ket qua copy nhung file trong folder 2 ko co trong folder 1

import os
import hashlib
from shutil import copyfile
import sys 

# ######################Lay gia tri thu tham so dau vao#################
try:
      opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
    except getopt.GetoptError:
        print 'test.py -i <inputfile> -o <outputfile>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'test.py -i <inputfile> -o <outputfile>'
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg

files_folder1 = os.listdir(inputfile)
files_folder2 = os.listdir(outputfile)
Directory1 = inputfile
Directory2 = outputfile
os.mkdir(outputfile + "\\res")
#########################################################################

MD5FullFile= []
bRet = False
i = 0
for b in range(len(files_folder1)):
    with open(Directory1+'\\'+files_folder1[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5Return = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5Return == md5:
            bRet = True
            break
    if bRet == True:
        bRet = False
    else:   
        MD5FullFile.append(md5Return)
        # copyfile(Directory1+'\\'+files_folder1[i], Directory1 + '\\res\\' + files_folder1[i])
    i+=1   
print("So mau da quet trong Folder1 la: ", i)
i = 0
for b in range(len(files_folder2)):
    with open(Directory2+'\\'+files_folder2[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5Return = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5Return == md5:
            bRet = True
            break
    if bRet == True:
        bRet = False
    else:   
        MD5FullFile.append(md5Return)
        copyfile(Directory2+'\\'+files_folder2[i], Directory2 + '\\res\\' + files_folder2[i])
    i+=1   

print("So mau da quet trong Folder2 la: ", i)
print("Tong so MD5 khac nhau trong 2 Folder: ", len(MD5FullFile))
print("Da luu cac mau trong folder 2 khac folder 1 vao trong thu muc: ", Directory2+"\\res")