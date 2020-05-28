# Sửa lại tất cả các mẫu có cấu trúc HTML sai đã crawl về từ trước tới giờ

# Thuật toán sửa: 
# --> Tạo một file temp trong thư mục chứa file cần sửa
# --> Đọc từng dòng trong file cần sửa
# --> Thay thế những ký tự \"\" thành \"
# --> Loại bỏ một số ký tự thừa đầu file
# --> Loại bỏ một số ký tự cuối file 
# Ghi lại nội dung ra file temp 
# Xóa file đọc
# Đóng file temp 
# Sửa tên file temp thành tên file bị xóa 
# Kết thúc 

import os
import hashlib 
import sys 

# Đường dẫn chứa tập mẫu (tập sạch)
# path = sys.argv[1]
path = "D:\\test"

def FixHTMLCrawlSample(path):
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        # Nếu là thư mục
        if os.path.isdir(Directory + "\\" + filename) == True:
            FixHTMLCrawlSample(Directory + "\\" + filename)
        # Nếu là file log hoặc text
        elif (".log" in filename) or (".txt" in filename):
            continue
        # Còn lại là file HTML, JavaScript
        else:
            # Truong hop neu la file 
            try:
                with open(Directory + "\\" + filename, 'r') as f:
                    # Tạo file temp
                    f_new = open(Directory + "\\temp","w+")

                    # Đọc dữ liệu từ file HTML gốc
                    contents = f.readlines()

                    # Đối với dòng đầu tiên::
                    # Thay thế ký tự kết thúc thành ""
                    new_content = contents[0].replace('\x00', '').lower()
                    # Thay thế ký tự
                    new_content = new_content.replace("html,\"", "")
                    f_new.write(new_content)

                    for content in contents[1:len(contents)-1]:
                        # Thay thế ký tự kết thúc thành ""
                        new_content = content.replace('\x00', '').lower()
                        # Thay thế ký tự
                        new_content = new_content.replace("\"\"", "\"")
                        f_new.write(new_content)

                    # Đối với dòng cuối cùng::
                    # Thay thế ký tự kết thúc thành ""
                    new_content = contents[len(contents)-1].replace('\x00', '').lower()
                    # Thay thế ký tự
                    new_content = new_content.replace("\"\"", "\"")
                    f_new.write(new_content[:len(new_content)-1])
                    # Đóng file temp
                    f_new.close()
                f.close()
                # Xóa file 
                os.remove(Directory + "\\" + filename)
                # Rename temp file 
                os.rename(Directory + "\\temp", Directory + "\\" + filename)
            except Exception as e:
                print(e)
                continue

# Entry point    
FixHTMLCrawlSample(path)