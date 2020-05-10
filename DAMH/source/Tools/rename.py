import os
import hashlib 

def sha256_cal(filename):
    sha256_hash = hashlib.sha256()
    with open(filename,"rb") as f:
        for byte_block in iter(lambda: f.read(4096),b""):
            sha256_hash.update(byte_block)
        f.close()
    return (sha256_hash.hexdigest())

path = "H:\\testhamdoiten"
def rename(path):
    Directory = path
    for count, filename in enumerate(os.listdir(path)): # listdir: thu muc chua tapa cac mau
        if os.path.isdir(Directory + "\\" + filename) == True:
            rename(Directory + "\\" + filename)
        else:
            if '.txt' not in filename:
                newName = sha256_cal(Directory + "\\" + filename)
                try:
                    os.rename(Directory + "\\" + filename, Directory + "\\" + newName)
                except:
                    continue 
        
rename(path)