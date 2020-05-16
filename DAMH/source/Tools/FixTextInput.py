# Chuẩn hóa lại file input train

import os
import hashlib 
import sys 

# path = sys.argv[1]
path = "H:\\DoAnTotNghiep\\DAMH\\data_crawl\\tap_test_virus\\res"
def GetNextWord(content, index):
    start_index = index 
    word = ""
    while content[index] == ' ':
        index += 1
    while content[index] != ' ':
        word += content[index]
        index += 1
        # Exception neu word la cac dau ngan cach
        if word == '.' or word == ',' or word == ';' \
                or word == '(' or word == ')' \
                or word == '[' or word == ']' \
                or word == '{' or word == '}':
            index += 1
            return GetNextWord(content, index)
    return [index, word]
def FixRepeatWords(path):
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        if os.path.isdir(Directory + "\\" + filename) == True:
            FixRepeatWords(Directory + "\\" + filename)
        else:
            # Truong hop neu la file 
            with open(Directory + "\\" + filename, 'r') as f:
                contents = f.readlines()
                for content in contents:
                    content = content.strip()
                    index = 0
                    current_word = ""
                    while index <= len(content):
                        temp = GetNextWord(content, index)
                        index, word = temp[0], temp[1]
                        index += 1
                        if current_word == word: 
                            count += 1
                            # replace thành ""
                            if count > 2:
                                content.replace(word, "")
                        else:
                            # refresh lai 
                            current_word = word 
                    
FixRepeatWords(path)