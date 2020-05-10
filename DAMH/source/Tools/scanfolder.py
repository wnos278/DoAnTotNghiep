# Tool quet trong thu muc chi dinh de tim cac mau va phan loai theo HTML, hoac VB, ...
# sau khi lay nguon tu VirusTotal
import glob, os

_dir = "H:\\crawl and extract\\sample\\out"

os.chdir(_dir)
folder_list = []
count = 0
list_key_check = [["<script", "/script>"], ["function"], ["eval"], ["document.write"], ["WScript"]]
list_accept_dir = []
for folder_name in glob.glob("*.*"):
    folder_list.append(folder_name)
    count += 1

# tra ve true neu co code js
# 
def _filter(_dir):
    os.chdir(_dir)
    for file_name in glob.glob("*"):
        if '.txt' not in file_name:
            __dir = _dir + "\\" + file_name
            # print(__dir)
            try:
                with open(__dir, 'r', encoding="utf-8") as content_file:
                    content = content_file.read()
            except:
                continue
            flag = True
            for checkKeys in list_key_check:
                flag = True
                for key in checkKeys:
                    if key not in content:
                        flag = False 
                if flag == True: 
                    break
            if flag==False:
                content_file.close()
                return False 
            else:
                content_file.close()
                return True
    return False

count1 = 0
for i in range(len(folder_list)):
    if(_filter(_dir + "\\" + folder_list[i])):
        print(folder_list[i])
        count1 += 1
print(count1)

# list_accept_file  = []
# def scan_1File(_dir):
#     count2 = 0
#     os.chdir(_dir)
#     for file_name in glob.glob("*"):
#         if '.txt' not in file_name:
#             try:
#                 __dir = _dir + "\\" + file_name
#                 with open(__dir, 'r', encoding="utf-8") as content_file:
#                     content = content_file.read()
#             except:
#                 continue
#             flag = True
#             for checkKeys in list_key_check:
#                 flag = True
#                 for key in checkKeys:
#                     if key not in content:
#                         flag = False 
#                 if flag == True: 
#                     break
#         else:
#             flag = False 
#         if flag == True:
#             list_accept_file.append(file_name)
#             count2 += 1
#             print(count2)
#             from shutil import copyfile
#             src = _dir + "\\" + file_name
#             dst = _dir + "\\re\\" + file_name
#             copyfile(src, dst)

#     print(list_accept_file)  

# scan_1File("H:\\crawl and extract\\sample\\out\\None")