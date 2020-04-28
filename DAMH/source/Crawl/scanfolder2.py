# Tool quet trong thu muc chi dinh de tim cac mau va phan loai theo HTML, hoac VB, ...
# sau khi lay nguon tu VirusTotal
import glob, os
from shutil import copytree
from shutil import copyfile
_dir = "H:\\crawl and extract\\sample\\fortinet\\"

os.chdir(_dir)
dest_folder = "H:\\crawl and extract\\sample\\fortinet\\JS\\"

magic_dict = {
    "\x1f\x8b\x08": "gz",
    "\x42\x5a\x68": "bz2",
    "\x50\x4b\x03\x04": "zip"
    }

max_len = max(len(x) for x in magic_dict)

def file_type(filename):
    with open(filename) as f:
        file_start = f.read(max_len)
    for magic, filetype in magic_dict.items():
        if file_start.startswith(magic):
            return filetype
    return "no match"

os.chdir(dest_folder)
dest_folder2 = dest_folder + "re\\"
for file_name in glob.glob("*"):
    content = ""
    __dir = dest_folder + file_name
        # print(__dir)
    for d in glob.glob(__dir + "\\*"):
        if ".txt" not in d:
            try:
                if file_type(d) == "no match":
                    arr = d.split("\\")
                    try: 
                        os.mkdir(dest_folder2+arr[len(arr)-2] + "\\")
                    except:
                        pass
                    copyfile(d, dest_folder2+arr[len(arr)-2] + "\\" + arr[len(arr)-1])
                else:
                    continue
            except Exception as e:
                print(e)

print("done")