#!/usr/bin/env python
#-*- coding:utf-8 -*-

from Tkinter import *
import threading
import msgpack
import socket
import time
import Queue

class sock():
    def __init__(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.ip = "192.168.5.137"
        self.port = 9527
        self.userName = ""

    def setUserName(self, userName):
        self.userName = userName

    def saveLocalTxt(self):
        f = open("config.txt", 'w')
        f.write(str(self.ip) + "\n")
        f.write(str(self.port) + "\n")
        f.write(str(self.userName))
        f.close()

    def onReadMsg(self):
        while True:
            self.needByteCnt = 2
            self.buf = ""

            while self.needByteCnt != 0:
                head = self.s.recv(self.needByteCnt)
                if not head:
                    #self.s._checkClosed()
                    raise Exception("Invalid socket!")
                self.buf += head
                self.needByteCnt -= len(head)

            self.needByteCnt = ord(self.buf[0]) + ord(self.buf[1]) * 256
            self.buf = ""

            while self.needByteCnt != 0:
                body = self.s.recv(self.needByteCnt)
                if not body:
                    raise Exception("Invalid socket!")
                self.buf += body
                self.needByteCnt -= len(body)
            data = msgpack.unpackb(self.buf)
            q.put(data)

            #time.sleep(1)

    def reconnect(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectServer()

    def connectServer(self):
        try:
            label["text"] = "try connect server->" + str(self.ip) + ":" + str(self.port) + " ..."
            self.s.connect((self.ip, self.port))
            CALL_SRV.login(self.userName, "py")
            self.onReadMsg()
        except:
            time.sleep(1)
            self.reconnect()

sk = sock()
with open ("config.txt") as f:
    list = f.readlines()
    for v in list:
        if list.index(v) == 0:
            sk.ip = str(v).strip()
        if list.index(v) == 1:
            sk.port = int(v.strip())
        if list.index(v) == 2:
            sk.userName = str(v).strip()

print (sk.ip)
print (sk.port)
print (sk.userName)

q = Queue.Queue()

allFrame = {}

sizeY = 380
sizeX = 600
def getSizeStr(incY):
    ans = '{}x{}' .format(sizeX,sizeY+incY)
    #print ans, incY
    return ans

top = Tk()
top.geometry(getSizeStr(0))

fTop = Frame(top)
fTop.pack(side=TOP, fill=X)

fMid = Frame(top)
fMid.pack(side=TOP)

fLeft = Frame(top)
fLeft.pack(side=TOP, fill=X)

label = Label(fLeft, text="ERROR", justify=LEFT)
label.pack(side=TOP, fill=X)

text = Text(fLeft)
text.pack(side=BOTTOM, fill=X)

class Proxy():
    def __init__(self):
        pass

    def qctool(self, msg):
        #label["text"] = msg
        #text.delete(1.0, END);
        text.insert(END, msg)
        nowLen = len(text.get(1.0, END))
        if nowLen >= 10000:
            text.delete(1.0, 100.0)
        #print nowLen, len(text.get(1.0, END))
        text.see(END)
        text.update()

    def syncCmd(self, list):
        for k, v in allFrame.items():
            btn_del(k)
        for info in list:
            btn_more(info["msg"])
        #q.put(list)

    def syncTitle(self, msg):
        label["text"] = msg

    def pullCmd(self):
        btn_sync(True)

    def saveLocalInfo(self, userName):
        print("save", userName)
        sk.setUserName(userName)
        sk.saveLocalTxt()
proxy = Proxy()


def run(number):
    sk.connectServer()

class Net():
    def __init__(self):
        pass

    def __getattr__(self, method_name):
        def caller(*args):
            b = msgpack.packb([method_name, args])
            h = str("%1c%1c" % (len(b)%256,int(len(b)/256)))
            try:
                sk.s.send(h + b)
            except:
                print ("socket error")
        return caller
CALL_SRV = Net()


def btn_send(idx):
    e = allFrame[idx]["e"]
    msg = e.get()
    if msg != "":
        localtime = time.asctime(time.localtime(time.time()))
        text.insert(END, "\n[%s][SEND][" % (localtime) + msg + "]\n")
        CALL_SRV.qctool(msg)

def updateRootSize():
    cnt = len(allFrame)
    top.geometry(getSizeStr(cnt * 30))

def btn_del(idx):
    if allFrame.has_key(idx):
        allFrame[idx]["e"].destroy()
        allFrame[idx]["b"].destroy()
        allFrame[idx]["x"].destroy()
        allFrame[idx]["f"].destroy()
        allFrame.pop(idx)
        btn_sync(True)
        updateRootSize()


def add_one_input_frame(idx, msg):
    if allFrame.has_key(idx):
        return -1
    f = Frame(fTop)
    f.pack(side=TOP, fill=X, expand=1)
    e = Entry(f, justify=RIGHT)
    e.pack(side=LEFT, fill=X, expand=1)
    e.insert(INSERT, msg)
    x = Button(f, text="X", command=lambda: btn_del(idx), bg="red")
    x.pack(side=RIGHT)
    b = Button(f, text="SEND", command=lambda: btn_send(idx))
    b.pack(side=RIGHT)
    allFrame[idx] = {"f": f, "e": e, "b": b, "x": x, "idx": idx}
    return idx


def btn_more(msg):
    max = 0
    for k,v in allFrame.items():
        if k > max:
            max = k
    idx = add_one_input_frame(max + 1, msg)
    btn_sync(True)
    updateRootSize()
    return idx

def cmpKey(elem):
    return elem["idx"]

def btn_sync(bg):
    cmdList = []
    for k, v in allFrame.items():
        msg = v["e"].get()
        d = {"idx": k, "msg": msg}
        cmdList.append(d)
    cmdList.sort(key=cmpKey)
    if bg:
        CALL_SRV.uploadCmdBg(cmdList)
    else:
        CALL_SRV.uploadCmd(cmdList)

def main():
    th = threading.Thread(target=run, args=(100,))
    th.setDaemon(True)
    th.start()

    more = Button(fMid, text="MORE", command=lambda: btn_more(""))
    more.pack(side=LEFT)

    sync = Button(fMid, text="SYNC", command=lambda: btn_sync(False))
    sync.pack(side=RIGHT)

    def timer():
        if not q.empty():
            data = q.get(False)
            method = data[0]
            args = data[1]
            if hasattr(proxy, method) and callable(getattr(proxy, method)):
                getattr(proxy, method)(*args)
            else:
                print ("error!", method)
        top.after(200, timer)
    timer()

    mainloop()

if __name__ == '__main__':
    main()