# Tool crawl data dang HTML tu website chinh thong
# lay nguon tu Alexa: top 500 link chinh thong: https://www.alexa.com/topsites
from bs4 import BeautifulSoup
import urllib.request
import time
import os

def writeToFile(file_name, data):
    import csv 
    with open(file_name, "w", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(data)

def random_name():
    import random 
    import string 
    return ''.join(random.choice(string.ascii_lowercase) for i in range(64))

def main():
    temp = []
    list_link = ["https://google.com", "https://youtube.com", "https://tmall.com", 
    "https://facebook.com", "https://baidu.com", "https://qq.com", 
    "https://sohu.com", "https://login.tmall.com", "https://taobao.com", 
    "https://360.cn", "https://yahoo.com", "https://jd.com", 
    "https://wikipedia.org", "https://amazon.com", "https://sina.com.cn", 
    "https://weibo.com", "https://pages.tmall.com", "https://zoom.us", 
    "https://live.com", "https://netflix.com", "https://reddit.com", 
    "https://xinhuanet.com", "https://microsoft.com", "https://office.com",
    "https://okezone.com", "https://vk.com", "https://blogspot.com"
    "https://csdn.net", "https://instagram.com", "https://alipay.com"
    "https://yahoo.co.jp", "https://twitch.tv", "https://bongacams.com"
    "https://bing.com", "https://google.com.hk", "https://microsoftonline.com",
    "https://livejasmin.com", "https://tribunnews.com", "https://panda.tv",
    "https://zhanqi.tv", "https://stackoverflow.com", "https://naver.com", 
    "https://amazon.co.jp", "https://worldometers.info", "https://twitter.com",
    "https://tianya.cn", "https://aliexpress.com", "https://google.co.in",
    "https://ebay.com", "https://mama.cn"
    ]
    limit_page = 5000
    count = 0
    leng = len(list_link)
    os.mkdir(".\\data")
    while (leng > 0) and (count < limit_page):
        try:
            fp = urllib.request.urlopen(list_link[0])
            mybytes = fp.read()
            mystr = mybytes.decode("utf8")
            fp.close()
            soup = BeautifulSoup(mystr, 'html.parser') #convert to html file
            # save to file html
            writeToFile('.\\data\\mausach_' + random_name(), soup)
        except Exception as e:
            print(e)
            list_link.remove(list_link[0])
            leng = len(list_link)
            continue
        
        print(str(count) + "_done:" + list_link[0])
        list_link.remove(list_link[0])

        for src in soup.find_all('a'):
            s = src.get('href')
            if s==None:
                continue
            if 'http' in s:
                # Tim tat ca link co mat trong trang
                if s not in temp:
                    list_link.append(s)
                    temp.append(s)
        # time.sleep(0.125)
        leng = len(list_link)
        count += 1
        if (len(temp) > 2000):
            temp = []

if __name__ == '__main__':
    main()