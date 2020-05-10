
import os
import hashlib
from shutil import copyfile

MD5FullFile= []

file_list_clean = os.listdir("H:\\DoAnTotNghiep\\DAMH\\data_crawl\\sontdc_data_js_copy\\clean_js")
DirectoryClean = "H:\\DoAnTotNghiep\\DAMH\\data_crawl\\sontdc_data_js_copy\\clean_js"
# os.mkdir(DirectoryClean + "\\res")
bRet = False
i = 0
arr_checktrungfile= []
for b in range(len(file_list_clean)):
    with open(DirectoryClean+'\\'+file_list_clean[i], 'r',encoding='utf-8') as file_to_check:
        data = file_to_check.read()    
        md5_returned = hashlib.md5(data.encode('utf-8')).hexdigest()
        file_to_check.close()
    for md5 in MD5FullFile:
        if md5_returned == md5:
            TenFileBiTrung = file_list_clean[i]
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
        mang = arr_checktrungfile.append([md5_returned, file_list_clean[i]])
        # 
        MD5FullFile.append(md5_returned)
        # copyfile(DirectoryClean+'\\'+file_list_clean[i], DirectoryClean + '\\res\\' + file_list_clean[i])
    i+=1   

# print(len(MD5FullFile))
print(arr_checktrungfile[2])
print(len(arr_checktrungfile[2]))