#!/usr/bin/python3

import sys
import os

config="~/.stran/.configrc"
configDst = os.path.expanduser(config)

configSrc = "../config/.configrc"

def configFileExist():
    return os.path.exists(configDst)

def generateNewLine( srcline, dstlines, result ):

    i = srcline.index(':') 
    srckey = srcline[0:i]

    for dstline in dstlines:
        if srckey in dstline:
            print("包含相同配置项: "+dstline.replace("\n","")+" 不覆盖")
            result.append(dstline)
            return

    print('发现新配置项，添加到目标文件')
    result.append(srcline)

def mergeConfigFile():

    dstlines = None
    result = []

    #读取目标配置文件所有行到dslines
    with open(configDst, "r") as d:
        dstlines = d.readlines()

    #读取源配置文件并对比目标配置文件进行合并
    with open(configSrc, "r") as s:
        lines = s.readlines()
        for srcline in lines:
            generateNewLine(srcline, dstlines, result)

    #将生成的结果写到目标配置文件中
    with open(configDst,"w") as d:
        d.writelines(result)


if __name__ == "__main__":

    if not configFileExist():
        os.system("cp "+configSrc+" "+configDst)
        sys.exit(0)

    mergeConfigFile()
