# Chinh sua lại code HTML cho đúng chuẩn để tránh sai sót
# Dùng cho trường hợp đang bị crawl sai cấu trúc HTML
# Save ra file temp, xoa file hien tai, doi ten file chuan

import os 
path = "H:\\DoAnTotNghiep\\DAMH\\source\\mau loi"

count_exception = 0
def FixCleanHTML(path):
    Directory = path
    global count_exception
    # Can tao va mo file text moi o day de save text vao
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        try :
            with open(Directory + "\\" + filename, errors='ignore', 'r') as f: # đọc file mẫu 
                Lines = f.readlines() # đọc từng dòng
                for line in Lines:
                    line.replace("\"\"", "\"")
                    with open(Directory + "\\" + filename, errors='ignore',  'w') as f:
                        f.write(line)
        except Exception as e:
            # Neu xay ra exception trong qua trinh check thi khong lay mau do nua
            count_exception += 1
            print(e)

FixCleanHTML(path)
print("So mau bi exception: ", count_exception)