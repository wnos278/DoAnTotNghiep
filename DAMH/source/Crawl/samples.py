import hashlib
 
def sha256_cal(filename):
    sha256_hash = hashlib.sha256()
    with open(filename,"rb") as f:
        for byte_block in iter(lambda: f.read(4096),b""):
            sha256_hash.update(byte_block)
        return (sha256_hash.hexdigest())

def put_sample(_dir): # folder name
    os.chdir(_dir)
    for file_name in glob.glob("*"):
        __dir = _dir + "\\" + file_name
        try:
            with open(__dir, 'r', encoding="utf-8") as content_file:
                content = content_file.read()

                #read sha256
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