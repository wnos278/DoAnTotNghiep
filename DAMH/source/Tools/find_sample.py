# tool có nhiệm vụ trả ra tên của file có mã sha256 cần tìm
import hashlib
 
def sha256_cal(filename):
    sha256_hash = hashlib.sha256()
    with open(filename,"rb") as f:
        for byte_block in iter(lambda: f.read(4096),b""):
            sha256_hash.update(byte_block)
        return (sha256_hash.hexdigest())

directory = ""

def _filter(_dir, sha):
    os.chdir(_dir)
    for file_name in glob.glob("*"):
        if '.txt' not in file_name:
            __dir = _dir + "\\" + file_name
            # print(__dir)
            try:
               sha_value = sha256_cal(__dir)
            except:
                continue
        if sha_value == sha:
            return file_name

    return "eror" 

print(_filter(directory), ) # them ma sha can tim