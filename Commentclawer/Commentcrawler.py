import urllib
import urllib.request
import os
import re
import binascii
import struct
import json
from  xml.dom import  minidom
import zlib
import getopt
import sys
import threading
from multiprocessing import Pool as ThreadPool 


bs=struct.Struct('III')
#------------------------------------------------------------
def getXml(url):

    try:
        req = urllib.request.urlopen(url)
        xmlfile = req.read()
        decompressor = zlib.decompressobj(-zlib.MAX_WBITS)
        xmlfile = decompressor.decompress(xmlfile)
    except:
        xmlfile = '<?xml version="1.0" encoding="UTF-8"?><i></i>'
    try:
        oxml = minidom.parseString(xmlfile)
    except:
        oxml = minidom.parseString('<?xml version="1.0" encoding="UTF-8"?><i></i>')
    xml=oxml.documentElement
    return xml
    
#------------------------------------------------------------
def getJson(url):
    #print(url)
    retryTimes=1
    for i in range(retryTimes):
        try:
            jsonfile = urllib.request.urlopen(url).read()
            decompressor = zlib.decompressobj(-zlib.MAX_WBITS)
            jsonfile = decompressor.decompress(jsonfile)
            break
        except:
            jsonfile = '{}'
    try:
        _json = json.loads(jsonfile.decode())
    except:
        _json = json.loads('{}')
    __json = _json
    return __json

#------------------------------------------------------------
def get_attrvalue(node, attrname):
     return node.getAttribute(attrname) if node else ''
#------------------------------------------------------------
def get_nodevalue(node, index = 0):
    try:
        return node.childNodes[index].nodeValue
    except:
        return ''
#------------------------------------------------------------
def get_xmlnode(node,name):
    return node.getElementsByTagName(name) if node else []
#------------------------------------------------------------
def parseComment(node):
    danmakus = get_xmlnode(node,"d")
    if len(danmakus)==0:
        return [],0
    result=[]
    for danmaku in danmakus:
        p=get_attrvalue(danmaku,"p")
        dattrs=p.split(',')
        time=dattrs[4]
        hash=dattrs[6]
        text=get_nodevalue(danmaku)
        if len(time)<2 or len(hash)< 2 or len(text)==0 :
            continue
        else:
            result.append({"time":time,"hash":hash,"text":text}) 
    return result,get_nodevalue(get_xmlnode(node,"maxlimit")[0])

#------------------------------------------------------------
def dicArrUnique(oldArr):
    newArr = []
    hashArr = []
    le = 0
    for x in oldArr:
        uhash = x["time"] + x["hash"]
        if uhash not in hashArr :
            hashArr.append(uhash)
            newArr.append(x)
            le +=1
    return newArr,le

#------------------------------------------------------------
def getDanmaku(cid):
    base=getXml(r"http://comment.bilibili.com/"+cid+".xml")
    content,limit=parseComment(base)
    if len(content)==0:
        return []
    limit = int(limit)
    if len(content)>=limit and limit!=0:
        his=getJson(r"http://comment.bilibili.com/rolldate,"+cid)
        childLim = 0
        for node in his:
            childLim += int(node['new'])
            if childLim>=limit:
                childLim = 0
                child=getXml(r'http://comment.bilibili.com/dmroll,'+node['timestamp']+','+cid)
                oldcontent,lim=parseComment(child)
                orilen=len(content)
                content += oldcontent
                content,le = dicArrUnique(content)
                if le == orilen:
                    break
    return content

#------------------------------------------------------------
def writeBin(cid,content):
    f=open(r'prx/'+cid+'.prx','wb+')
    
    for node in content:
        usrhash = int(node['hash'],16)
        values=(usrhash,int(cid),int(node['time']))
        bdata=bs.pack(*values)
        f.write(bdata)
        f.write(node['text'].encode()+'\0'.encode('gb2312'))
    f.close()
    return


#------------------------------------------------------------
def danmakuInstance(cid):
    cid = str(cid)
    content=getDanmaku(cid)
    if len(content)>0:
        writeBin(cid,content)
        #print(cid, end='\n')

#--main------------------------------------------------------

#danmakuInstance('959965')
if __name__ == '__main__':


    opts, args = getopt.getopt(sys.argv[1:], "s:e:t:")

    threads = 15
    start = 1
    end = 4900000
    for op, value in opts:
        if op == "-s":
            start = int(value)
        elif op == "-e":
            end = int(value)
        elif op == "-t":
            threads = int(value)

    cids = []
    for i in range(start,end):
        cids.append(i)

    # Make the Pool of workers
    pool = ThreadPool(threads) 

    pool.map(danmakuInstance, cids)

    pool.close() 
    pool.join() 